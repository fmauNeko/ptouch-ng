#include "lvgl.h"
#include "theme.h"
#include "i18n.h"
#include "fonts.h"
#include "material_symbols.h"
#include "esp_log.h"

static const char *TAG = "status_bar";

void status_bar_create(lv_obj_t *parent)
{
    const theme_palette_t *p = theme_get_palette();

    lv_obj_set_style_bg_color(parent, p ? p->surface : lv_color_black(), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(parent, 0, 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_set_style_border_side(parent, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(parent, 1, 0);
    if (p) {
        lv_obj_set_style_border_color(parent, p->border, 0);
    }
    lv_obj_set_style_pad_hor(parent, 12, 0);
    lv_obj_set_style_pad_ver(parent, 0, 0);
    lv_obj_remove_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(parent, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(parent, LV_FLEX_ALIGN_CENTER, 0);

    lv_color_t text_color = p ? p->subtext : lv_color_white();

    lv_obj_t *left = lv_obj_create(parent);
    lv_obj_remove_flag(left, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(left, LV_SIZE_CONTENT, LV_PCT(100));
    lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left, 0, 0);
    lv_obj_set_style_pad_all(left, 0, 0);
    lv_obj_set_style_radius(left, 0, 0);

    lv_obj_t *wifi_icon = lv_label_create(left);
    lv_label_set_text(wifi_icon, ICON_WIFI_OFF);
    lv_obj_set_style_text_font(wifi_icon, &material_symbols_20, 0);
    lv_obj_set_style_text_color(wifi_icon, text_color, 0);
    lv_obj_align(wifi_icon, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *wifi_label = lv_label_create(left);
    lv_label_set_text(wifi_label, i18n_get(STR_STATUS_NO_WIFI));
    lv_obj_set_style_text_font(wifi_label, &inter_regular_14, 0);
    lv_obj_set_style_text_color(wifi_label, text_color, 0);
    lv_obj_align_to(wifi_label, wifi_icon, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    lv_obj_t *center = lv_obj_create(parent);
    lv_obj_remove_flag(center, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(center, LV_SIZE_CONTENT, LV_PCT(100));
    lv_obj_set_style_bg_opa(center, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(center, 0, 0);
    lv_obj_set_style_pad_all(center, 0, 0);
    lv_obj_set_style_radius(center, 0, 0);

    lv_obj_t *clock_label = lv_label_create(center);
    lv_label_set_text(clock_label, i18n_get(STR_STATUS_CLOCK_PLACEHOLDER));
    lv_obj_set_style_text_font(clock_label, &inter_regular_14, 0);
    lv_obj_set_style_text_color(clock_label, text_color, 0);
    lv_obj_center(clock_label);

    lv_obj_t *right = lv_obj_create(parent);
    lv_obj_remove_flag(right, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(right, LV_SIZE_CONTENT, LV_PCT(100));
    lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right, 0, 0);
    lv_obj_set_style_pad_all(right, 0, 0);
    lv_obj_set_style_radius(right, 0, 0);

    lv_obj_t *printer_label = lv_label_create(right);
    lv_label_set_text(printer_label, i18n_get(STR_STATUS_NO_PRINTER));
    lv_obj_set_style_text_font(printer_label, &inter_regular_14, 0);
    lv_obj_set_style_text_color(printer_label, text_color, 0);
    lv_obj_center(printer_label);

    ESP_LOGD(TAG, "status bar created");
}
