#pragma once

#include "lvgl.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    UI_BTN_PRIMARY,
    UI_BTN_SECONDARY,
    UI_BTN_DANGER,
} ui_btn_variant_t;

lv_obj_t *ui_btn_create(lv_obj_t *parent, const char *text, ui_btn_variant_t variant);

lv_obj_t *ui_card_create(lv_obj_t *parent, const char *title);

lv_obj_t *ui_list_create(lv_obj_t *parent);
lv_obj_t *ui_list_add_item(lv_obj_t *list, const char *icon_text, const char *label_text, lv_event_cb_t click_cb);

lv_obj_t *ui_spinner_create(lv_obj_t *parent, int32_t size);

void ui_toast_show(const char *message, uint32_t duration_ms);

typedef enum {
    UI_DIALOG_INFO,
    UI_DIALOG_WARNING,
    UI_DIALOG_ERROR,
    UI_DIALOG_CONFIRM,
} ui_dialog_type_t;

typedef struct {
    ui_dialog_type_t type;
    const char *title;
    const char *message;
    const char *btn_confirm;
    const char *btn_cancel;
    lv_event_cb_t on_confirm;
    lv_event_cb_t on_cancel;
} ui_dialog_config_t;

void ui_dialog_show(const ui_dialog_config_t *config);
void ui_dialog_close(void);

void ui_keyboard_show(lv_obj_t *textarea);
void ui_keyboard_hide(void);
bool ui_keyboard_is_visible(void);
