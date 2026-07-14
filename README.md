# esp32x2_repro
`ESP32^2 Repro`
*Reprogram ESP32 devices via an ESP32 device.*

## Requirements
* Target device: ESP32S3
* Framework: ESP-IDF v6.1

## Definition of terms
- `ToolESP`: The ESP32S3 device running this firmware. Used to reprogram other devices.
- `TargetESP`: The ESP32 device being reprogrammed.

## Structure
- `WiFi AP` -> AP running on ESP32 (ToolESP)
- `HTTP Server` -> Hosts html page for repro configuration, firmware upload, and control. (ToolESP)
- `USB CDC` -> Connects ToolESP to TargetESP for reprogramming
- `esptool` -> tool to do the actual reprogramming.

### WiFi AP

#### HTTP Server

#### HTML Page

### USB CDC

#### esptool



## Roadmap
- [ ] WiFi AP
  - [ ] HTTP Server
  - [ ] HTML Page
- [ ] USB-CDC
- [ ] Reprogramming
