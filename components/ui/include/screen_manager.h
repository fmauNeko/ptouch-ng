#pragma once

#include "lvgl.h"

typedef enum {
    SCREEN_HOME = 0,
    SCREEN_FILAMENT,
    SCREEN_FILES,
    SCREEN_CAMERA,
    SCREEN_SETTINGS,
    SCREEN__COUNT,
} screen_id_t;

typedef struct {
    lv_obj_t *(*create)(lv_obj_t *parent);
    void (*on_show)(void);
    void (*on_hide)(void);
} screen_handler_t;

void screen_manager_init(lv_obj_t *content_container);
void screen_manager_register(screen_id_t id, const screen_handler_t *handler);
void screen_manager_show(screen_id_t id);
screen_id_t screen_manager_get_active(void);
