#include "ui.h"
#include "nav_shell.h"
#include "screen_manager.h"
#include "i18n.h"
#include "esp_log.h"

static const char *TAG = "ui";

extern const screen_handler_t screen_home_handler;
extern const screen_handler_t screen_filament_handler;
extern const screen_handler_t screen_files_handler;
extern const screen_handler_t screen_camera_handler;
extern const screen_handler_t screen_settings_handler;

void ui_init(void)
{
    i18n_init();
    nav_shell_create();

    lv_obj_t *content = nav_shell_get_content();
    screen_manager_init(content);

    screen_manager_register(SCREEN_HOME,     &screen_home_handler);
    screen_manager_register(SCREEN_FILAMENT, &screen_filament_handler);
    screen_manager_register(SCREEN_FILES,    &screen_files_handler);
    screen_manager_register(SCREEN_CAMERA,   &screen_camera_handler);
    screen_manager_register(SCREEN_SETTINGS, &screen_settings_handler);

    screen_manager_show(SCREEN_HOME);
    nav_shell_set_active_tab(0);

    ESP_LOGI(TAG, "ui initialized");
}
