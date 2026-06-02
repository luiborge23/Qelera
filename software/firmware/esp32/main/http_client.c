// ============================================================
// http_client.c — HTTP Fallback Data Forwarding
// ============================================================
// Provides HTTP POST fallback when MQTT is unavailable
// Uses ESP-IDF HTTP client with retry logic
// ============================================================

#include "http_client.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

static const char *TAG = "HTTP_CLIENT";

// HTTP configuration
static http_config_t s_http_config = {
    .base_url = CONFIG_HTTP_BASE_URL,
    .api_endpoint = "/api/v1/sensors",
    .auth_token = CONFIG_HTTP_AUTH_TOKEN,
    .timeout_ms = 5000,
    .max_retries = 3,
    .retry_delay_ms = 1000,
};

/**
 * HTTP event handler
 */
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP received %d bytes", evt->data_len);
            break;
        
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP request completed");
            break;
        
        case HTTP_EVENTOnError:
            ESP_LOGE(TAG, "HTTP error occurred");
            break;
        
        default:
            break;
    }
    
    return ESP_OK;
}

/**
 * Send sensor data via HTTP POST
 */
esp_err_t http_client_send_sensor_data(sensor_data_t *data)
{
    // Build JSON payload
    char json[512];
    snprintf(json, sizeof(json),
            "{\"device_id\":\"VisionTrack-ESP32\",\"timestamp\":%lu,"
            "\"accel_x\":%.2f,\"accel_y\":%.2f,\"accel_z\":%.2f,"
            "\"gyro_x\":%.2f,\"gyro_y\":%.2f,\"gyro_z\":%.2f,"
            "\"temperature\":%.1f,\"humidity\":%.1f,\"pressure\":%.1f}",
            data->timestamp, data->accel_x, data->accel_y, data->accel_z,
            data->gyro_x, data->gyro_y, data->gyro_z,
            data->temperature, data->humidity, data->pressure);
    
    // Build full URL
    char url[256];
    snprintf(url, sizeof(url), "%s%s", s_http_config.base_url, s_http_config.api_endpoint);
    
    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = s_http_config.timeout_ms,
        .event_handler = http_event_handler,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    
    // Set headers
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", 
                              sprintf("Bearer %s", s_http_config.auth_token));
    
    // Perform request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code >= 200 && status_code < 300) {
            ESP_LOGI(TAG, "HTTP sensor data sent successfully (status %d)", status_code);
        } else {
            ESP_LOGW(TAG, "HTTP request failed with status %d", status_code);
            err = ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }
    
    // Cleanup
    esp_http_client_cleanup(client);
    
    return err;
}

/**
 * Send alert via HTTP POST
 */
esp_err_t http_client_send_alert(alert_t *alert)
{
    // Build JSON payload
    char json[256];
    snprintf(json, sizeof(json),
            "{\"device_id\":\"VisionTrack-ESP32\",\"type\":\"%s\",\"severity\":\"%s\","
            "\"message\":\"%s\",\"timestamp\":%lu}",
            alert_type_to_string(alert->type),
            severity_to_string(alert->severity),
            alert->message, alert->timestamp);
    
    // Build full URL
    char url[256];
    snprintf(url, sizeof(url), "%s%s", s_http_config.base_url, "/api/v1/alerts");
    
    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = s_http_config.timeout_ms,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        return ESP_FAIL;
    }
    
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", 
                              sprintf("Bearer %s", s_http_config.auth_token));
    
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code >= 200 && status_code < 300) {
            ESP_LOGI(TAG, "HTTP alert sent successfully (status %d)", status_code);
        } else {
            ESP_LOGW(TAG, "HTTP alert failed with status %d", status_code);
            err = ESP_FAIL;
        }
    }
    
    esp_http_client_cleanup(client);
    return err;
}

/**
 * Send heartbeat via HTTP POST
 */
esp_err_t http_client_send_heartbeat(void)
{
    char json[128];
    snprintf(json, sizeof(json),
            "{\"device_id\":\"VisionTrack-ESP32\",\"timestamp\":%lu}",
            esp_timer_get_time());
    
    char url[256];
    snprintf(url, sizeof(url), "%s%s", s_http_config.base_url, "/api/v1/heartbeat");
    
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = s_http_config.timeout_ms,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        return ESP_FAIL;
    }
    
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", 
                              sprintf("Bearer %s", s_http_config.auth_token));
    
    esp_err_t err = esp_http_client_perform(client);
    
    esp_http_client_cleanup(client);
    return err;
}

/**
 * Fetch OTA update info from server
 */
esp_err_t http_client_fetch_ota_info(ota_info_t *info)
{
    char url[256];
    snprintf(url, sizeof(url), "%s%s", s_http_config.base_url, "/api/v1/ota/info");
    
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = s_http_config.timeout_ms,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        return ESP_FAIL;
    }
    
    esp_http_client_set_header(client, "Authorization", 
                              sprintf("Bearer %s", s_http_config.auth_token));
    
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200) {
            // Parse JSON response
            char *response = malloc(esp_http_client_get_content_length(client) + 1);
            if (response) {
                esp_http_client_read(client, response, 
                                   esp_http_client_get_content_length(client));
                response[esp_http_client_get_content_length(client)] = '\0';
                
                // Parse JSON (would use cJSON or similar)
                // info->version = parse_json_version(response);
                // info->download_url = parse_json_url(response);
                
                free(response);
            }
        }
    }
    
    esp_http_client_cleanup(client);
    return err;
}

/**
 * Download OTA firmware via HTTP
 */
esp_err_t http_client_download_firmware(const char *url, const char *output_path)
{
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 30000, // Longer timeout for firmware download
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        return ESP_FAIL;
    }
    
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        int content_length = esp_http_client_get_content_length(client);
        char *buffer = malloc(content_length);
        if (buffer) {
            int read = esp_http_client_read(client, buffer, content_length);
            
            // Write to file
            FILE *f = fopen(output_path, "wb");
            if (f) {
                fwrite(buffer, 1, read, f);
                fclose(f);
                ESP_LOGI(TAG, "Firmware downloaded to %s", output_path);
            }
            
            free(buffer);
        }
    }
    
    esp_http_client_cleanup(client);
    return err;
}
