#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_err.h"

#include "esp_loader.h"
#include "esp32_port.h"  // replace with the port header for your platform

static esp_loader_t loader;
static esp32_port_t port;
static esp_loader_flash_cfg_t flash_cfg;
static const char *FLASH_TAG = "ESP_SERIAL_FLASHER";

static void init_serial_flasher_static_variables(void) {
  // 1. Fill in the port struct with hardware parameters
  esp32_port_t port = {
    .port.ops          = &esp32_uart_ops,
    .baud_rate         = 115200,
    .uart_port         = UART_NUM_1,
    .uart_rx_pin       = GPIO_NUM_5,
    .uart_tx_pin       = GPIO_NUM_4,
    .reset_pin = GPIO_NUM_25,
    .boot_pin  = GPIO_NUM_26,
  };

  esp_loader_flash_cfg_t flash_cfg = {
    .offset     = 0x10000,
    .image_size = 65536,
    .block_size = 4096,
  };

}


esp_loader_error_t serial_flasher_init(void) {
  esp_loader_error_t err = ESP_LOADER_SUCCESS;
  init_serial_flasher_static_variables();
  // 2. Initialize the loader context — binds the protocol and port vtable
  err = esp_loader_init_serial(&loader, &port.port);
  return err;
}

// 3. Connect to the target chip
static esp_loader_error_t serial_flasher_connect(void) {
  esp_loader_error_t err = ESP_LOADER_SUCCESS;
  esp_loader_connect_args_t connect_args = ESP_LOADER_CONNECT_DEFAULT();
  err = esp_loader_connect(&loader, &connect_args);
  return err;
}

// 4. Flash a binary (example: 64KB at 0x10000)
// Variable holding your binary image. Typical sources:
// - Read from storage (SD card, filesystem, flash)
// - Received over a link (UART/SPI/USB/Wi-Fi) into a RAM buffer
// - Compiled-in C array generated from a .bin
//const uint8_t *data = /* pointer to your firmware image buffer */;

esp_err_t serial_flasher_flash(uint8_t *data) {
  esp_loader_error_t err = ESP_LOADER_SUCCESS;
  esp_err_t ret = ESP_OK;

  ESP_LOGI(FLASH_TAG, "Connecting to serial flasher...");
  err = serial_flasher_connect();
  if (err != ESP_LOADER_SUCCESS) {
    ESP_LOGE(FLASH_TAG, "Failed to connect to esp-serial-flasher device.");
    return ESP_FAIL;
  }
  ESP_LOGI(FLASH_TAG, "Successfully connected to serial flasher! Starting flash...");
  err = esp_loader_flash_start(&loader, &flash_cfg);
  if (err != ESP_LOADER_SUCCESS) {
    ESP_LOGE(FLASH_TAG, "Failed to start esp_loader_flash_start.");
    return ESP_FAIL;
  }
  size_t offset = 0;
  while (offset < flash_cfg.image_size) {
    size_t chunk = MIN(flash_cfg.block_size, flash_cfg.image_size - offset);
    err = esp_loader_flash_write(&loader, &flash_cfg, (void *)(data + offset), chunk);
    if (err != ESP_LOADER_SUCCESS) {
      ESP_LOGE(FLASH_TAG, "Failed to write serial flasher data to device.");
      return ESP_FAIL;
    }
    offset += chunk;
  }

  err = esp_loader_flash_finish(&loader, &flash_cfg);
  if ( err != ESP_LOADER_SUCCESS) {
    ESP_LOGE(FLASH_TAG, "Failed to finish serial flash.");
    ret = ESP_FAIL;
  }
  return ret;
}
