#include "widgets.h"
#include "theme.h"
#include "i18n.h"
#include "material_symbols.h"
#include "esp_log.h"

static const char *TAG = "widgets";

static lv_obj_t *s_active_toast = NULL;

lv_obj_t *ui_btn_create(lv_obj_t *parent, const char *text, ui_btn_variant_t variant)
{
    const theme_palette_t *p = theme_get_palette();

    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_style_radius(btn, 8, 0);
    lv_obj_set_style_pad_hor(btn, 16, 0);
    lv_obj_set_style_pad_ver(btn, 10, 0);
    lv_obj_set_style_border_width(btn, 0, 0);

    lv_color_t bg_color;
    lv_color_t text_color;

    if (p) {
        switch (variant) {
        case UI_BTN_SECONDARY:
            bg_color   = p->secondary;
            text_color = p->bg;
            break;
        case UI_BTN_DANGER:
            bg_color   = p->error;
            text_color = p->bg;
            break;
        case UI_BTN_PRIMARY:
        default:
            bg_color   = p->primary;
            text_color = p->bg;
            break;
        }
        lv_obj_set_style_bg_color(btn, bg_color, 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(btn, bg_color, LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(btn, LV_OPA_80, LV_STATE_PRESSED);
    }

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    if (p) {
        lv_obj_set_style_text_color(label, text_color, 0);
    }
    lv_obj_center(label);

    return btn;
}

lv_obj_t *ui_card_create(lv_obj_t *parent, const char *title)
{
    const theme_palette_t *p = theme_get_palette();

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_pad_all(card, 16, 0);

    if (p) {
        lv_obj_set_style_bg_color(card, p->surface, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(card, p->border, 0);
    }

    if (title != NULL) {
        lv_obj_t *title_label = lv_label_create(card);
        lv_label_set_text(title_label, title);
        if (p) {
            lv_obj_set_style_text_color(title_label, p->text, 0);
        }
    }

    return card;
}

lv_obj_t *ui_list_create(lv_obj_t *parent)
{
    const theme_palette_t *p = theme_get_palette();

    lv_obj_t *list = lv_list_create(parent);
    lv_obj_set_style_radius(list, 8, 0);
    lv_obj_set_style_border_width(list, 1, 0);
    lv_obj_set_style_pad_all(list, 0, 0);

    if (p) {
        lv_obj_set_style_bg_color(list, p->surface, 0);
        lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(list, p->border, 0);
    }

    return list;
}

lv_obj_t *ui_list_add_item(lv_obj_t *list, const char *icon_text, const char *label_text, lv_event_cb_t click_cb)
{
    const theme_palette_t *p = theme_get_palette();

    lv_obj_t *btn = lv_list_add_button(list, NULL, label_text);

    if (icon_text && icon_text[0] != '\0') {
        lv_obj_t *icon_label = lv_label_create(btn);
        lv_label_set_text(icon_label, icon_text);
        if (p) {
            lv_obj_set_style_text_color(icon_label, p->text, 0);
        }
        lv_obj_set_style_text_font(icon_label, &material_symbols_24, 0);
    }

    if (p) {
        lv_obj_set_style_bg_color(btn, p->surface, 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(btn, p->text, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_pad_hor(btn, 16, 0);
        lv_obj_set_style_pad_ver(btn, 12, 0);
    }

    if (click_cb) {
        lv_obj_add_event_cb(btn, click_cb, LV_EVENT_CLICKED, NULL);
    }

    return btn;
}

lv_obj_t *ui_spinner_create(lv_obj_t *parent, int32_t size)
{
    const theme_palette_t *p = theme_get_palette();

    if (size <= 0) {
        size = 48;
    }

    lv_obj_t *spinner = lv_spinner_create(parent);
    lv_obj_set_size(spinner, size, size);
    lv_spinner_set_anim_params(spinner, 1000, 60);

    if (p) {
        lv_obj_set_style_arc_color(spinner, p->primary, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(spinner, p->muted, LV_PART_MAIN);
        lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spinner, 4, LV_PART_MAIN);
    }

    return spinner;
}

static void toast_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *toast = (lv_obj_t *)lv_timer_get_user_data(timer);
    if (toast && lv_obj_is_valid(toast)) {
        lv_obj_delete(toast);
    }
    if (s_active_toast == toast) {
        s_active_toast = NULL;
    }
    lv_timer_delete(timer);
}

void ui_toast_show(const char *message, uint32_t duration_ms)
{
    const theme_palette_t *p = theme_get_palette();

    if (s_active_toast && lv_obj_is_valid(s_active_toast)) {
        lv_obj_delete(s_active_toast);
        s_active_toast = NULL;
    }

    lv_obj_t *toast = lv_obj_create(lv_layer_top());
    lv_obj_set_style_radius(toast, 24, 0);
    lv_obj_set_style_border_width(toast, 1, 0);
    lv_obj_set_style_pad_hor(toast, 24, 0);
    lv_obj_set_style_pad_ver(toast, 12, 0);
    lv_obj_set_style_shadow_width(toast, 8, 0);
    lv_obj_set_style_shadow_opa(toast, LV_OPA_30, 0);

    if (p) {
        lv_obj_set_style_bg_color(toast, p->surface, 0);
        lv_obj_set_style_bg_opa(toast, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(toast, p->border, 0);
    }

    lv_obj_t *label = lv_label_create(toast);
    lv_label_set_text(label, message);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, LV_SIZE_CONTENT);
    if (p) {
        lv_obj_set_style_text_color(label, p->text, 0);
    }

    lv_obj_set_size(toast, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -64);

    lv_obj_fade_in(toast, 200, 0);

    s_active_toast = toast;

    lv_timer_create(toast_timer_cb, duration_ms, toast);

    ESP_LOGD(TAG, "toast: %s (%u ms)", message, (unsigned int)duration_ms);
}
