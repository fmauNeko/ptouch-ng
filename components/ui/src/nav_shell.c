#include "nav_shell.h"
#include "screen_manager.h"
#include "theme.h"
#include "i18n.h"
#include "fonts.h"
#include "material_symbols.h"
#include "esp_log.h"

static const char *TAG = "nav_shell";

static lv_obj_t *s_content = NULL;
static lv_obj_t *s_tab_btns[SCREEN__COUNT];

extern void status_bar_create(lv_obj_t *parent);

typedef struct {
    const char *icon;
    string_id_t label_id;
    screen_id_t screen;
} tab_def_t;

static const tab_def_t TAB_DEFS[SCREEN__COUNT] = {
    { ICON_HOME,        STR_TAB_HOME,     SCREEN_HOME     },
    { ICON_LAYERS,      STR_TAB_FILAMENT, SCREEN_FILAMENT },
    { ICON_FOLDER,      STR_TAB_FILES,    SCREEN_FILES    },
    { ICON_CAMERA_ALT,  STR_TAB_CAMERA,   SCREEN_CAMERA   },
    { ICON_SETTINGS,    STR_TAB_SETTINGS, SCREEN_SETTINGS },
};

static void tab_clicked_cb(lv_event_t *e)
{
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    screen_manager_show((screen_id_t)idx);
    nav_shell_set_active_tab(idx);
}

void nav_shell_create(void)
{
    const theme_palette_t *p = theme_get_palette();

    lv_obj_t *root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
    if (p) {
        lv_obj_set_style_bg_color(root, p->bg, 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    }
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(root, LV_FLEX_ALIGN_START, 0);
    lv_obj_set_style_pad_row(root, 0, 0);

    lv_obj_t *status_bar = lv_obj_create(root);
    lv_obj_set_size(status_bar, LV_PCT(100), 40);
    status_bar_create(status_bar);

    s_content = lv_obj_create(root);
    lv_obj_set_width(s_content, LV_PCT(100));
    lv_obj_set_flex_grow(s_content, 1);
    lv_obj_set_style_pad_all(s_content, 0, 0);
    lv_obj_set_style_border_width(s_content, 0, 0);
    lv_obj_set_style_radius(s_content, 0, 0);
    if (p) {
        lv_obj_set_style_bg_color(s_content, p->bg, 0);
        lv_obj_set_style_bg_opa(s_content, LV_OPA_COVER, 0);
    }

    lv_obj_t *nav_bar = lv_obj_create(root);
    lv_obj_set_size(nav_bar, LV_PCT(100), 56);
    lv_obj_set_style_pad_all(nav_bar, 0, 0);
    lv_obj_set_style_border_width(nav_bar, 0, 0);
    lv_obj_set_style_border_side(nav_bar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_width(nav_bar, 1, 0);
    lv_obj_set_style_radius(nav_bar, 0, 0);
    if (p) {
        lv_obj_set_style_bg_color(nav_bar, p->surface, 0);
        lv_obj_set_style_bg_opa(nav_bar, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(nav_bar, p->border, 0);
    }
    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(nav_bar, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    lv_obj_set_style_flex_cross_place(nav_bar, LV_FLEX_ALIGN_CENTER, 0);

    lv_color_t inactive_color = p ? p->muted : lv_color_white();

    for (int i = 0; i < SCREEN__COUNT; i++) {
        lv_obj_t *btn = lv_obj_create(nav_bar);
        lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_pad_hor(btn, 8, 0);
        lv_obj_set_style_pad_ver(btn, 4, 0);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_flex_main_place(btn, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_flex_cross_place(btn, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_row(btn, 2, 0);
        lv_obj_add_event_cb(btn, tab_clicked_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);

        lv_obj_t *icon = lv_label_create(btn);
        lv_label_set_text(icon, TAB_DEFS[i].icon);
        lv_obj_set_style_text_font(icon, &material_symbols_28, 0);
        lv_obj_set_style_text_color(icon, inactive_color, 0);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, i18n_get(TAB_DEFS[i].label_id));
        lv_obj_set_style_text_font(label, &inter_regular_14, 0);
        lv_obj_set_style_text_color(label, inactive_color, 0);

        s_tab_btns[i] = btn;
    }

    ESP_LOGI(TAG, "nav shell created");
}

void nav_shell_set_active_tab(int tab_index)
{
    const theme_palette_t *p = theme_get_palette();
    lv_color_t active_color   = p ? p->primary : lv_color_white();
    lv_color_t inactive_color = p ? p->muted   : lv_color_white();

    for (int i = 0; i < SCREEN__COUNT; i++) {
        if (!s_tab_btns[i]) {
            continue;
        }
        lv_color_t color = (i == tab_index) ? active_color : inactive_color;
        lv_obj_t *icon  = lv_obj_get_child(s_tab_btns[i], 0);
        lv_obj_t *label = lv_obj_get_child(s_tab_btns[i], 1);
        if (icon) {
            lv_obj_set_style_text_color(icon, color, 0);
        }
        if (label) {
            lv_obj_set_style_text_color(label, color, 0);
        }
    }
}

lv_obj_t *nav_shell_get_content(void)
{
    return s_content;
}
