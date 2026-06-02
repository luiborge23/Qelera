// ============================================================
// sensor_collector.c — BLE 5.0 Sensor Data Collection
// ============================================================
// Collects data from connected BLE sensors (IMU, temperature,
// humidity, pressure) and formats for transmission
// ============================================================

#include "sensor_collector.h"
#include "esp_log.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

static const char *TAG = "SENSOR_COLLECTOR";

// BLE service UUIDs (128-bit)
static const ble_uuid128_t svc_uuid = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x00, 0x00, 0x40, 0x3e
);

// Characteristic UUIDs
static const ble_uuid128_t char_sensor_uuid = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe1,
    0x93, 0xf3, 0xa3, 0xb5, 0x00, 0x00, 0x41, 0x3e
);

// Sensor data buffer
static sensor_data_t s_current_data;
static bool s_data_updated = false;

// BLE connection state
static bool s_ble_connected = false;
static int s_conn_handle = -1;

/**
 * BLE event handler
 */
static int ble_event_handler(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                ESP_LOGI(TAG, "BLE connected");
                s_ble_connected = true;
                s_conn_handle = event->connect.conn_handle;
            } else {
                ESP_LOGW(TAG, "BLE connection failed");
                // Resume advertising
                ble_gap_adv_start();
            }
            return 0;
        
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "BLE disconnected");
            s_ble_connected = false;
            s_conn_handle = -1;
            // Resume advertising
            ble_gap_adv_start();
            return 0;
        
        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(TAG, "Advertising complete");
            // Restart advertising
            ble_gap_adv_start();
            return 0;
        
        default:
            return 0;
    }
}

/**
 * BLE read callback
 */
static int ble_sensor_read_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (!s_data_updated) {
        return BLE_ATT_ERR_INSUFFICIENT_AUTHENT;
    }
    
    // Copy sensor data to GATT response
    memcpy(ctxt->om->buf, &s_current_data, sizeof(sensor_data_t));
    om_append(ctxt->om, sizeof(sensor_data_t));
    
    ESP_LOGD(TAG, "Sensor data read by client");
    return 0;
}

/**
 * Initialize BLE sensor service
 */
esp_err_t sensor_collector_init(void)
{
    ESP_LOGI(TAG, "Initializing BLE sensor collector...");
    
    // Initialize NimBLE host
    nimble_port_init();
    
    // Set device name
    ble_svc_gap_device_name_set("VisionTrack-ESP32");
    
    // Initialize GATT
    ble_svc_gatt_init();
    
    // Add custom service
    ble_uuid128_t svc_uuid128 = {
        .u.type = BLE_UUID_TYPE_128,
        .u128 = {
            0x3e, 0x40, 0x00, 0x00, 0xb5, 0xa3, 0xf3, 0x93,
            0xe0, 0xa9, 0xe5, 0x0e, 0x24, 0xdc, 0xca, 0x9e
        }
    };
    
    ble_uuid128_t char_uuid128 = {
        .u.type = BLE_UUID_TYPE_128,
        .u128 = {
            0x3e, 0x41, 0x00, 0x00, 0xb5, 0xa3, 0xf3, 0x93,
            0xe0, 0xa9, 0xe5, 0x0e, 0x24, 0xdc, 0xca, 0x9e
        }
    };
    
    // Start NimBLE port
    nimble_port_freertos_init(ble_event_handler);
    
    // Start advertising
    ble_gap_adv_start();
    
    ESP_LOGI(TAG, "BLE sensor collector initialized");
    return ESP_OK;
}

/**
 * Read sensor data from IMU and other sensors
 */
esp_err_t sensor_collector_read(sensor_data_t *data)
{
    if (!data) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Read accelerometer (simulated - would read from I2C IMU)
    data->accel_x = 0.0f;
    data->accel_y = 0.0f;
    data->accel_z = 9.81f; // Gravity
    
    // Read gyroscope
    data->gyro_x = 0.0f;
    data->gyro_y = 0.0f;
    data->gyro_z = 0.0f;
    
    // Read magnetometer
    data->mag_x = 0.0f;
    data->mag_y = 0.0f;
    data->mag_z = 0.0f;
    
    // Read temperature and humidity (simulated)
    data->temperature = 25.0f + (rand() % 100) / 10.0f;
    data->humidity = 50.0f + (rand() % 200) / 10.0f;
    data->pressure = 1013.25f + (rand() % 1000) / 100.0f - 5.0f;
    
    // Set timestamp
    data->timestamp = esp_timer_get_time();
    
    // Copy to current data
    memcpy(&s_current_data, data, sizeof(sensor_data_t));
    s_data_updated = true;
    
    return ESP_OK;
}

/**
 * Send sensor data to connected BLE client
 */
esp_err_t sensor_collector_send(void)
{
    if (!s_ble_connected || s_conn_handle < 0) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!s_data_updated) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Create output message
    struct os_mbuf *om = os_mbuf_get_pkthdr(sizeof(sensor_data_t), 0);
    if (!om) {
        ESP_LOGE(TAG, "Failed to allocate mbuf");
        return ESP_ERR_NO_MEM;
    }
    
    // Copy data to mbuf
    os_mbuf_append(om, &s_current_data, sizeof(sensor_data_t));
    
    // Send via GATT
    int rc = ble_gattc_notify_custom(s_conn_handle, char_handle, om);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to send sensor data: %d", rc);
        return ESP_FAIL;
    }
    
    s_data_updated = false;
    ESP_LOGD(TAG, "Sensor data sent to BLE client");
    return ESP_OK;
}

/**
 * Get BLE connection status
 */
bool sensor_collector_is_connected(void)
{
    return s_ble_connected;
}

/**
 * Calculate motion intensity from IMU data
 */
float sensor_collector_motion_intensity(sensor_data_t *data)
{
    if (!data) {
        return 0.0f;
    }
    
    // Calculate acceleration magnitude
    float accel_mag = sqrtf(data->accel_x * data->accel_x +
                           data->accel_y * data->accel_y +
                           data->accel_z * data->accel_z);
    
    // Calculate angular velocity magnitude
    float gyro_mag = sqrtf(data->gyro_x * data->gyro_x +
                          data->gyro_y * data->gyro_y +
                          data->gyro_z * data->gyro_z);
    
    // Combine acceleration and gyro for motion intensity
    float intensity = (accel_mag - 9.81f) * 0.5f + gyro_mag * 0.5f;
    
    return intensity;
}
