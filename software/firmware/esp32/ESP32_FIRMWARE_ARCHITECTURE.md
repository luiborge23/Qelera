# Qelera ESP32 Firmware — BLE Sensor Gateway v1.0
# ESP-IDF 5.2+, C/C++, BLE 5.0, I2C/SPI sensors
# Created: May 28, 2026

# ============================================================
# PROJECT STRUCTURE
# ============================================================
# esp32-firmware/
# ├── CMakeLists.txt
# ├── sdkconfig.defaults
# ├── partitions.csv
# ├── main/
# │   ├── CMakeLists.txt
# │   ├── main.c                    # Entry point
# │   ├── app_main.c                # Application initialization
# │   ├── ble_gateway.c             # BLE GATT server
# │   ├── sensor_manager.c          # Sensor abstraction layer
# │   ├── wifi_manager.c            # WiFi connection management
# │   ├── mqtt_client.c             # MQTT communication
# │   ├── ota_manager.c             # Over-the-air updates
# │   ├── power_manager.c           # Power optimization
# │   ├── event_loop.c              # FreeRTOS event handling
# │   ├── crypto_utils.c            # Encryption helpers
# │   ├── config_store.c            # NVS configuration storage
# │   ├── logger.c                  # Structured logging
# │   └── utils/
# │       ├── crc16.c               # CRC checksums
# │       ├── base64.c              # Base64 encoding
# │       └── json_parser.c         # Lightweight JSON parsing
# ├── components/
# │   ├── visiontrack_sdk/
# │   │   ├── CMakeLists.txt
# │   │   ├── include/
# │   │   │   └── vt_sdk.h
# │   │   └── vt_sdk.c
# │   ├── sensor_drivers/
# │   │   ├── CMakeLists.txt
# │   │   ├── include/
# │   │   │   └── sensor_types.h
# │   │   └── imu_bmi270.c          # Bosch BMI270 IMU
# │   │   ├── gps_neo_m8n.c         # u-blox NEO-M8N GPS
# │   │   ├── wifi_signal.c         # WiFi signal strength
# │   │   ├── battery_monitor.c     # Battery voltage/percentage
# │   │   └── env_sens_bme680.c     # BME680 temp/hum/pressure/gas
# │   └── network_stack/
# │       ├── CMakeLists.txt
# │       ├── include/
# │       │   └── network_types.h
# │       └── mqtt_handler.c        # MQTT client implementation
# ├── docs/
# │   ├── hardware.md               # Hardware pinout
# │   ├── api.md                    # BLE GATT API
# │   └── ota.md                    # OTA update procedure
# ├── scripts/
# │   ├── flash.sh                  # Flash script
# │   ├── monitor.sh                # Serial monitor
# │   └── ota_server.py             # Local OTA server
# └── README.md

# ============================================================
# PARTITIONS (partitions.csv)
# ============================================================
# # ESP-IDF partition table
# # Name, Type, SubType, Offset, Size, Flags
# nvs,data,nvs,0x9000,24K,
# phy_init,data,phy,0xf000,4K,
# factory,app,factory,0x10000,2M,
# ota_data,ota,ota,0x210000,8K,
# ota_0,app,ota_0,0x220000,1M,
# ota_1,app,ota_1,0x320000,1M,
# spiffs,data,spiffs,0x420000,1M,

# ============================================================
# MAIN APPLICATION (main/app_main.c)
# ============================================================
# #include "freertos/FreeRTOS.h"
# #include "freertos/task.h"
# #include "freertos/event_groups.h"
# #include "esp_system.h"
# #include "esp_wifi.h"
# #include "esp_event.h"
# #include "esp_log.h"
# #include "nvs_flash.h"
# #include "ble_gateway.h"
# #include "sensor_manager.h"
# #include "wifi_manager.h"
# #include "mqtt_client.h"
# #include "ota_manager.h"
# #include "power_manager.h"
# #include "config_store.h"
#
# static const char *TAG = "VISIONTRACK";
# static EventGroupHandle_t s_wifi_event_group;
# #define WIFI_CONNECTED_BIT BIT0
# #define BLE_CONNECTED_BIT BIT1
# #define MQTT_CONNECTED_BIT BIT2
#
# void app_main(void)
# {
#     ESP_LOGI(TAG, "Qelera ESP32 Firmware v1.0.0");
#
#     // Initialize NVS
#     esp_err_t ret = nvs_flash_init();
#     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
#         nvs_flash_erase();
#         nvs_flash_init();
#     }
#
#     // Load configuration
#     config_load();
#
#     // Initialize sensor manager
#     sensor_init();
#
#     // Initialize WiFi
#     wifi_init_sta();
#
#     // Wait for WiFi connection
#     s_wifi_event_group = xEventGroupCreate();
#     xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT,
#                         pdFALSE, pdFALSE, portMAX_DELAY);
#
#     // Initialize BLE gateway
#     ble_gateway_init();
#
#     // Initialize MQTT client
#     mqtt_client_init();
#
#     // Initialize OTA manager
#     ota_manager_init();
#
#     // Initialize power manager
#     power_manager_init();
#
#     // Start main data collection task
#     xTaskCreate(data_collection_task, "data_collection", 4096, NULL, 5, NULL);
#
#     // Start sensor reading task
#     xTaskCreate(sensor_reading_task, "sensor_reading", 4096, NULL, 6, NULL);
#
#     // Start BLE advertising task
#     xTaskCreate(ble_advertising_task, "ble_advertising", 2048, NULL, 3, NULL);
#
#     ESP_LOGI(TAG, "System initialized successfully");
# }
#
# void data_collection_task(void *pvParameters)
# {
#     EventGroupHandle_t wifi_event_group = (EventGroupHandle_t)pvParameters;
#     TickType_t xLastWakeTime = xTaskGetTickCount();
#     const TickType_t xFrequency = pdMS_TO_TICKS(1000);  // Every second
#
#     while (1) {
#         xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
#                           pdFALSE, pdFALSE, portMAX_DELAY);
#
#         // Collect sensor data
#         sensor_data_t sensors = sensor_read_all();
#
#         // Package data
#         vt_packet_t packet;
#         packet.type = PACKET_SENSOR_DATA;
#         packet.timestamp = esp_timer_get_time();
#         packet.data = &sensors;
#
#         // Send via MQTT
#         mqtt_send_packet(&packet);
#
#         xTaskDelayUntil(&xLastWakeTime, xFrequency);
#     }
# }
#
# void sensor_reading_task(void *pvParameters)
# {
#     TickType_t xLastWakeTime = xTaskGetTickCount();
#     const TickType_t xFrequency = pdMS_TO_TICKS(100);  // Every 100ms
#
#     while (1) {
#         // Read IMU data (accelerometer + gyroscope)
#         imu_data_t imu = imu_read();
#
#         // Read GPS data
#         gps_data_t gps = gps_read();
#
#         // Read environmental sensors
#         env_data_t env = bme680_read();
#
#         // Read battery level
#         battery_data_t battery = battery_read();
#
#         // Store in ring buffer
#         sensor_buffer_push(&imu, &gps, &env, &battery);
#
#         xTaskDelayUntil(&xLastWakeTime, xFrequency);
#     }
# }

# ============================================================
# BLE GATEWAY (components/visiontrack_sdk/include/vt_sdk.h)
# ============================================================
# /*
#  * Qelera SDK Header for ESP32
#  * Defines BLE GATT service and characteristics
#  */
#
# #ifndef VT_SDK_H
# #define VT_SDK_H
#
# #include <stdint.h>
# #include <stdbool.h>
#
# // GATT Service UUID
# #define VT_SERVICE_UUID         0x181F  // Battery Service (standard)
# #define VT_CUSTOM_SERVICE_UUID  0xFF00  // Custom Qelera service
#
# // GATT Characteristic UUIDs
# #define VT_CHAR_SENSOR_DATA     0xFF01  // Sensor data stream
# #define VT_CHAR_COMMAND         0xFF02  // Commands from phone
# #define VT_CHAR_STATUS          0xFF03  // Device status
# #define VT_CHAR_CONFIG          0xFF04  // Configuration
# #define VT_CHAR_FIRMWARE        0xFF05  // Firmware version
# #define VT_CHAR_OTA             0xFF06  // OTA update
#
# // Packet types
# #define PACKET_SENSOR_DATA      0x01
# #define PACKET_COMMAND          0x02
# #define PACKET_STATUS           0x03
# #define PACKET_ACK              0x04
# #define PACKET_ERROR            0x05
#
# // Maximum payload size
# #define VT_MAX_PAYLOAD_SIZE     20  // BLE MTU
# #define VT_MAX_BUFFER_SIZE      512
#
# // Data structures
# typedef struct {
#     uint32_t timestamp;
#     uint8_t packet_type;
#     uint8_t sequence_number;
#     uint8_t payload[VT_MAX_PAYLOAD_SIZE];
#     uint16_t payload_length;
#     uint8_t crc[2];
# } vt_packet_t;
#
# typedef struct {
#     float accel_x, accel_y, accel_z;
#     float gyro_x, gyro_y, gyro_z;
#     int32_t temperature;
# } imu_data_t;
#
# typedef struct {
#     double latitude;
#     double longitude;
#     float altitude;
#     uint8_t satellites;
#     float speed;
# } gps_data_t;
#
# typedef struct {
#     float temperature;
#     float humidity;
#     float pressure;
#     uint32_t gas_resistance;
# } env_data_t;
#
# typedef struct {
#     uint8_t percentage;
#     float voltage;
#     bool charging;
# } battery_data_t;
#
# // Sensor data structure
# typedef struct {
#     uint32_t timestamp;
#     imu_data_t imu;
#     gps_data_t gps;
#     env_data_t env;
#     battery_data_t battery;
#     int8_t wifi_rssi;
# } sensor_data_t;
#
# // Function prototypes
# esp_err_t vt_ble_init(void);
# esp_err_t vt_ble_send_packet(vt_packet_t *packet);
# esp_err_t vt_ble_set_command_handler(void (*handler)(vt_packet_t *));
# esp_err_t vt_config_get(char *key, char *value, size_t max_len);
# esp_err_t vt_config_set(const char *key, const char *value);
# const char *vt_firmware_version_get(void);
#
# #endif // VT_SDK_H

# ============================================================
# POWER MANAGEMENT (main/power_manager.c)
# ============================================================
# #include "esp_pm.h"
# #include "esp_sleep.h"
# #include "driver/gpio.h"
# #include "power_manager.h"
#
# static const char *TAG = "POWER";
#
# void power_manager_init(void)
# {
#     // Configure power management
#     esp_pm_config_32k_t pm_config = {
#         .max_freq_mhz = 240,
#         .min_freq_mhz = 40,
#         .light_sleep_enable = true
#     };
#     esp_pm_configure(&pm_config);
#
#     // Configure deep sleep wake sources
#     esp_sleep_enable_timer_wakeup(1000000);  // 1 second
#     esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 1);  // Button wake
#
#     ESP_LOGI(TAG, "Power manager initialized");
# }
#
# void power_enter_deep_sleep(uint64_t wake_time_us)
# {
#     ESP_LOGI(TAG, "Entering deep sleep for %llu us", wake_time_us);
#     esp_sleep_enable_timer_wakeup(wake_time_us);
#     esp_deep_sleep_start();
# }
#
# void power_optimize_for_battery(uint8_t battery_pct)
# {
#     if (battery_pct <= 10) {
#         // Critical: minimal sampling
#         esp_pm_configure(&light_sleep_config);
#     } else if (battery_pct <= 20) {
#         // Low: reduced sampling rate
#         esp_pm_configure(&medium_sleep_config);
#     }
#     // Normal operation for >20%
# }

# ============================================================
# OTA MANAGER (main/ota_manager.c)
# ============================================================
# #include "esp_ota_ops.h"
# #include "esp_partition.h"
# #include "esp_https_ota.h"
# #include "ota_manager.h"
#
# static const char *TAG = "OTA";
#
# esp_err_t ota_manager_init(void)
# {
#     // Get current partition info
#     const esp_partition_t *running = esp_ota_get_running_partition();
#     ESP_LOGI(TAG, "Running firmware on partition: %s", running->label);
#
#     return ESP_OK;
# }
#
# esp_err_t ota_manager_update_from_url(const char *url)
# {
#     esp_http_client_config_t config = {
#         .url = url,
#         .cert_pem = NULL,  // Use system CA bundle
#         .timeout_ms = 30000,
#     };
#
#     esp_err_t err = esp_https_ota(&config);
#     if (err == ESP_OK) {
#         ESP_LOGI(TAG, "OTA successful. Rebooting...");
#         esp_restart();
#     } else {
#         ESP_LOGE(TAG, "OTA failed: %s", esp_err_to_name(err));
#     }
#     return err;
# }
#
# esp_err_t ota_manager_update_from_ble(uint8_t *data, size_t length)
# {
#     // Handle BLE OTA (split into chunks)
#     static uint32_t total_bytes = 0;
#     static uint8_t buffer[VT_MAX_BUFFER_SIZE];
#
#     // Append to buffer
#     memcpy(buffer + total_bytes, data, length);
#     total_bytes += length;
#
#     // When complete, write to OTA partition
#     if (total_bytes >= VT_MAX_BUFFER_SIZE) {
#         esp_partition_write(ota_partition, 0, buffer, VT_MAX_BUFFER_SIZE);
#         total_bytes = 0;
#     }
#
#     return ESP_OK;
# }

# ============================================================
# CMAKE CONFIGURATION (CMakeLists.txt)
# ============================================================
# cmake_minimum_required(VERSION 3.16)
#
# include($ENV{IDF_PATH}/tools/cmake/project.cmake)
#
# set(COMPONENTS main visiontrack_sdk sensor_drivers network_stack)
# set(EXTRA_COMPONENT_DIRS components)
#
# project(qelera-esp32)
#
# # Compiler warnings
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
#
# # Optimization
# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")
#
# # Linker script
# set(LD_FLAGS "${LD_FLAGS} -T esp32.peripherals.ld")

# ============================================================
# FLASH SCRIPT (scripts/flash.sh)
# ============================================================
#!/bin/bash
# set -euo pipefail
#
# PORT="${1:-/dev/ttyUSB0}"
# BAUD="${2:-115200}"
#
# echo "=== Flashing Qelera ESP32 Firmware ==="
# echo "Port: $PORT"
# echo "Baud: $BAUD"
#
# # Build
# echo "Building..."
# idf.py build
#
# # Flash
# echo "Flashing..."
# idf.py -p $PORT -b $BAUD flash
#
# # Monitor
# echo "Opening monitor..."
# idf.py -p $PORT monitor
