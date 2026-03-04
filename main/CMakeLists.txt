idf_component_register(
    SRCS "spike_malloc.c"
         "spike_mqtt.c"
         "spike_littlefs.c"
         "spike_camera.c"
         "spike_theme.c"
         "spike_multi_tls.c"
         "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "certs/ca_cert.pem"
)
