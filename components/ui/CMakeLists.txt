idf_component_register(
    SRCS "src/ui_init.c"
         "src/i18n.c"
         "src/screen_manager.c"
         "src/screen_home.c"
         "src/screen_filament.c"
         "src/screen_files.c"
         "src/screen_camera.c"
         "src/screen_settings.c"
         "src/widgets.c"
         "src/dialog.c"
         "src/keyboard.c"
    INCLUDE_DIRS "include" "../../main/fonts"
    REQUIRES lvgl__lvgl fmauneko__pandatouch theme main
)
