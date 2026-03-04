#include "theme.h"
#include "esp_log.h"

static const char *TAG = "theme";

void theme_init(void)
{
    ESP_LOGI(TAG, "theme: stub (M1 scaffold)");
}

void theme_set_active(const char *name)
{
    ESP_LOGI(TAG, "theme_set_active: %s (stub)", name);
}

bool theme_is_dark(void)
{
    return false;
}
