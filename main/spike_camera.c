#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_tls.h"

#include "esp_jpeg_dec.h"

#include "credentials.h"

static const char *TAG = "spike-camera";

extern const char ca_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const char ca_cert_pem_end[]   asm("_binary_ca_cert_pem_end");

void spike_camera_run(void)
{
    esp_tls_cfg_t tls_cfg = {
        .cacert_buf   = (const unsigned char *)ca_cert_pem_start,
        .cacert_bytes = ca_cert_pem_end - ca_cert_pem_start,
        .common_name  = CREDENTIALS_PRINTER_SN,
        .timeout_ms   = 30000,
        .non_block    = false,
    };

    esp_tls_t *tls = esp_tls_init();
    if (!tls) {
        ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — esp_tls_init failed");
        return;
    }

    int ret = esp_tls_conn_new_sync(CREDENTIALS_PRINTER_IP,
                                    strlen(CREDENTIALS_PRINTER_IP),
                                    6000, &tls_cfg, tls);
    if (ret == 0) {
        ESP_LOGW(TAG, "SPIKE_RESULT: PARTIAL — camera port 6000 timeout (printer may need to be active/printing)");
        esp_tls_conn_destroy(tls);
        return;
    }
    if (ret != 1) {
        ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — TLS connect error: %d", ret);
        esp_tls_conn_destroy(tls);
        return;
    }
    ESP_LOGI(TAG, "Camera TLS connected");

    // --- Send 80-byte auth packet (all little-endian) ---
    uint8_t auth[80] = {0};
    // Binary protocol layout (little-endian, Bambu camera auth packet):
    // Bytes  0- 3: payload size = 0x40 (64)
    // Bytes  4- 7: type = 0x3000
    // Bytes  8-15: reserved (zero)
    // Bytes 16-47: "bblp" null-padded to 32 bytes
    // Bytes 48-79: passcode null-padded to 32 bytes
    auth[0] = 0x40;
    auth[4] = 0x00; auth[5] = 0x30;
    memcpy(&auth[16], "bblp", 4);
    strncpy((char *)&auth[48], CREDENTIALS_PRINTER_PASSCODE, 32);

    size_t written = 0;
    while (written < sizeof(auth)) {
        int n = esp_tls_conn_write(tls, auth + written, sizeof(auth) - written);
        if (n < 0) {
            ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — auth write error: %d", n);
            esp_tls_conn_destroy(tls);
            return;
        }
        written += n;
    }
    ESP_LOGI(TAG, "Auth packet sent (%d bytes)", (int)written);

    // --- Read 16-byte frame header ---
    uint8_t header[16] = {0};
    size_t hdr_read = 0;
    int64_t deadline = esp_timer_get_time() + 15000000LL; // 15s
    while (hdr_read < 16) {
        if (esp_timer_get_time() > deadline) {
            ESP_LOGW(TAG, "CAMERA_NOTE: stream may require MQTT activation — no frame within 15s");
            ESP_LOGW(TAG, "SPIKE_RESULT: PARTIAL — camera connected, auth sent, no frame received");
            esp_tls_conn_destroy(tls);
            return;
        }
        int n = esp_tls_conn_read(tls, header + hdr_read, 16 - hdr_read);
        if (n > 0) {
            hdr_read += n;
        } else if (n < 0 && n != ESP_TLS_ERR_SSL_WANT_READ) {
            ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — header read error: %d", n);
            esp_tls_conn_destroy(tls);
            return;
        }
    }

    uint32_t payload_size = (uint32_t)header[0] | ((uint32_t)header[1] << 8) |
                            ((uint32_t)header[2] << 16) | ((uint32_t)header[3] << 24);
    ESP_LOGI(TAG, "Frame header: payload_size=%lu", (unsigned long)payload_size);

    if (payload_size == 0 || payload_size > 500000) {
        ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — invalid payload size: %lu", (unsigned long)payload_size);
        esp_tls_conn_destroy(tls);
        return;
    }

    // --- Allocate PSRAM buffer and read JPEG payload ---
    uint8_t *jpeg_buf = heap_caps_malloc(payload_size, MALLOC_CAP_SPIRAM);
    if (!jpeg_buf) {
        ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — PSRAM alloc failed for %lu bytes", (unsigned long)payload_size);
        esp_tls_conn_destroy(tls);
        return;
    }

    size_t payload_read = 0;
    deadline = esp_timer_get_time() + 10000000LL; // 10s
    while (payload_read < payload_size) {
        if (esp_timer_get_time() > deadline) {
            ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — payload read timeout at %u/%lu bytes",
                     (unsigned)payload_read, (unsigned long)payload_size);
            heap_caps_free(jpeg_buf);
            esp_tls_conn_destroy(tls);
            return;
        }
        int n = esp_tls_conn_read(tls, jpeg_buf + payload_read, payload_size - payload_read);
        if (n > 0) {
            payload_read += n;
        } else if (n < 0 && n != ESP_TLS_ERR_SSL_WANT_READ) {
            ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — payload read error: %d", n);
            heap_caps_free(jpeg_buf);
            esp_tls_conn_destroy(tls);
            return;
        }
    }

    // Verify JPEG markers
    bool jpeg_valid = (jpeg_buf[0] == 0xFF && jpeg_buf[1] == 0xD8 &&
                       jpeg_buf[payload_size - 2] == 0xFF && jpeg_buf[payload_size - 1] == 0xD9);
    ESP_LOGI(TAG, "FRAME: size=%lu bytes, JPEG markers valid=%s",
             (unsigned long)payload_size, jpeg_valid ? "YES" : "NO");

    // --- JPEG decode using esp_new_jpeg ---
    size_t psram_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    int64_t t_start = esp_timer_get_time();

    jpeg_dec_handle_t jpeg_dec = NULL;
    jpeg_dec_config_t dec_cfg = DEFAULT_JPEG_DEC_CONFIG();
    dec_cfg.output_type = JPEG_PIXEL_FORMAT_RGB565_LE;

    jpeg_error_t jerr = jpeg_dec_open(&dec_cfg, &jpeg_dec);
    if (jerr != JPEG_ERR_OK) {
        ESP_LOGW(TAG, "SPIKE_RESULT: PARTIAL — camera connected, frame %lu bytes, decode open failed: %d",
                 (unsigned long)payload_size, (int)jerr);
        heap_caps_free(jpeg_buf);
        esp_tls_conn_destroy(tls);
        return;
    }

    jpeg_dec_io_t io = {
        .inbuf     = jpeg_buf,
        .inbuf_len = (int)payload_size,
        .outbuf    = NULL,
        .out_size  = 0,
    };
    jpeg_dec_header_info_t hdr_info = {0};
    jerr = jpeg_dec_parse_header(jpeg_dec, &io, &hdr_info);
    if (jerr != JPEG_ERR_OK) {
        ESP_LOGW(TAG, "SPIKE_RESULT: PARTIAL — camera connected, frame %lu bytes, header parse failed: %d",
                 (unsigned long)payload_size, (int)jerr);
        jpeg_dec_close(jpeg_dec);
        heap_caps_free(jpeg_buf);
        esp_tls_conn_destroy(tls);
        return;
    }

    uint16_t out_w = hdr_info.width;
    uint16_t out_h = hdr_info.height;
    size_t out_size = (size_t)out_w * (size_t)out_h * 2; // RGB565 = 2 bytes/pixel

    // ESP32-S3 requires 16-byte aligned output buffer for the decoder
    uint8_t *rgb_buf = heap_caps_aligned_alloc(16, out_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!rgb_buf) {
        ESP_LOGW(TAG, "SPIKE_RESULT: PARTIAL — PSRAM alloc failed for RGB buffer (%u bytes)", (unsigned)out_size);
        jpeg_dec_close(jpeg_dec);
        heap_caps_free(jpeg_buf);
        esp_tls_conn_destroy(tls);
        return;
    }

    io.outbuf = rgb_buf;
    jerr = jpeg_dec_process(jpeg_dec, &io);
    int64_t t_end = esp_timer_get_time();
    int decode_ms = (int)((t_end - t_start) / 1000);

    size_t psram_after = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t peak_used = (psram_before > psram_after) ? (psram_before - psram_after) : 0;

    ESP_LOGI(TAG, "DECODE: time=%dms, output=%ux%u, peak_heap_used=%uKB",
             decode_ms, (unsigned)out_w, (unsigned)out_h, (unsigned)(peak_used / 1024));

    jpeg_dec_close(jpeg_dec);
    heap_caps_free(rgb_buf);
    heap_caps_free(jpeg_buf);
    esp_tls_conn_destroy(tls);

    if (jerr != JPEG_ERR_OK) {
        ESP_LOGW(TAG, "SPIKE_RESULT: PARTIAL — camera connected, frame %lu bytes, decode failed: %d",
                 (unsigned long)payload_size, (int)jerr);
        return;
    }

    if (decode_ms > 200) {
        ESP_LOGW(TAG, "SPIKE_RESULT: PARTIAL — decode time %dms exceeds 200ms threshold", decode_ms);
        return;
    }

    ESP_LOGI(TAG, "SPIKE_RESULT: PASS — camera auth OK, frame %lu bytes, decode %dms, memory %uKB",
             (unsigned long)payload_size, decode_ms, (unsigned)(peak_used / 1024));
}
