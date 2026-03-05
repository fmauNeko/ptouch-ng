#include "ui.h"
#include "nav_shell.h"
#include "screen_manager.h"
#include "i18n.h"
#include "esp_log.h"

static const char *TAG = "ui";

extern const screen_handler_t screen_home_handler;
extern const screen_handler_t screen_control_handler;
extern const screen_handler_t screen_files_handler;
extern const screen_handler_t screen_camera_handler;
extern const screen_handler_t screen_settings_handler;

static void register_screens(void)
{
    screen_manager_register(SCREEN_HOME,     &screen_home_handler);
    screen_manager_register(SCREEN_CONTROL,  &screen_control_handler);
    screen_manager_register(SCREEN_FILES,    &screen_files_handler);
    screen_manager_register(SCREEN_CAMERA,   &screen_camera_handler);
    screen_manager_register(SCREEN_SETTINGS, &screen_settings_handler);
}

void ui_init(void)
{
    i18n_init();
    nav_shell_create();

    lv_obj_t *content = nav_shell_get_content();
    screen_manager_init(content);
    register_screens();

    screen_manager_show(SCREEN_HOME);
    nav_shell_set_active_tab(0);

    ESP_LOGI(TAG, "ui initialized");
}

void ui_rebuild(void)
{
    screen_id_t active = screen_manager_get_active();

    screen_manager_reset();

    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);

    nav_shell_create();

    lv_obj_t *content = nav_shell_get_content();
    screen_manager_init(content);
    register_screens();

    screen_manager_show(active);
    nav_shell_set_active_tab((int)active);

    ESP_LOGI(TAG, "ui rebuilt for theme change");
}
