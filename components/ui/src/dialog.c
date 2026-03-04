#include "widgets.h"
#include "theme.h"
#include "i18n.h"
#include "fonts.h"
#include "material_symbols.h"
#include "esp_log.h"

static const char *TAG = "dialog";

static lv_obj_t *s_active_dialog = NULL;

void ui_dialog_close(void)
{
    if (s_active_dialog && lv_obj_is_valid(s_active_dialog)) {
        lv_obj_delete(s_active_dialog);
    }
    s_active_dialog = NULL;
}

static void backdrop_clicked_cb(lv_event_t *e)
{
    ui_dialog_config_t *cfg = (ui_dialog_config_t *)lv_event_get_user_data(e);
    if (cfg && cfg->type == UI_DIALOG_INFO) {
        ui_dialog_close();
    }
}

static void confirm_btn_cb(lv_event_t *e)
{
    lv_event_cb_t user_cb = (lv_event_cb_t)lv_event_get_user_data(e);
    ui_dialog_close();
    if (user_cb) {
        user_cb(e);
    }
}

static void cancel_btn_cb(lv_event_t *e)
{
    lv_event_cb_t user_cb = (lv_event_cb_t)lv_event_get_user_data(e);
    ui_dialog_close();
    if (user_cb) {
        user_cb(e);
    }
}

void ui_dialog_show(const ui_dialog_config_t *config)
{
    if (!config) {
        return;
    }

    ui_dialog_close();

    const theme_palette_t *p = theme_get_palette();

    lv_obj_t *backdrop = lv_obj_create(lv_layer_top());
    lv_obj_set_size(backdrop, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_border_width(backdrop, 0, 0);
    lv_obj_set_style_radius(backdrop, 0, 0);
    lv_obj_set_style_pad_all(backdrop, 0, 0);

    if (p) {
        lv_obj_set_style_bg_color(backdrop, p->overlay, 0);
        lv_obj_set_style_bg_opa(backdrop, LV_OPA_70, 0);
    } else {
        lv_obj_set_style_bg_color(backdrop, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(backdrop, LV_OPA_50, 0);
    }

    if (config->type == UI_DIALOG_INFO) {
        lv_obj_add_event_cb(backdrop, backdrop_clicked_cb, LV_EVENT_CLICKED, (void *)config);
    }

    lv_obj_t *card = lv_obj_create(backdrop);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_pad_all(card, 24, 0);
    lv_obj_set_width(card, 400);
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_center(card);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(card, LV_FLEX_ALIGN_START, 0);

    if (p) {
        lv_obj_set_style_bg_color(card, p->surface, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(card, p->border, 0);
    }

    const char *icon_text = ICON_INFO;
    lv_color_t icon_color = p ? p->info : lv_color_white();

    switch (config->type) {
    case UI_DIALOG_WARNING:
        icon_text  = ICON_WARNING;
        icon_color = p ? p->warning : lv_color_white();
        break;
    case UI_DIALOG_ERROR:
        icon_text  = ICON_ERROR;
        icon_color = p ? p->error : lv_color_white();
        break;
    case UI_DIALOG_CONFIRM:
        icon_text  = ICON_HELP;
        icon_color = p ? p->primary : lv_color_white();
        break;
    case UI_DIALOG_INFO:
    default:
        break;
    }

    lv_obj_t *icon_label = lv_label_create(card);
    lv_label_set_text(icon_label, icon_text);
    lv_obj_set_style_text_font(icon_label, &material_symbols_28, 0);
    lv_obj_set_style_text_color(icon_label, icon_color, 0);
    lv_obj_set_style_pad_bottom(icon_label, 8, 0);

    const char *title_text = config->title;
    if (!title_text) {
        switch (config->type) {
        case UI_DIALOG_WARNING: title_text = i18n_get(STR_DIALOG_TITLE_WARNING); break;
        case UI_DIALOG_ERROR:   title_text = i18n_get(STR_DIALOG_TITLE_ERROR);   break;
        default:                title_text = i18n_get(STR_DIALOG_TITLE_INFO);    break;
        }
    }

    lv_obj_t *title_label = lv_label_create(card);
    lv_label_set_text(title_label, title_text);
    lv_obj_set_style_text_font(title_label, &inter_semibold_18, 0);
    if (p) {
        lv_obj_set_style_text_color(title_label, p->text, 0);
    }
    lv_obj_set_style_pad_bottom(title_label, 8, 0);

    if (config->message) {
        lv_obj_t *msg_label = lv_label_create(card);
        lv_label_set_text(msg_label, config->message);
        lv_label_set_long_mode(msg_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(msg_label, LV_PCT(100));
        lv_obj_set_style_text_font(msg_label, &inter_regular_16, 0);
        if (p) {
            lv_obj_set_style_text_color(msg_label, p->subtext, 0);
        }
        lv_obj_set_style_pad_bottom(msg_label, 16, 0);
    }

    lv_obj_t *btn_row = lv_obj_create(card);
    lv_obj_set_size(btn_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(btn_row, LV_FLEX_ALIGN_END, 0);

    if (config->type == UI_DIALOG_CONFIRM) {
        const char *cancel_text = config->btn_cancel ? config->btn_cancel : i18n_get(STR_BTN_CANCEL);
        lv_obj_t *cancel_btn = lv_button_create(btn_row);
        lv_obj_set_style_bg_opa(cancel_btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(cancel_btn, 0, 0);
        lv_obj_set_style_pad_hor(cancel_btn, 12, 0);
        lv_obj_set_style_pad_ver(cancel_btn, 8, 0);
        lv_obj_t *cancel_label = lv_label_create(cancel_btn);
        lv_label_set_text(cancel_label, cancel_text);
        if (p) {
            lv_obj_set_style_text_color(cancel_label, p->subtext, 0);
        }
        lv_obj_add_event_cb(cancel_btn, cancel_btn_cb, LV_EVENT_CLICKED, (void *)config->on_cancel);
    }

    const char *confirm_text = config->btn_confirm ? config->btn_confirm : i18n_get(STR_BTN_OK);
    lv_obj_t *confirm_btn = lv_button_create(btn_row);
    lv_obj_set_style_radius(confirm_btn, 8, 0);
    lv_obj_set_style_border_width(confirm_btn, 0, 0);
    lv_obj_set_style_pad_hor(confirm_btn, 16, 0);
    lv_obj_set_style_pad_ver(confirm_btn, 8, 0);
    if (p) {
        lv_obj_set_style_bg_color(confirm_btn, p->primary, 0);
        lv_obj_set_style_bg_opa(confirm_btn, LV_OPA_COVER, 0);
    }
    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, confirm_text);
    if (p) {
        lv_obj_set_style_text_color(confirm_label, p->bg, 0);
    }
    lv_obj_add_event_cb(confirm_btn, confirm_btn_cb, LV_EVENT_CLICKED, (void *)config->on_confirm);

    s_active_dialog = backdrop;

    ESP_LOGD(TAG, "dialog shown: type=%d", config->type);
}
