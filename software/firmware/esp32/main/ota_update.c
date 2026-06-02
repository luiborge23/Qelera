// ============================================================
// ota_update.c — Over-The-Air Firmware Updates
// ============================================================
// Handles firmware update checks, downloads, and installations
// Uses ESP-IDF HTTPS OTA with rollback support
// ============================================================

#include "ota_update.h"
#include "esp_log.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

static const char *TAG = "OTA_UPDATE";

// OTA configuration
static ota_config_t s_ota_config = {
    .check_interval_ms = 3600000, // Check every hour
    .firmware_version = "1.0.0",
    .server_url = CONFIG_OTA_SERVER_URL,
    .auth_token = CONFIG_OTA_AUTH_TOKEN,
};

// OTA state
static ota_state_t s_ota_state = OTA_IDLE;
static ota_progress_t s_ota_progress = {0};

/**
 * Check for pending OTA updates
 */
esp_err_t ota_update_check(void)
{
    if (s_ota_state != OTA_IDLE) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Checking for OTA updates...");
    
    // Build URL for update check
    char url[256];
    snprintf(url, sizeof(url), "%s/check?version=%s&device_id=VisionTrack-ESP32",
            s_ota_config.server_url, s_ota_config.firmware_version);
    
    // Make HTTP request (would use http_client_fetch_ota_info)
    // For now, simulate update check
    
    // If update available:
    // s_ota_state = OTA_DOWNLOADING;
    // xTaskCreate(ota_update_task, "ota_update", 4096, NULL, 5, NULL);
    
    return ESP_OK;
}

/**
 * Start OTA update process
 */
esp_err_t ota_update_start(const char *update_url)
{
    if (s_ota_state != OTA_IDLE) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Starting OTA update from: %s", update_url);
    
    s_ota_state = OTA_DOWNLOADING;
    s_ota_progress.percentage = 0;
    s_ota_progress.status = OTA_STATUS_DOWNLOADING;
    
    // Configure HTTPS OTA
    esp_http_client_config_t config = {
        .url = update_url,
        .cert_pem = NULL, // Would load CA certificate
        .skip_cert_common_name_check = false,
        .keep_alive_enable = true,
    };
    
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    
    // Start OTA update
    esp_err_t err = esp_https_ota_perform(&ota_config);
    
    if (err == ESP_OK) {
        s_ota_state = OTA_COMPLETE;
        s_ota_progress.percentage = 100;
        s_ota_progress.status = OTA_STATUS_COMPLETE;
        
        ESP_LOGI(TAG, "OTA update completed successfully");
        
        // Schedule restart
        esp_restart();
    } else {
        s_ota_state = OTA_ERROR;
        s_ota_progress.status = OTA_STATUS_ERROR;
        s_ota_progress.error_code = err;
        
        ESP_LOGE(TAG, "OTA update failed: %s", esp_err_to_name(err));
    }
    
    return err;
}

/**
 * OTA update task (runs in background)
 */
static void ota_update_task(void *arg)
{
    // Monitor OTA progress
    while (s_ota_state == OTA_DOWNLOADING) {
        // Get current progress
        s_ota_progress.percentage = esp_https_ota_get_image_len_read() / 
                                   esp_https_ota_get_image_len() * 100;
        
        ESP_LOGI(TAG, "OTA progress: %lu%%", s_ota_progress.percentage);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    vTaskDelete(NULL);
}

/**
 * Get OTA state
 */
ota_state_t ota_update_get_state(void)
{
    return s_ota_state;
}

/**
 * Get OTA progress
 */
ota_progress_t ota_update_get_progress(void)
{
    return s_ota_progress;
}

/**
 * Get OTA error message
 */
const char *ota_update_get_error_message(esp_err_t error)
{
    switch (error) {
        case ESP_ERR_HTTP_EAGAIN:
            return "Connection timeout";
        case ESP_ERR_HTTP_TCP_ERROR:
            return "TCP connection error";
        case ESP_ERR_HTTP_SSL_ERROR:
            return "SSL/TLS error";
        case ESP_ERR_HTTP_RESPONSE_TOO_BIG:
            return "Response too large";
        case ESP_ERR_OTA_BLOCK_SIZE:
            return "Invalid OTA block size";
        default:
            return "Unknown error";
    }
}

/**
 * Initialize OTA update handler
 */
esp_err_t ota_update_init(void)
{
    ESP_LOGI(TAG, "Initializing OTA update handler...");
    
    // Load current version from NVS
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open("ota", NVS_READONLY, &nvs_handle);
    if (ret == ESP_OK) {
        char version[32];
        size_t len = sizeof(version);
        ret = nvs_get_str(nvs_handle, "version", version, &len);
        if (ret == ESP_OK) {
            strncpy(s_ota_config.firmware_version, version, sizeof(s_ota_config.firmware_version) - 1);
        }
        nvs_close(nvs_handle);
    }
    
    ESP_LOGI(TAG, "Current firmware version: %s", s_ota_config.firmware_version);
    
    return ESP_OK;
}

/**
 * Save firmware version to NVS
 */
esp_err_t ota_update_save_version(const char *version)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open("ota", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = nvs_set_str(nvs_handle, "version", version);
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs_handle);
    }
    
    nvs_close(nvs_handle);
    return ret;
}
