#include "i18n.h"
#include "esp_log.h"

static const char *TAG = "i18n";

static const char *strings_en[STR__COUNT] = {
    [STR_TAB_HOME]                  = "Home",
    [STR_TAB_FILAMENT]              = "Filament",
    [STR_TAB_FILES]                 = "Files",
    [STR_TAB_CAMERA]                = "Camera",
    [STR_TAB_SETTINGS]              = "Settings",
    [STR_STATUS_NO_PRINTER]         = "No Printer",
    [STR_STATUS_CONNECTED]          = "Connected",
    [STR_STATUS_DISCONNECTED]       = "Disconnected",
    [STR_STATUS_NO_WIFI]            = "No WiFi",
    [STR_STATUS_CLOCK_PLACEHOLDER]  = "12:00",
    [STR_HOME_TITLE]                = "Home",
    [STR_HOME_NO_PRINTER_MSG]       = "No printer connected.\nGo to Settings to add one.",
    [STR_SETTINGS_TITLE]            = "Settings",
    [STR_SETTINGS_THEME]            = "Theme",
    [STR_SETTINGS_THEME_LIGHT]      = "Light",
    [STR_SETTINGS_THEME_DARK]       = "Dark",
    [STR_SETTINGS_LANGUAGE]         = "Language",
    [STR_SETTINGS_ABOUT]            = "About",
    [STR_BTN_OK]                    = "OK",
    [STR_BTN_CANCEL]                = "Cancel",
    [STR_BTN_CONFIRM]               = "Confirm",
    [STR_BTN_CLOSE]                 = "Close",
    [STR_DIALOG_TITLE_INFO]         = "Information",
    [STR_DIALOG_TITLE_WARNING]      = "Warning",
    [STR_DIALOG_TITLE_ERROR]        = "Error",
    [STR_TOAST_THEME_CHANGED]       = "Theme changed",
};

static const char **current_strings = strings_en;

void i18n_init(void)
{
    current_strings = strings_en;
    ESP_LOGI(TAG, "i18n initialized (language: en)");
}

const char *i18n_get(string_id_t id)
{
    if (id < 0 || id >= STR__COUNT || current_strings[id] == NULL) {
        return "???";
    }
    return current_strings[id];
}

void i18n_set_language(const char *lang_code)
{
    ESP_LOGW(TAG, "Language '%s' not supported, using English", lang_code);
    current_strings = strings_en;
}
