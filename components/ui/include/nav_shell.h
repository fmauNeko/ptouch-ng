#pragma once
#include "lvgl.h"

void nav_shell_create(void);
void nav_shell_set_active_tab(int tab_index);
lv_obj_t *nav_shell_get_content(void);
