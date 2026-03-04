#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "credentials.h"

static const char *TAG = "spike-mqtt";

extern const char ca_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const char ca_cert_pem_end[]   asm("_binary_ca_cert_pem_end");

static EventGroupHandle_t mqtt_event_group;
#define MQTT_CONNECTED_BIT  BIT0
#define MQTT_DATA_BIT       BIT1

static size_t heap_before;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);

        char sub_topic[64];
        snprintf(sub_topic, sizeof(sub_topic), "device/%s/report", CREDENTIALS_PRINTER_SN);
        int msg_id = esp_mqtt_client_subscribe(event->client, sub_topic, 1);
        if (msg_id < 0) {
            ESP_LOGW(TAG, "esp_mqtt_client_subscribe failed: %d", msg_id);
        }
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
        char pub_topic[64];
        snprintf(pub_topic, sizeof(pub_topic), "device/%s/request", CREDENTIALS_PRINTER_SN);
        const char *pushall = "{\"pushing\":{\"sequence_id\":\"1\",\"command\":\"pushall\","
                              "\"version\":1,\"push_target\":1}}";
        int pub_msg_id = esp_mqtt_client_publish(event->client, pub_topic, pushall, 0, 0, 0);
        if (pub_msg_id < 0) {
            ESP_LOGW(TAG, "esp_mqtt_client_publish failed: %d", pub_msg_id);
        }
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA topic_len=%d data_len=%d",
                 event->topic_len, event->data_len);
        int log_len = event->data_len < 256 ? event->data_len : 256;
        ESP_LOGI(TAG, "DATA(first %d bytes): %.*s", log_len, log_len, event->data);
        xEventGroupSetBits(mqtt_event_group, MQTT_DATA_BIT);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "  transport error: esp_tls_last_error=0x%x tls_stack_error=0x%x",
                     event->error_handle->esp_tls_last_esp_err,
                     event->error_handle->esp_tls_stack_err);
        }
        break;

    default:
        break;
    }
}

void spike_mqtt_run(void)
{
    heap_before = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    ESP_LOGI(TAG, "Heap before MQTT client: %u KB", (unsigned)(heap_before / 1024));

    mqtt_event_group = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .hostname  = CREDENTIALS_PRINTER_IP,
                .port      = 8883,
                .transport = MQTT_TRANSPORT_OVER_SSL,
            },
            .verification = {
                .certificate     = ca_cert_pem_start,
                .certificate_len = ca_cert_pem_end - ca_cert_pem_start,
                .common_name     = CREDENTIALS_PRINTER_SN,
            },
        },
        .credentials = {
            .username = "bblp",
            .authentication = {
                .password = CREDENTIALS_PRINTER_PASSCODE,
            },
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    if (!client) {
        ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — esp_mqtt_client_init() returned NULL (OOM)");
        return;
    }

    esp_err_t err = esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "esp_mqtt_client_register_event failed: %s", esp_err_to_name(err));
    }

    err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "esp_mqtt_client_start failed: %s", esp_err_to_name(err));
    }

    EventBits_t bits = xEventGroupWaitBits(mqtt_event_group,
                                           MQTT_CONNECTED_BIT,
                                           pdFALSE, pdTRUE,
                                           pdMS_TO_TICKS(30000));

    if (!(bits & MQTT_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — MQTT not connected within 30 s");
        esp_mqtt_client_destroy(client);
        return;
    }

    bits = xEventGroupWaitBits(mqtt_event_group,
                               MQTT_DATA_BIT,
                               pdFALSE, pdTRUE,
                               pdMS_TO_TICKS(60000));

    size_t heap_after = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    size_t delta = heap_before > heap_after ? heap_before - heap_after : 0;

    ESP_LOGI(TAG, "Heap after  MQTT client: %u KB", (unsigned)(heap_after / 1024));
    ESP_LOGI(TAG, "HEAP_DELTA: %u KB", (unsigned)(delta / 1024));

    if (delta > 100 * 1024) {
        ESP_LOGW(TAG, "HEAP_WARNING: TLS connection cost %u KB — consider reducing MBEDTLS buffer sizes",
                 (unsigned)(delta / 1024));
    }

    if (bits & MQTT_DATA_BIT) {
        ESP_LOGI(TAG, "SPIKE_RESULT: PASS — MQTT connected, data received, heap delta %u KB",
                 (unsigned)(delta / 1024));
    } else {
        ESP_LOGW(TAG, "SPIKE_RESULT: PARTIAL — MQTT connected but no data within 60 s, heap delta %u KB",
                 (unsigned)(delta / 1024));
    }

    // Leave client running (don't destroy — keeps connection for multi-TLS spike)
}
