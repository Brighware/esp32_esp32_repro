set(SRC_FILES "main.c" "rest_server.c" "host_usb_cdc.c" "serial_flasher.c")
set(INCLUDE_FILES ".")


idf_component_register(SRCS ${SRC_FILES}
                       INCLUDE_DIRS ${INCLUDE_FILES}
                       PRIV_REQUIRES esp_http_server nvs_flash esp_driver_gpio
                       )

if(CONFIG_EXAMPLE_DEPLOY_WEB_PAGES)
    set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../front/website")
    if(EXISTS ${WEB_SRC_DIR}/dist)
        littlefs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
    else()
        message(FATAL_ERROR "'${WEB_SRC_DIR}/dist' doesn't exist. Please run 'pnpm build' under '${WEB_SRC_DIR}'")
    endif()
endif()
