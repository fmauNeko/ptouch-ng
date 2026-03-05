#include "nav_shell.h"
#include "screen_manager.h"
#include "theme.h"
#include "fonts.h"
#include "material_symbols.h"
#include "esp_log.h"

static const char *TAG = "nav_shell";

static lv_obj_t *s_content = NULL;
static lv_obj_t *s_tab_btns[SCREEN__COUNT];

extern void status_bar_create(lv_obj_t *parent);

typedef struct
{
    const char *icon;
    screen_id_t screen;
} tab_def_t;

static const tab_def_t TAB_DEFS[SCREEN__COUNT] = {
    {ICON_HOME, SCREEN_HOME},
    {ICON_TUNE, SCREEN_CONTROL},
    {ICON_FOLDER, SCREEN_FILES},
    {ICON_CAMERA_ALT, SCREEN_CAMERA},
    {ICON_SETTINGS, SCREEN_SETTINGS},
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

    /* root: full-screen column — status bar on top, body row below */
    lv_obj_t *root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
    if (p)
    {
        lv_obj_set_style_bg_color(root, p->bg, 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    }
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(root, 0, 0);

    /* ── status bar: full width, 24px ── */
    lv_obj_t *status_bar = lv_obj_create(root);
    lv_obj_set_size(status_bar, LV_PCT(100), 24);
    status_bar_create(status_bar);

    /* ── body row: sidebar | content ── */
    lv_obj_t *body = lv_obj_create(root);
    lv_obj_set_width(body, LV_PCT(100));
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_style_pad_all(body, 0, 0);
    lv_obj_set_style_pad_column(body, 0, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_radius(body, 0, 0);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_ROW);

    /* ── left sidebar: 64px wide, icon-only ── */
    lv_obj_t *sidebar = lv_obj_create(body);
    lv_obj_remove_flag(sidebar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(sidebar, 64, LV_PCT(100));
    lv_obj_set_style_pad_all(sidebar, 0, 0);
    lv_obj_set_style_border_side(sidebar, LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_style_border_width(sidebar, 1, 0);
    lv_obj_set_style_radius(sidebar, 0, 0);
    if (p)
    {
        lv_obj_set_style_bg_color(sidebar, p->surface, 0);
        lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(sidebar, p->border, 0);
    }
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sidebar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(sidebar, 4, 0);

    lv_color_t inactive_color = p ? p->muted : lv_color_white();
    lv_color_t btn_bg = p ? p->overlay : lv_color_black();

    for (int i = 0; i < SCREEN__COUNT; i++)
    {
        lv_obj_t *btn = lv_obj_create(sidebar);
        lv_obj_remove_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(btn, 56, 56);
        lv_obj_set_style_bg_color(btn, btn_bg, 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, p ? p->border : lv_color_white(), 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_set_style_radius(btn, 10, 0);
        lv_obj_add_event_cb(btn, tab_clicked_cb, LV_EVENT_CLICKED,
                            (void *)(intptr_t)i);

        lv_obj_t *icon = lv_label_create(btn);
        lv_label_set_text(icon, TAB_DEFS[i].icon);
        lv_obj_set_style_text_font(icon, &material_symbols_40, 0);
        lv_obj_set_style_text_color(icon, inactive_color, 0);
        lv_obj_center(icon);

        s_tab_btns[i] = btn;
    }

    /* ── content area: fills remaining space ── */
    s_content = lv_obj_create(body);
    lv_obj_set_height(s_content, LV_PCT(100));
    lv_obj_set_flex_grow(s_content, 1);
    lv_obj_set_style_pad_all(s_content, 0, 0);
    lv_obj_set_style_border_width(s_content, 0, 0);
    lv_obj_set_style_radius(s_content, 0, 0);
    if (p)
    {
        lv_obj_set_style_bg_color(s_content, p->bg, 0);
        lv_obj_set_style_bg_opa(s_content, LV_OPA_COVER, 0);
    }

    ESP_LOGI(TAG, "nav shell created (left sidebar)");
}

void nav_shell_set_active_tab(int tab_index)
{
    const theme_palette_t *p = theme_get_palette();
    lv_color_t active_color = p ? p->primary : lv_color_white();
    lv_color_t inactive_color = p ? p->muted : lv_color_white();

    for (int i = 0; i < SCREEN__COUNT; i++)
    {
        if (!s_tab_btns[i])
        {
            continue;
        }
        lv_color_t color = (i == tab_index) ? active_color : inactive_color;
        lv_obj_t *icon = lv_obj_get_child(s_tab_btns[i], 0);
        if (icon)
        {
            lv_obj_set_style_text_color(icon, color, 0);
        }

        /* highlight active tab, restore inactive to overlay bg */
        if (i == tab_index)
        {
            if (p)
            {
                lv_obj_set_style_bg_color(s_tab_btns[i], p->primary, 0);
            }
            lv_obj_set_style_bg_opa(s_tab_btns[i], LV_OPA_20, 0);
        }
        else
        {
            if (p)
            {
                lv_obj_set_style_bg_color(s_tab_btns[i], p->overlay, 0);
            }
            lv_obj_set_style_bg_opa(s_tab_btns[i], LV_OPA_COVER, 0);
        }
    }
}

lv_obj_t *nav_shell_get_content(void)
{
    return s_content;
}
