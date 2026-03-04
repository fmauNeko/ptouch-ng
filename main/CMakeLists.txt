idf_component_register(
    SRCS "spike_malloc.c" "main.c"
    INCLUDE_DIRS "."
    EMBED_FILES "certs/ca_cert.pem"
)
