#include "theme.h"

static theme_palette_t palette_latte;
static theme_palette_t palette_mocha;

static const theme_config_t builtin_themes[] = {
    { .name = "Catppuccin Latte", .is_dark = false, .palette = &palette_latte },
    { .name = "Catppuccin Mocha", .is_dark = true,  .palette = &palette_mocha },
};

#define BUILTIN_THEME_COUNT (sizeof(builtin_themes) / sizeof(builtin_themes[0]))

void palettes_init(void)
{
    palette_latte.bg        = lv_color_hex(0xEFF1F5);
    palette_latte.surface   = lv_color_hex(0xCCD0DA);
    palette_latte.overlay   = lv_color_hex(0xE6E9EF);
    palette_latte.text      = lv_color_hex(0x4C4F69);
    palette_latte.subtext   = lv_color_hex(0x5C5F77);
    palette_latte.muted     = lv_color_hex(0x7C7F93);
    palette_latte.border    = lv_color_hex(0xBCC0CC);
    palette_latte.primary   = lv_color_hex(0x8839EF);
    palette_latte.secondary = lv_color_hex(0x7287FD);
    palette_latte.accent    = lv_color_hex(0xEA76CB);
    palette_latte.error     = lv_color_hex(0xD20F39);
    palette_latte.warning   = lv_color_hex(0xFE640B);
    palette_latte.success   = lv_color_hex(0x40A02B);
    palette_latte.info      = lv_color_hex(0x1E66F5);

    palette_mocha.bg        = lv_color_hex(0x1E1E2E);
    palette_mocha.surface   = lv_color_hex(0x313244);
    palette_mocha.overlay   = lv_color_hex(0x181825);
    palette_mocha.text      = lv_color_hex(0xCDD6F4);
    palette_mocha.subtext   = lv_color_hex(0xBAC2DE);
    palette_mocha.muted     = lv_color_hex(0x9399B2);
    palette_mocha.border    = lv_color_hex(0x45475A);
    palette_mocha.primary   = lv_color_hex(0xCBA6F7);
    palette_mocha.secondary = lv_color_hex(0xB4BEFE);
    palette_mocha.accent    = lv_color_hex(0xF5C2E7);
    palette_mocha.error     = lv_color_hex(0xF38BA8);
    palette_mocha.warning   = lv_color_hex(0xFAB387);
    palette_mocha.success   = lv_color_hex(0xA6E3A1);
    palette_mocha.info      = lv_color_hex(0x89B4FA);
}

const theme_config_t *palettes_get_builtin(size_t index)
{
    if (index >= BUILTIN_THEME_COUNT) {
        return NULL;
    }
    return &builtin_themes[index];
}

size_t palettes_get_count(void)
{
    return BUILTIN_THEME_COUNT;
}
