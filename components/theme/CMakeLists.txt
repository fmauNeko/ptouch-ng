idf_component_register(
    SRCS "src/theme_engine.c"
         "src/theme_palettes.c"
    INCLUDE_DIRS "include" "../../main/fonts"
    REQUIRES lvgl__lvgl main
)
