/*
 * SPDX-FileCopyrightText: 2010-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include "sdkconfig.h"
#include "esp_littlefs.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "usb/cdc_host_types.h"
#include "usb/usb_host.h"
#include "usb/cdc_acm_host.h"
#include "usb/vcp_ch34x.h"
#include "usb/vcp_cp210x.h"
#include "usb/vcp_ftdi.h"

#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#include "protocol_examples_common.h"
#include "esp_loader.h"
#include "esp32_port.h"
#include "file_serving_example_common.h"

#include "config.h"

/* rest server */
#define MDNS_INSTANCE "dashboard web server"
#define MDNS_HOST_NAME CONFIG_EXAMPLE_MDNS_HOST_NAME
#define WEB_PAGE_MOUNT_POINT_IN_FS CONFIG_EXAMPLE_WEB_MOUNT_POINT


[[maybe_unused]] static const char *TAG = "esp32_esp32_repro_main";

extern esp_err_t start_rest_server(const char *base_path);
extern esp_err_t serial_flasher_init(esp_loader_t load, esp32_port_t port);
extern esp_err_t serial_flasher_flash(const uint8_t *data, esp_loader_t load);

static void initialise_mdns(void)
{
    mdns_init();
    mdns_hostname_set(MDNS_HOST_NAME);
    mdns_instance_name_set(MDNS_INSTANCE);

    mdns_txt_item_t serviceTxtData[] = {
        {"chip", CONFIG_IDF_TARGET},
        {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}

esp_err_t init_fs(void)
{
#if CONFIG_EXAMPLE_DEPLOY_WEB_PAGES
    esp_vfs_littlefs_conf_t conf = {
        .base_path = WEB_PAGE_MOUNT_POINT_IN_FS,
        .partition_label = "www",
        .format_if_mount_failed = true,
    };
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        esp_littlefs_format(conf.partition_label);
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
#endif // CONFIG_EXAMPLE_DEPLOY_WEB_PAGES
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting main...");
    esp32_port_t port = {
      .port.ops          = &esp32_uart_ops,
      .baud_rate         = 115200,
      .uart_port         = UART_NUM_1,
      .uart_rx_pin       = GPIO_NUM_5,
      .uart_tx_pin       = GPIO_NUM_4,
      .reset_pin = GPIO_NUM_25,
      .boot_pin  = GPIO_NUM_26,
    };

    esp_loader_t loader;
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    serial_flasher_init(loader, port);
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    initialise_mdns();
    netbiosns_init();
    netbiosns_set_name(MDNS_HOST_NAME);
     /* Initialize file storage */
    const char* base_path = "/data";
    ESP_ERROR_CHECK(example_mount_storage(base_path));


    ESP_ERROR_CHECK(example_connect());
    /* Start the file server */
    ESP_ERROR_CHECK(example_start_file_server(base_path));
    ESP_LOGI(TAG, "Repro server started");
//    ESP_ERROR_CHECK(init_fs());
//    ESP_ERROR_CHECK(start_rest_server(WEB_PAGE_MOUNT_POINT_IN_FS));
}
