// ============================================================
// mqtt_client.c — MQTT Data Forwarding
// ============================================================
// Connects to MQTT broker for real-time sensor data transmission
// Includes auto-reconnect, QoS, and message queuing
// ============================================================

#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_mqtt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "string.h"
#include <math.h>

static const char *TAG = "MQTT_CLIENT";

// MQTT configuration
static mqtt_config_t s_mqtt_config = {
    .broker_url = CONFIG_MQTT_BROKER_URL,
    .broker_port = CONFIG_MQTT_BROKER_PORT,
    .client_id = "VisionTrack-ESP32",
    .username = CONFIG_MQTT_USERNAME,
    .password = CONFIG_MQTT_PASSWORD,
    .topic_prefix = "visiontrack/esp32/",
    .qos = MQTT_QOS_1,
    .keep_alive = 60,
    .clean_session = true,
    .ssl_enabled = true,
};

// MQTT client handle
static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static bool s_mqtt_connected = false;
static int s_message_id = 0;

// Message queue for offline buffering
static QueueHandle_t s_message_queue = NULL;
#define MAX_QUEUED_MESSAGES 50

/**
 * MQTT event handler
 */
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            s_mqtt_connected = true;
            
            // Subscribe to commands
            char subscribe_topic[128];
            snprintf(subscribe_topic, sizeof(subscribe_topic), 
                    "%s#", s_mqtt_config.topic_prefix);
            msg_id = esp_mqtt_client_subscribe(client, subscribe_topic, 1);
            ESP_LOGI(TAG, "Subscribed to commands, msg_id=%d", msg_id);
            break;
        
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected");
            s_mqtt_connected = false;
            break;
        
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT subscribed, msg_id=%d", event->msg_id);
            break;
        
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT unsubscribed, msg_id=%d", event->msg_id);
            break;
        
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT published, msg_id=%d", event->msg_id);
            break;
        
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT received data");
            // Process incoming commands
            // (Would parse and execute commands here)
            break;
        
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            break;
        
        default:
            break;
    }
    
    return ESP_OK;
}

/**
 * Initialize MQTT client
 */
esp_err_t mqtt_client_init(void)
{
    ESP_LOGI(TAG, "Initializing MQTT client...");
    
    // Create message queue
    s_message_queue = xQueueCreate(MAX_QUEUED_MESSAGES, sizeof(char *));
    if (!s_message_queue) {
        ESP_LOGE(TAG, "Failed to create message queue");
        return ESP_ERR_NO_MEM;
    }
    
    // Configure MQTT client
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.hostname = s_mqtt_config.broker_url,
        .broker.address.port = s_mqtt_config.broker_port,
        .credentials.client_id = s_mqtt_config.client_id,
        .credentials.username = s_mqtt_config.username,
        .credentials.authentication.password = s_mqtt_config.password,
        .session.keepalive = s_mqtt_config.keep_alive,
        .session.last_will = NULL,
    };
    
    // Create and start MQTT client
    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!s_mqtt_client) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }
    
    // Register event handler
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, 
                                  mqtt_event_handler, NULL);
    
    // Start MQTT client
    esp_err_t ret = esp_mqtt_client_start(s_mqtt_client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "MQTT client initialized");
    return ESP_OK;
}

/**
 * Send sensor data via MQTT
 */
esp_err_t mqtt_client_send_sensor_data(sensor_data_t *data)
{
    if (!s_mqtt_connected || !s_mqtt_client) {
        // Queue message for later
        char *json = malloc(512);
        if (json) {
            snprintf(json, 512, 
                    "{\"timestamp\":%lu,\"accel_x\":%.2f,\"accel_y\":%.2f,\"accel_z\":%.2f,"
                    "\"gyro_x\":%.2f,\"gyro_y\":%.2f,\"gyro_z\":%.2f,"
                    "\"temperature\":%.1f,\"humidity\":%.1f,\"pressure\":%.1f}",
                    data->timestamp, data->accel_x, data->accel_y, data->accel_z,
                    data->gyro_x, data->gyro_y, data->gyro_z,
                    data->temperature, data->humidity, data->pressure);
            
            xQueueSend(s_message_queue, &json, pdMS_TO_TICKS(100));
            ESP_LOGD(TAG, "Message queued for later delivery");
        }
        return ESP_ERR_INVALID_STATE;
    }
    
    // Build JSON payload
    char json[512];
    snprintf(json, sizeof(json),
            "{\"timestamp\":%lu,\"accel_x\":%.2f,\"accel_y\":%.2f,\"accel_z\":%.2f,"
            "\"gyro_x\":%.2f,\"gyro_y\":%.2f,\"gyro_z\":%.2f,"
            "\"temperature\":%.1f,\"humidity\":%.1f,\"pressure\":%.1f}",
            data->timestamp, data->accel_x, data->accel_y, data->accel_z,
            data->gyro_x, data->gyro_y, data->gyro_z,
            data->temperature, data->humidity, data->pressure);
    
    // Publish to topic
    char topic[128];
    snprintf(topic, sizeof(topic), "%s%s/sensors", 
            s_mqtt_config.topic_prefix, "node1");
    
    int msg_id = esp_mqtt_client_publish(s_mqtt_client, topic, json, strlen(json),
                                        s_mqtt_config.qos, 0);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to publish message");
        return ESP_FAIL;
    }
    
    ESP_LOGD(TAG, "Sensor data published, msg_id=%d", msg_id);
    return ESP_OK;
}

/**
 * Send alert via MQTT
 */
esp_err_t mqtt_client_send_alert(alert_t *alert)
{
    if (!s_mqtt_connected || !s_mqtt_client) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Build JSON payload
    char json[256];
    snprintf(json, sizeof(json),
            "{\"type\":\"%s\",\"severity\":\"%s\",\"message\":\"%s\",\"timestamp\":%lu}",
            alert_type_to_string(alert->type),
            severity_to_string(alert->severity),
            alert->message, alert->timestamp);
    
    // Publish to alert topic
    char topic[128];
    snprintf(topic, sizeof(topic), "%s%s/alerts", 
            s_mqtt_config.topic_prefix, "node1");
    
    int msg_id = esp_mqtt_client_publish(s_mqtt_client, topic, json, strlen(json),
                                        s_mqtt_config.qos, 1); // Retained
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to publish alert");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Alert published, msg_id=%d", msg_id);
    return ESP_OK;
}

/**
 * Send heartbeat
 */
esp_err_t mqtt_client_send_heartbeat(void)
{
    if (!s_mqtt_connected || !s_mqtt_client) {
        return ESP_ERR_INVALID_STATE;
    }
    
    char json[128];
    snprintf(json, sizeof(json),
            "{\"client_id\":\"%s\",\"timestamp\":%lu,\"state\":\"running\"}",
            s_mqtt_config.client_id, esp_timer_get_time());
    
    char topic[128];
    snprintf(topic, sizeof(topic), "%s%s/heartbeat", 
            s_mqtt_config.topic_prefix, "node1");
    
    int msg_id = esp_mqtt_client_publish(s_mqtt_client, topic, json, strlen(json),
                                        0, 0);
    
    return msg_id >= 0 ? ESP_OK : ESP_FAIL;
}

/**
 * Reconnect MQTT client
 */
esp_err_t mqtt_client_reconnect(void)
{
    if (s_mqtt_client) {
        esp_mqtt_client_stop(s_mqtt_client);
        esp_mqtt_client_destroy(s_mqtt_client);
    }
    
    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!s_mqtt_client) {
        ESP_LOGE(TAG, "Failed to reinitialize MQTT client");
        return ESP_FAIL;
    }
    
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, 
                                  mqtt_event_handler, NULL);
    
    esp_err_t ret = esp_mqtt_client_start(s_mqtt_client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to restart MQTT client: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "MQTT reconnection initiated");
    return ESP_OK;
}

/**
 * Check MQTT connection status
 */
bool mqtt_client_is_connected(void)
{
    return s_mqtt_connected;
}

/**
 * Process queued messages
 */
void mqtt_client_process_queue(void)
{
    char *json = NULL;
    
    while (xQueueReceive(s_message_queue, &json, 0) == pdTRUE) {
        if (s_mqtt_connected && s_mqtt_client) {
            char topic[128];
            snprintf(topic, sizeof(topic), "%s%s/sensors", 
                    s_mqtt_config.topic_prefix, "node1");
            
            esp_mqtt_client_publish(s_mqtt_client, topic, json, strlen(json),
                                   s_mqtt_config.qos, 0);
        }
        
        free(json);
    }
}

/**
 * String conversion helpers
 */
const char *alert_type_to_string(alert_type_t type)
{
    switch (type) {
        case ALERT_MOTION: return "motion";
        case ALERT_TAMPER: return "tamper";
        case ALERT_BATTERY_LOW: return "battery_low";
        case ALERT_CONNECTION_LOST: return "connection_lost";
        default: return "unknown";
    }
}

const char *severity_to_string(severity_t severity)
{
    switch (severity) {
        case SEVERITY_LOW: return "low";
        case SEVERITY_MEDIUM: return "medium";
        case SEVERITY_HIGH: return "high";
        case SEVERITY_CRITICAL: return "critical";
        default: return "unknown";
    }
}
