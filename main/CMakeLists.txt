idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES ui theme
    PRIV_REQUIRES nvs_flash
)
