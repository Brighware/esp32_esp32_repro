#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "usb/cdc_host_types.h"
#include "usb/usb_host.h"
#include "usb/cdc_acm_host.h"

#include "esp_loader.h"
#include "esp32_port.h"  // replace with the port header for your platform

#include "config.h"


static esp_loader_flash_cfg_t flash_cfg = {
    .offset     = 0x10000,
    .image_size = 65536,
    .block_size = 4096,
  };


static const char *FLASH_TAG = "ESP_SERIAL_FLASHER";

static size_t size_min(size_t a, size_t b) {
  if (a >= b) return b;
  else return a;
}

esp_err_t serial_flasher_init(esp_loader_t load, esp32_port_t port) {
  esp_loader_error_t err = ESP_LOADER_SUCCESS;
  // 2. Initialize the loader context — binds the protocol and port vtable
  err = esp_loader_init_serial(&load, &port.port);
  if ( err != ESP_LOADER_SUCCESS) {
    ESP_LOGE(FLASH_TAG, "Serial flasher initialization failed");
    return ESP_FAIL;
  }
  return ESP_OK;
}

// 3. Connect to the target chip
static esp_loader_error_t serial_flasher_connect(esp_loader_t load) {
  esp_loader_error_t err = ESP_LOADER_SUCCESS;
  esp_loader_connect_args_t connect_args = ESP_LOADER_CONNECT_DEFAULT();
  err = esp_loader_connect(&load, &connect_args);
  return err;
}

// 4. Flash a binary (example: 64KB at 0x10000)
// Variable holding your binary image. Typical sources:
// - Read from storage (SD card, filesystem, flash)
// - Received over a link (UART/SPI/USB/Wi-Fi) into a RAM buffer
// - Compiled-in C array generated from a .bin
//const uint8_t *data = /* pointer to your firmware image buffer */;

esp_err_t serial_flasher_flash(const uint8_t *data, esp_loader_t load) {
  esp_loader_error_t err = ESP_LOADER_SUCCESS;
  esp_err_t ret = ESP_OK;

  ESP_LOGI(FLASH_TAG, "Connecting to serial flasher...");
  err = serial_flasher_connect(load);
  if (err != ESP_LOADER_SUCCESS) {
    ESP_LOGE(FLASH_TAG, "Failed to connect to esp-serial-flasher device.");
    return ESP_FAIL;
  }
  ESP_LOGI(FLASH_TAG, "Successfully connected to serial flasher! Starting flash...");
  err = esp_loader_flash_start(&load, &flash_cfg);
  if (err != ESP_LOADER_SUCCESS) {
    ESP_LOGE(FLASH_TAG, "Failed to start esp_loader_flash_start.");
    return ESP_FAIL;
  }
  size_t offset = 0;
  while (offset < flash_cfg.image_size) {
    size_t chunk = size_min(flash_cfg.block_size, flash_cfg.image_size - offset);
    err = esp_loader_flash_write(&load, &flash_cfg, (void *)(data + offset), chunk);
    if (err != ESP_LOADER_SUCCESS) {
      ESP_LOGE(FLASH_TAG, "Failed to write serial flasher data to device.");
      return ESP_FAIL;
    }
    offset += chunk;
  }

  err = esp_loader_flash_finish(&load, &flash_cfg);
  if ( err != ESP_LOADER_SUCCESS) {
    ESP_LOGE(FLASH_TAG, "Failed to finish serial flash.");
    ret = ESP_FAIL;
  }
  return ret;
}
