#include <string.h>
#include "theme.h"
#include "fonts.h"
#include "material_symbols.h"
#include "esp_log.h"
#include "themes/lv_theme_private.h"

static const char *TAG = "theme";

static const theme_config_t *s_active_config = NULL;
static lv_theme_t *s_custom_theme = NULL;

static lv_style_t style_bg;
static lv_style_t style_card;
static lv_style_t style_btn;
static lv_style_t style_btn_pressed;
static lv_style_t style_label;
static lv_style_t style_label_secondary;
static lv_style_t style_list;
static lv_style_t style_list_btn;
static lv_style_t style_spinner;
static lv_style_t style_switch;
static lv_style_t style_slider;
static lv_style_t style_textarea;
static lv_style_t style_dropdown;

static bool s_styles_initialized = false;

static void styles_init(const theme_palette_t *p)
{
    if (s_styles_initialized) {
        lv_style_reset(&style_bg);
        lv_style_reset(&style_card);
        lv_style_reset(&style_btn);
        lv_style_reset(&style_btn_pressed);
        lv_style_reset(&style_label);
        lv_style_reset(&style_label_secondary);
        lv_style_reset(&style_list);
        lv_style_reset(&style_list_btn);
        lv_style_reset(&style_spinner);
        lv_style_reset(&style_switch);
        lv_style_reset(&style_slider);
        lv_style_reset(&style_textarea);
        lv_style_reset(&style_dropdown);
    }

    lv_style_set_bg_color(&style_bg, p->bg);
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);
    lv_style_set_text_color(&style_bg, p->text);
    lv_style_set_text_font(&style_bg, &inter_regular_16);
    lv_style_set_border_width(&style_bg, 0);
    lv_style_set_pad_all(&style_bg, 0);

    lv_style_set_bg_color(&style_card, p->surface);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_border_color(&style_card, p->border);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_radius(&style_card, 12);
    lv_style_set_pad_all(&style_card, 16);
    lv_style_set_text_color(&style_card, p->text);

    lv_style_set_bg_color(&style_btn, p->primary);
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_text_color(&style_btn, p->bg);
    lv_style_set_text_font(&style_btn, &inter_semibold_16);
    lv_style_set_radius(&style_btn, 8);
    lv_style_set_pad_hor(&style_btn, 16);
    lv_style_set_pad_ver(&style_btn, 10);
    lv_style_set_border_width(&style_btn, 0);

    lv_style_set_bg_color(&style_btn_pressed, p->primary);
    lv_style_set_bg_opa(&style_btn_pressed, LV_OPA_80);
    lv_style_set_text_color(&style_btn_pressed, p->bg);

    lv_style_set_text_color(&style_label, p->text);
    lv_style_set_text_font(&style_label, &inter_regular_16);

    lv_style_set_text_color(&style_label_secondary, p->subtext);
    lv_style_set_text_font(&style_label_secondary, &inter_regular_14);

    lv_style_set_bg_color(&style_list, p->surface);
    lv_style_set_bg_opa(&style_list, LV_OPA_COVER);
    lv_style_set_border_color(&style_list, p->border);
    lv_style_set_border_width(&style_list, 1);
    lv_style_set_radius(&style_list, 8);
    lv_style_set_pad_all(&style_list, 0);

    lv_style_set_bg_color(&style_list_btn, p->surface);
    lv_style_set_bg_opa(&style_list_btn, LV_OPA_COVER);
    lv_style_set_text_color(&style_list_btn, p->text);
    lv_style_set_text_font(&style_list_btn, &inter_regular_16);
    lv_style_set_border_width(&style_list_btn, 0);
    lv_style_set_pad_hor(&style_list_btn, 16);
    lv_style_set_pad_ver(&style_list_btn, 12);

    lv_style_set_arc_color(&style_spinner, p->primary);
    lv_style_set_arc_width(&style_spinner, 4);

    lv_style_set_bg_color(&style_switch, p->muted);
    lv_style_set_bg_opa(&style_switch, LV_OPA_COVER);

    lv_style_set_bg_color(&style_slider, p->primary);
    lv_style_set_bg_opa(&style_slider, LV_OPA_COVER);

    lv_style_set_bg_color(&style_textarea, p->surface);
    lv_style_set_bg_opa(&style_textarea, LV_OPA_COVER);
    lv_style_set_border_color(&style_textarea, p->border);
    lv_style_set_border_width(&style_textarea, 1);
    lv_style_set_radius(&style_textarea, 8);
    lv_style_set_text_color(&style_textarea, p->text);
    lv_style_set_text_font(&style_textarea, &inter_regular_16);
    lv_style_set_pad_all(&style_textarea, 10);

    lv_style_set_bg_color(&style_dropdown, p->surface);
    lv_style_set_bg_opa(&style_dropdown, LV_OPA_COVER);
    lv_style_set_border_color(&style_dropdown, p->border);
    lv_style_set_border_width(&style_dropdown, 1);
    lv_style_set_radius(&style_dropdown, 8);
    lv_style_set_text_color(&style_dropdown, p->text);
    lv_style_set_text_font(&style_dropdown, &inter_regular_16);

    s_styles_initialized = true;
}

static void theme_apply_cb(lv_theme_t *th, lv_obj_t *obj)
{
    LV_UNUSED(th);

    if (lv_obj_check_type(obj, &lv_obj_class)) {
        lv_obj_add_style(obj, &style_bg, 0);
    }
    if (lv_obj_check_type(obj, &lv_button_class)) {
        lv_obj_add_style(obj, &style_btn, 0);
        lv_obj_add_style(obj, &style_btn_pressed, LV_STATE_PRESSED);
    }
    if (lv_obj_check_type(obj, &lv_label_class)) {
        lv_obj_add_style(obj, &style_label, 0);
    }
    if (lv_obj_check_type(obj, &lv_list_class)) {
        lv_obj_add_style(obj, &style_list, 0);
    }
    if (lv_obj_check_type(obj, &lv_list_button_class)) {
        lv_obj_add_style(obj, &style_list_btn, 0);
    }
    if (lv_obj_check_type(obj, &lv_spinner_class)) {
        lv_obj_add_style(obj, &style_spinner, LV_PART_INDICATOR);
    }
    if (lv_obj_check_type(obj, &lv_switch_class)) {
        lv_obj_add_style(obj, &style_switch, 0);
    }
    if (lv_obj_check_type(obj, &lv_slider_class)) {
        lv_obj_add_style(obj, &style_slider, LV_PART_INDICATOR);
    }
    if (lv_obj_check_type(obj, &lv_textarea_class)) {
        lv_obj_add_style(obj, &style_textarea, 0);
    }
    if (lv_obj_check_type(obj, &lv_dropdown_class)) {
        lv_obj_add_style(obj, &style_dropdown, 0);
    }
}

void theme_init(void)
{
    palettes_init();

    lv_style_init(&style_bg);
    lv_style_init(&style_card);
    lv_style_init(&style_btn);
    lv_style_init(&style_btn_pressed);
    lv_style_init(&style_label);
    lv_style_init(&style_label_secondary);
    lv_style_init(&style_list);
    lv_style_init(&style_list_btn);
    lv_style_init(&style_spinner);
    lv_style_init(&style_switch);
    lv_style_init(&style_slider);
    lv_style_init(&style_textarea);
    lv_style_init(&style_dropdown);

    lv_display_t *disp = lv_display_get_default();

    s_active_config = palettes_get_builtin(1);
    const theme_palette_t *p = s_active_config->palette;

    lv_theme_t *default_th = lv_theme_default_init(
        disp,
        p->primary,
        p->secondary,
        s_active_config->is_dark,
        &inter_regular_16
    );

    s_custom_theme = lv_theme_create();
    lv_theme_set_parent(s_custom_theme, default_th);
    lv_theme_set_apply_cb(s_custom_theme, theme_apply_cb);

    s_custom_theme->font_small  = &inter_regular_14;
    s_custom_theme->font_normal = &inter_regular_16;
    s_custom_theme->font_large  = &inter_regular_24;

    styles_init(p);

    lv_display_set_theme(disp, s_custom_theme);

    ESP_LOGI(TAG, "theme initialized: %s", s_active_config->name);
}

void theme_set_active(const char *name)
{
    for (size_t i = 0; i < palettes_get_count(); i++) {
        const theme_config_t *cfg = palettes_get_builtin(i);
        if (cfg && cfg->name && strcmp(cfg->name, name) == 0) {
            s_active_config = cfg;
            const theme_palette_t *p = cfg->palette;

            styles_init(p);

            lv_display_t *disp = lv_display_get_default();
            lv_theme_t *default_th = lv_theme_default_init(
                disp,
                p->primary,
                p->secondary,
                cfg->is_dark,
                &inter_regular_16
            );

            lv_theme_set_parent(s_custom_theme, default_th);
            lv_display_set_theme(disp, s_custom_theme);

            ESP_LOGI(TAG, "theme set to: %s", name);
            return;
        }
    }
    ESP_LOGW(TAG, "theme_set_active: unknown theme '%s'", name);
}

const theme_palette_t *theme_get_palette(void)
{
    return s_active_config ? s_active_config->palette : NULL;
}

bool theme_is_dark(void)
{
    return s_active_config ? s_active_config->is_dark : false;
}
