#include "bsp/pandatouch.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ui.h"
#include "theme.h"

static const char *TAG = "ptouch-ng";

void app_main(void)
{
    // NVS init (required by BSP internals)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    } else {
        ESP_ERROR_CHECK(ret);
    }

    // BSP display init
    bsp_display_start();
    bsp_display_brightness_set(80);
    ESP_LOGI(TAG, "BSP display initialized");

    // Theme + UI init (inside display lock)
    bsp_display_lock(0);
    theme_init();
    ui_init();
    bsp_display_unlock();

    ESP_LOGI(TAG, "ptouch-ng M1 ready");
}
