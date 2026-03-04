#include "screen_manager.h"
#include "esp_log.h"

static const char *TAG = "screen_mgr";

static lv_obj_t *s_container = NULL;
static const screen_handler_t *s_handlers[SCREEN__COUNT] = {NULL};
static lv_obj_t *s_screens[SCREEN__COUNT] = {NULL};
static screen_id_t s_active = SCREEN_HOME;
static bool s_initialized = false;

void screen_manager_init(lv_obj_t *content_container)
{
    s_container = content_container;
    s_initialized = true;
    ESP_LOGI(TAG, "screen manager initialized");
}

void screen_manager_register(screen_id_t id, const screen_handler_t *handler)
{
    if (id < 0 || id >= SCREEN__COUNT) {
        ESP_LOGE(TAG, "register: invalid screen id %d", id);
        return;
    }
    s_handlers[id] = handler;
}

void screen_manager_show(screen_id_t id)
{
    if (!s_initialized || id < 0 || id >= SCREEN__COUNT) {
        ESP_LOGE(TAG, "show: invalid state or screen id %d", id);
        return;
    }

    if (id == s_active && s_screens[id] != NULL) {
        return;
    }

    if (s_screens[s_active] != NULL) {
        if (s_handlers[s_active] && s_handlers[s_active]->on_hide) {
            s_handlers[s_active]->on_hide();
        }
        lv_obj_add_flag(s_screens[s_active], LV_OBJ_FLAG_HIDDEN);
    }

    if (s_screens[id] == NULL) {
        if (!s_handlers[id] || !s_handlers[id]->create) {
            ESP_LOGE(TAG, "show: no create handler for screen %d", id);
            return;
        }
        s_screens[id] = s_handlers[id]->create(s_container);
    }

    lv_obj_remove_flag(s_screens[id], LV_OBJ_FLAG_HIDDEN);

    if (s_handlers[id] && s_handlers[id]->on_show) {
        s_handlers[id]->on_show();
    }

    s_active = id;
    ESP_LOGI(TAG, "showing screen %d", id);
}

screen_id_t screen_manager_get_active(void)
{
    return s_active;
}
