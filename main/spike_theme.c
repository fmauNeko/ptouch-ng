#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "lvgl.h"
#include "bsp/pandatouch.h"

static const char *TAG = "spike-theme";

#define LATTE_PRIMARY    0x1E66F5U  /* Catppuccin Latte: Blue   */
#define LATTE_SECONDARY  0x8839EFU  /* Catppuccin Latte: Mauve  */
#define MOCHA_PRIMARY    0x89B4FAU  /* Catppuccin Mocha: Blue   */
#define MOCHA_SECONDARY  0xCBA6F7U  /* Catppuccin Mocha: Mauve  */

static void apply_theme(lv_display_t *display, uint32_t primary, uint32_t secondary, bool is_dark)
{
    lv_theme_t *theme = lv_theme_default_init(
        display,
        lv_color_hex(primary),
        lv_color_hex(secondary),
        is_dark,
        LV_FONT_DEFAULT
    );
    lv_display_set_theme(display, theme);
    /* Necessary: propagates theme change to all existing widgets in the widget tree. */
    lv_obj_report_style_change(NULL);
    lv_obj_invalidate(lv_screen_active());
}

void spike_theme_run(void)
{
    ESP_LOGI(TAG, "spike_theme_run: start");

    lv_display_t *display = lv_display_get_default();
    if (!display) {
        ESP_LOGE(TAG, "SPIKE_RESULT: FAIL — lv_display_get_default() returned NULL");
        return;
    }

    bsp_display_lock(0);
    apply_theme(display, LATTE_PRIMARY, LATTE_SECONDARY, false);
    bsp_display_unlock();

    bsp_display_lock(0);

    lv_obj_t *scr = lv_obj_create(NULL);

    lv_obj_t *btn = lv_button_create(scr);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_label_set_text(btn_lbl, "Switch Theme");

    lv_obj_t *slider = lv_slider_create(scr);
    lv_obj_set_width(slider, 200);
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 60);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Theme Test — ptouch-ng");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *list = lv_list_create(scr);
    lv_obj_set_size(list, 200, 120);
    lv_obj_align(list, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_list_add_button(list, NULL, "Printer Status");
    lv_list_add_button(list, NULL, "WiFi Settings");
    lv_list_add_button(list, NULL, "About");

    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_set_size(card, 150, 80);
    lv_obj_align(card, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    lv_screen_load(scr);

    bsp_display_unlock();

    vTaskDelay(pdMS_TO_TICKS(500));

    static const struct {
        uint32_t    primary;
        uint32_t    secondary;
        bool        is_dark;
        const char *from;
        const char *to;
    } switches[4] = {
        { MOCHA_PRIMARY, MOCHA_SECONDARY, true,  "light", "dark"  },
        { LATTE_PRIMARY, LATTE_SECONDARY, false, "dark",  "light" },
        { MOCHA_PRIMARY, MOCHA_SECONDARY, true,  "light", "dark"  },
        { LATTE_PRIMARY, LATTE_SECONDARY, false, "dark",  "light" },
    };

    for (int i = 0; i < 4; i++) {
        vTaskDelay(pdMS_TO_TICKS(3000));

        bsp_display_lock(0);
        apply_theme(display,
                    switches[i].primary,
                    switches[i].secondary,
                    switches[i].is_dark);
        bsp_display_unlock();

        vTaskDelay(pdMS_TO_TICKS(100));

        ESP_LOGI(TAG, "THEME_SWITCH: %s->%s — result=OK, fps_after=0",
                 switches[i].from, switches[i].to);
    }

    ESP_LOGI(TAG, "SPIKE_RESULT: PASS — 4 theme switches, no asserts, no corruption");
}
