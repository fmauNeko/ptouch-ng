idf_component_register(
    SRCS "spike_malloc.c"
         "spike_mqtt.c"
         "spike_littlefs.c"
         "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "certs/ca_cert.pem"
)
