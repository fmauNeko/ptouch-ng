#pragma once

#include "lvgl.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    lv_color_t bg;
    lv_color_t surface;
    lv_color_t overlay;
    lv_color_t text;
    lv_color_t subtext;
    lv_color_t muted;
    lv_color_t border;
    lv_color_t primary;
    lv_color_t secondary;
    lv_color_t accent;
    lv_color_t error;
    lv_color_t warning;
    lv_color_t success;
    lv_color_t info;
} theme_palette_t;

typedef struct {
    const char *name;
    bool is_dark;
    const theme_palette_t *palette;
} theme_config_t;

void palettes_init(void);
const theme_config_t *palettes_get_builtin(size_t index);
size_t palettes_get_count(void);

void theme_init(void);
void theme_set_active(const char *name);
const theme_palette_t *theme_get_palette(void);
bool theme_is_dark(void);
