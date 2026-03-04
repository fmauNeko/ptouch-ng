#include "screen_manager.h"
#include "i18n.h"

static lv_obj_t *create(lv_obj_t *parent)
{
    lv_obj_t *screen = lv_obj_create(parent);
    lv_obj_set_size(screen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(screen, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);

    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, i18n_get(STR_TAB_CAMERA));
    lv_obj_center(label);

    return screen;
}

static void on_show(void) { }
static void on_hide(void) { }

const screen_handler_t screen_camera_handler = {
    .create  = create,
    .on_show = on_show,
    .on_hide = on_hide,
};
