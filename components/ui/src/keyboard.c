#include "widgets.h"
#include "theme.h"
#include "esp_log.h"

static const char *TAG = "keyboard";

static lv_obj_t *s_keyboard = NULL;

static void keyboard_ready_cb(lv_event_t *e)
{
    (void)e;
    ui_keyboard_hide();
}

void ui_keyboard_show(lv_obj_t *textarea)
{
    if (!s_keyboard || !lv_obj_is_valid(s_keyboard)) {
        s_keyboard = lv_keyboard_create(lv_layer_top());
        lv_obj_set_size(s_keyboard, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_align(s_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_keyboard_set_mode(s_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);

        const theme_palette_t *p = theme_get_palette();
        if (p) {
            lv_obj_set_style_bg_color(s_keyboard, p->surface, 0);
            lv_obj_set_style_bg_opa(s_keyboard, LV_OPA_COVER, 0);
            lv_obj_set_style_border_color(s_keyboard, p->border, 0);
            lv_obj_set_style_border_width(s_keyboard, 1, 0);
        }

        lv_obj_add_event_cb(s_keyboard, keyboard_ready_cb, LV_EVENT_READY, NULL);
        ESP_LOGD(TAG, "keyboard created");
    }

    lv_keyboard_set_textarea(s_keyboard, textarea);
    lv_obj_remove_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);

    /* Slide-up animation */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_keyboard);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    int32_t h = lv_obj_get_height(s_keyboard);
    lv_anim_set_values(&a, h, 0);
    lv_anim_set_duration(&a, 200);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    ESP_LOGD(TAG, "keyboard shown");
}

void ui_keyboard_hide(void)
{
    if (s_keyboard && lv_obj_is_valid(s_keyboard)) {
        lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
        ESP_LOGD(TAG, "keyboard hidden");
    }
}

bool ui_keyboard_is_visible(void)
{
    return s_keyboard &&
           lv_obj_is_valid(s_keyboard) &&
           !lv_obj_has_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
}
