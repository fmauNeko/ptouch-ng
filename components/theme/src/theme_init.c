#include <string.h>
#include "theme.h"
#include "esp_log.h"

static const char *TAG = "theme";

static const theme_config_t *s_active = NULL;

void theme_init(void)
{
    palettes_init();
    s_active = palettes_get_builtin(0);
    ESP_LOGI(TAG, "theme: initialized (active: %s)", s_active ? s_active->name : "none");
}

void theme_set_active(const char *name)
{
    for (size_t i = 0; i < palettes_get_count(); i++) {
        const theme_config_t *cfg = palettes_get_builtin(i);
        if (cfg && cfg->name && strcmp(cfg->name, name) == 0) {
            s_active = cfg;
            ESP_LOGI(TAG, "theme_set_active: %s", name);
            return;
        }
    }
    ESP_LOGW(TAG, "theme_set_active: unknown theme '%s'", name);
}

const theme_palette_t *theme_get_palette(void)
{
    return s_active ? s_active->palette : NULL;
}

bool theme_is_dark(void)
{
    return s_active ? s_active->is_dark : false;
}
