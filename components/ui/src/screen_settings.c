#include "screen_manager.h"
#include "i18n.h"
#include "theme.h"
#include "widgets.h"
#include "fonts.h"
#include "esp_log.h"

static const char *TAG = "screen_settings";

static void theme_toggle_cb(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e);
    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        theme_set_active("Catppuccin Mocha");
    } else {
        theme_set_active("Catppuccin Latte");
    }
    ui_toast_show(i18n_get(STR_TOAST_THEME_CHANGED), 2000);
    ESP_LOGI(TAG, "theme toggled");
}

static lv_obj_t *create(lv_obj_t *parent)
{
    const theme_palette_t *p = theme_get_palette();

    lv_obj_t *screen = lv_obj_create(parent);
    lv_obj_set_size(screen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(screen, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_pad_all(screen, 24, 0);
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(screen, LV_FLEX_ALIGN_START, 0);
    lv_obj_set_style_pad_row(screen, 16, 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, i18n_get(STR_SETTINGS_TITLE));
    lv_obj_set_style_text_font(title, &inter_semibold_24, 0);
    if (p) {
        lv_obj_set_style_text_color(title, p->text, 0);
    }

    lv_obj_t *row = lv_obj_create(screen);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(row, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(row, LV_FLEX_ALIGN_CENTER, 0);

    lv_obj_t *theme_label = lv_label_create(row);
    lv_label_set_text(theme_label, i18n_get(STR_SETTINGS_THEME));
    lv_obj_set_style_text_font(theme_label, &inter_regular_16, 0);
    if (p) {
        lv_obj_set_style_text_color(theme_label, p->text, 0);
    }

    lv_obj_t *toggle = lv_switch_create(row);
    if (theme_is_dark()) {
        lv_obj_add_state(toggle, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(toggle, theme_toggle_cb, LV_EVENT_VALUE_CHANGED, NULL);

    return screen;
}

static void on_show(void) { }
static void on_hide(void) { }

const screen_handler_t screen_settings_handler = {
    .create  = create,
    .on_show = on_show,
    .on_hide = on_hide,
};
