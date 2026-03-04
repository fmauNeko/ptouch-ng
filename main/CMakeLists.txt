idf_component_register(
    SRCS "main.c"
         "fonts/inter_regular_14.c"
         "fonts/inter_regular_16.c"
         "fonts/inter_regular_18.c"
         "fonts/inter_regular_24.c"
         "fonts/inter_regular_48.c"
         "fonts/inter_semibold_14.c"
         "fonts/inter_semibold_16.c"
         "fonts/inter_semibold_18.c"
         "fonts/inter_semibold_24.c"
         "fonts/inter_semibold_48.c"
         "fonts/material_symbols_20.c"
         "fonts/material_symbols_24.c"
         "fonts/material_symbols_28.c"
    INCLUDE_DIRS "." "fonts"
    REQUIRES ui theme
    PRIV_REQUIRES nvs_flash
)
