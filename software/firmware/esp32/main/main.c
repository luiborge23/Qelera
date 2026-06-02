# ============================================================
# ESP32 Firmware — VisionTrack Edge Sensor Node
# ============================================================
# ESP-IDF 5.2+ | BLE 5.0 | MQTT | OTA | Low-Power
# ============================================================

# ============================================================
# File: main/CMakeLists.txt — Component Build Configuration
# ============================================================

idf_component_register(
    SRCS "main.c" "sensor_collector.c" "motion_detector.c" 
         "power_manager.c" "mqtt_client.c" "http_client.c" "ota_update.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_bt esp_mqtt esp_https_ota nvs_flash 
             driver esp_event esp_timer esp_sleep
)

# ============================================================
# File: sdkconfig — ESP-IDF Configuration
# ============================================================

# sdkconfig
CONFIG_ESP_TASK_WDT_TIMEOUT_S=30
CONFIG_FREERTOS_UNICORE=y
CONFIG_ESP_SYSTEM_HW_STACK_CHECK=y
CONFIG_ESP_SYSTEM_PANIC_SILENT_REBOOT=y

# WiFi
CONFIG_ESP_WIFI_SSID="VisionTrack_Network"
CONFIG_ESP_WIFI_PASSWORD="visiontrack2026"
CONFIG_ESP_MAXIMUM_RETRY=5

# Bluetooth BLE
CONFIG_BT_ENABLED=y
CONFIG_BT_BLE_ENABLED=y
CONFIG_BT_BLE_42_FEATURES_ENABLED=y
CONFIG_BT_RFCOMM_ENABLED=y
CONFIG_BT_NIMBLE_ENABLED=y

# MQTT
CONFIG_MQTT_PROTOCOL_3_1_1=y
CONFIG_MQTT_TRANSPORT_SSL=y
CONFIG_MQTT_TRANSPORT_WEBSOCKET=y
CONFIG_MQTT_NETWORK_WRITE_TIMEOUT_MS=5000
CONFIG_MQTT_REPORT_DELETED_MESSAGES=y
CONFIG_MQTT_SKIP_PUBLISH_IF_DISCONNECTED=y
CONFIG_MQTT_TASK_CORE_SELECTION_ENABLED=y

# OTA
CONFIG_APP_RESTART_AFTER_REBOOT=y
CONFIG_ESP_HTTPS_OTA_ALLOW_HTTP=y

# Power Management
CONFIG_ESP_SLEEP_POWER_DOWN_FLASH=y
CONFIG_ESP_SLEEP_FLASH_LEAKAGE_FIX=y
CONFIG_ESP32_DEEP_SLEEP_WAKEUP_DELAY=100000

# Log Level
CONFIG_LOG_DEFAULT_LEVEL_INFO=y
CONFIG_LOG_MAXIMUM_LEVEL_DEBUG=y

# ============================================================
# File: main/main.c — Main Entry Point
# ============================================================

/**
 * main.c - VisionTrack ESP32 Edge Sensor Node
 * 
 * Primary functions:
 * - BLE 5.0 peripheral for sensor data collection
 * - WiFi connectivity for MQTT/HTTP data forwarding
 * - Low-power sleep modes with periodic wake
 * - OTA firmware updates
 * - Motion-triggered data collection
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "ota_update.h"
#include "sensor_collector.h"
#include "motion_detector.h"
#include "power_manager.h"
#include "http_client.h"

static const char *TAG = "VISIONTRACK_ESP32";
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_count = 0;

// WiFi connection event bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/**
 * WiFi event handler
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_count < CONFIG_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_count++;
            ESP_LOGI(TAG, "Retrying WiFi connection... (%d/%d)", s_retry_count, CONFIG_ESP_MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "WiFi connection failed after %d retries", CONFIG_ESP_MAXIMUM_RETRY);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_count = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * Initialize WiFi
 */
static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi STA started. SSID: %s", CONFIG_ESP_WIFI_SSID);
}

/**
 * Wait for WiFi connection
 */
static void wait_for_wifi(void)
{
    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP");
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to AP");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Unexpected event");
        esp_restart();
    }
}

/**
 * Main application task
 */
static void app_main(void)
{
    ESP_LOGI(TAG, "VisionTrack ESP32 Edge Sensor Node v1.0.0");
    ESP_LOGI(TAG, "Starting initialization...");

    // Initialize NVS for persistent storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize power manager
    power_manager_init();

    // Initialize sensor collector
    sensor_collector_init();

    // Initialize motion detector
    motion_detector_init();

    // Initialize WiFi
    wifi_init_sta();
    wait_for_wifi();

    // Initialize MQTT client
    mqtt_client_init();

    // Initialize OTA update handler
    ota_update_init();

    ESP_LOGI(TAG, "All subsystems initialized");
    ESP_LOGI(TAG, "Starting main loop...");

    // Main application loop
    while (1) {
        // Check for motion
        motion_state_t motion = motion_detector_check();
        
        if (motion == MOTION_DETECTED) {
            ESP_LOGI(TAG, "Motion detected - collecting sensor data");
            
            // Collect and send sensor data
            sensor_data_t sensors;
            sensor_collector_read(&sensors);
            mqtt_client_send_sensor_data(&sensors);
            
            // Send to HTTP fallback if MQTT fails
            if (mqtt_client_is_connected() == false) {
                http_client_send_sensor_data(&sensors);
            }
        }
        
        // Check for MQTT connection
        if (mqtt_client_is_connected() == false) {
            ESP_LOGW(TAG, "MQTT disconnected - attempting reconnect");
            mqtt_client_reconnect();
        }
        
        // Check for pending OTA updates
        ota_update_check();

        // Enter light sleep between checks
        power_manager_enter_light_sleep(1000); // 1 second sleep
    }
}
