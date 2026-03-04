#include <string.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"

static const char *TAG = "spike-multi-tls";

#define MAX_CONNS                   5
#define TARGET_HOST                 "homeassistant.dissidence.ovh"
#define TARGET_PORT                 443
#define CONN_TIMEOUT_MS             20000
#define SPIKE_EPOCH_2026_03_04      1772582400L  /* 2026-03-04 00:00:00 UTC */
#define HEAP_NOTE_THRESHOLD_BYTES   (50 * 1024)

void spike_multi_tls_run(void)
{
    struct timeval tv = { .tv_sec = SPIKE_EPOCH_2026_03_04 };
    settimeofday(&tv, NULL);

    size_t heap_initial = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    ESP_LOGI(TAG, "Initial heap free: %u KB", (unsigned)(heap_initial / 1024));

    esp_tls_t *handles[MAX_CONNS] = {0};
    int connected = 0;
    size_t heap_after_last = heap_initial;

    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
        .non_block = false,
        .timeout_ms = CONN_TIMEOUT_MS,
    };

    for (int i = 0; i < MAX_CONNS; i++) {
        size_t heap_before = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);

        esp_tls_t *tls = esp_tls_init();
        if (!tls) {
            ESP_LOGW(TAG, "CONN_%d/%d: FAILED — esp_tls_init() returned NULL",
                     i + 1, MAX_CONNS);
            heap_after_last = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
            continue;
        }

        int ret = esp_tls_conn_new_sync(TARGET_HOST, strlen(TARGET_HOST),
                                        TARGET_PORT, &cfg, tls);

        heap_after_last = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
        size_t delta = heap_before > heap_after_last
                       ? heap_before - heap_after_last : 0;

        if (ret == 1) {
            ESP_LOGI(TAG, "CONN_%d/%d: heap_free=%uKB, delta=%uKB",
                     i + 1, MAX_CONNS,
                     (unsigned)(heap_after_last / 1024),
                     (unsigned)(delta / 1024));
            handles[i] = tls;
            connected++;
        } else {
            int esp_err = 0, tls_err = 0;
            esp_tls_error_handle_t err_handle = NULL;
            esp_tls_get_error_handle(tls, &err_handle);
            if (err_handle) {
                esp_tls_get_and_clear_last_error(err_handle, &esp_err, &tls_err);
            }
            ESP_LOGW(TAG, "CONN_%d/%d: FAILED — esp_err=0x%x tls_err=0x%x"
                     " (heap_free=%uKB, delta=%uKB)",
                     i + 1, MAX_CONNS, esp_err, tls_err,
                     (unsigned)(heap_after_last / 1024),
                     (unsigned)(delta / 1024));
            esp_tls_conn_destroy(tls);
        }
    }

    size_t total_used = heap_initial > heap_after_last
                        ? heap_initial - heap_after_last : 0;
    size_t per_conn = connected > 0 ? total_used / connected : 0;
    size_t extrap_10 = per_conn * 10;

    ESP_LOGI(TAG, "TOTAL_HEAP_USED: %uKB for %d connections",
             (unsigned)(total_used / 1024), connected);
    ESP_LOGI(TAG, "PER_CONN_COST: %uKB average", (unsigned)(per_conn / 1024));
    ESP_LOGI(TAG, "EXTRAPOLATED_10: %uKB estimated for 10 connections",
             (unsigned)(extrap_10 / 1024));
    ESP_LOGI(TAG, "HEAP_REMAINING: %uKB free", (unsigned)(heap_after_last / 1024));

    if (per_conn > HEAP_NOTE_THRESHOLD_BYTES) {
        ESP_LOGW(TAG, "HEAP_NOTE: Consider reducing CONFIG_MBEDTLS_SSL_IN_CONTENT_LEN and "
                      "CONFIG_MBEDTLS_SSL_OUT_CONTENT_LEN from 16384 to 4096 (saves ~24KB/conn)");
    }

    for (int i = 0; i < MAX_CONNS; i++) {
        if (handles[i]) {
            esp_tls_conn_destroy(handles[i]);
        }
    }

    if (connected == 0) {
        ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — 0 connections established");
        return;
    }

    if (connected < MAX_CONNS) {
        ESP_LOGW(TAG, "SPIKE_RESULT: PARTIAL — %d/5 connections, heap_remaining=%uKB, per_conn=%uKB",
                 connected, (unsigned)(heap_after_last / 1024), (unsigned)(per_conn / 1024));
        return;
    }

    ESP_LOGI(TAG, "SPIKE_RESULT: PASS — 5 connections, heap_remaining=%uKB, per_conn=%uKB",
             (unsigned)(heap_after_last / 1024), (unsigned)(per_conn / 1024));
}
