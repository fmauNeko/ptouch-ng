#pragma once

#include <stdbool.h>

void theme_init(void);
void theme_set_active(const char *name);
bool theme_is_dark(void);
