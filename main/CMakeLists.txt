set(SRC_FILES "main.c" "file_server.c" "mount.c")
set(INCLUDE_FILES ".")


idf_component_register(SRCS ${SRC_FILES}
                       INCLUDE_DIRS ${INCLUDE_FILES}
                       PRIV_REQUIRES esp_http_server nvs_flash esp_driver_gpio esp_netif fatfs esp_event vfs spiffs
                       EMBED_FILES "favicon.ico" "esp_web_flasher_standalone.html"
                       )


