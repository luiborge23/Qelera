// ============================================================
// motion_detector.c — Motion Detection with IMU
// ============================================================
// Uses accelerometer and gyroscope data to detect motion events
// Configurable thresholds and debounce logic
// ============================================================

#include "motion_detector.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <math.h>

static const char *TAG = "MOTION_DETECTOR";

// Motion detection configuration
static motion_config_t s_config = {
    .accel_threshold = 0.5f,    // m/s² above gravity
    .gyro_threshold = 0.1f,     // rad/s
    .debounce_ms = 500,         // 500ms debounce
    .min_motion_duration_ms = 100, // minimum motion duration
    .idle_timeout_ms = 5000,    // 5s before going idle
};

// Motion state tracking
static motion_state_t s_current_state = MOTION_IDLE;
static uint32_t s_motion_start_time = 0;
static uint32_t s_last_motion_time = 0;
static bool s_motion_active = false;

// I2C IMU handle (BMP388 or similar)
static i2c_port_t s_i2c_port = I2C_NUM_0;
static int s_imu_addr = 0x68; // IMU I2C address

/**
 * Initialize I2C bus for IMU
 */
static esp_err_t i2c_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21, // ESP32 SDA
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = GPIO_NUM_22, // ESP32 SCL
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000, // 400kHz fast mode
    };
    
    esp_err_t ret = i2c_param_config(s_i2c_port, &conf);
    if (ret != ESP_OK) {
        return ret;
    }
    
    return i2c_driver_install(s_i2c_port, conf.mode, 0, 0, 0);
}

/**
 * Read single byte from IMU register
 */
static esp_err_t imu_read_reg(uint8_t reg, uint8_t *data)
{
    uint8_t write_buf[1] = {reg};
    uint8_t read_buf[1] = {0};
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (s_imu_addr << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
    i2c_master_write(cmd, write_buf, 1, I2C_MASTER_ACK);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (s_imu_addr << 1) | I2C_MASTER_READ, I2C_MASTER_ACK);
    i2c_master_read(cmd, read_buf, 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(s_i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        *data = read_buf[0];
    }
    
    return ret;
}

/**
 * Read multiple bytes from IMU
 */
static esp_err_t imu_read_regs(uint8_t reg, uint8_t *data, uint8_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (s_imu_addr << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
    i2c_master_write(cmd, &reg, 1, I2C_MASTER_ACK);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (s_imu_addr << 1) | I2C_MASTER_READ, I2C_MASTER_ACK);
    i2c_master_read(cmd, data, len, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(s_i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

/**
 * Read accelerometer data from IMU
 */
static void imu_read_accel(float *ax, float *ay, float *az)
{
    uint8_t data[6];
    
    // Read 6 bytes of accelerometer data (X, Y, Z - 2 bytes each)
    if (imu_read_regs(0x28, data, 6) == ESP_OK) {
        // Convert to signed 16-bit values
        int16_t raw_x = (int16_t)((data[1] << 8) | data[0]);
        int16_t raw_y = (int16_t)((data[3] << 8) | data[2]);
        int16_t raw_z = (int16_t)((data[5] << 8) | data[4]);
        
        // Convert to m/s² (assuming ±2g range, 16384 LSB/g)
        *ax = raw_x / 16384.0f * 2.0f * 9.81f;
        *ay = raw_y / 16384.0f * 2.0f * 9.81f;
        *az = raw_z / 16384.0f * 2.0f * 9.81f;
    }
}

/**
 * Read gyroscope data from IMU
 */
static void imu_read_gyro(float *gx, float *gy, float *gz)
{
    uint8_t data[6];
    
    // Read 6 bytes of gyroscope data
    if (imu_read_regs(0x2d, data, 6) == ESP_OK) {
        // Convert to signed 16-bit values
        int16_t raw_x = (int16_t)((data[1] << 8) | data[0]);
        int16_t raw_y = (int16_t)((data[3] << 8) | data[2]);
        int16_t raw_z = (int16_t)((data[5] << 8) | data[4]);
        
        // Convert to rad/s (assuming ±250°/s range, 131 LSB/°/s)
        *gx = raw_x / 131.0f * (M_PI / 180.0f);
        *gy = raw_y / 131.0f * (M_PI / 180.0f);
        *gz = raw_z / 131.0f * (M_PI / 180.0f);
    }
}

/**
 * Initialize motion detector
 */
esp_err_t motion_detector_init(void)
{
    ESP_LOGI(TAG, "Initializing motion detector...");
    
    // Initialize I2C bus
    esp_err_t ret = i2c_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Initialize IMU
    // (Would configure IMU registers here)
    
    ESP_LOGI(TAG, "Motion detector initialized");
    return ESP_OK;
}

/**
 * Check for motion and update state
 */
motion_state_t motion_detector_check(void)
{
    float ax, ay, az, gx, gy, gz;
    
    // Read IMU data
    imu_read_accel(&ax, &ay, &az);
    imu_read_gyro(&gx, &gy, &gz);
    
    // Calculate motion intensity
    float accel_mag = sqrtf(ax * ax + ay * ay + az * az);
    float gyro_mag = sqrtf(gx * gx + gy * gy + gz * gz);
    
    // Check if motion exceeds threshold
    bool motion_detected = (accel_mag - 9.81f) > s_config.accel_threshold ||
                          gyro_mag > s_config.gyro_threshold;
    
    uint32_t current_time = esp_timer_get_time() / 1000; // Convert to ms
    
    if (motion_detected) {
        if (s_current_state == MOTION_IDLE) {
            // Motion just started
            s_motion_start_time = current_time;
            s_current_state = MOTION_DETECTED;
            s_motion_active = true;
            ESP_LOGI(TAG, "Motion detected");
        } else if (s_current_state == MOTION_DETECTED) {
            // Motion is ongoing
            s_last_motion_time = current_time;
        }
    } else {
        if (s_current_state == MOTION_DETECTED) {
            // Check debounce
            if (current_time - s_last_motion_time > s_config.debounce_ms) {
                // Motion ended
                s_current_state = MOTION_IDLE;
                s_motion_active = false;
                ESP_LOGD(TAG, "Motion ended");
            }
        }
    }
    
    return s_current_state;
}

/**
 * Get current motion state
 */
motion_state_t motion_detector_get_state(void)
{
    return s_current_state;
}

/**
 * Set motion detection thresholds
 */
void motion_detector_set_thresholds(float accel_threshold, float gyro_threshold)
{
    s_config.accel_threshold = accel_threshold;
    s_config.gyro_threshold = gyro_threshold;
    ESP_LOGI(TAG, "Thresholds updated: accel=%.2f, gyro=%.2f", 
             accel_threshold, gyro_threshold);
}

/**
 * Get motion intensity (0.0 to 1.0)
 */
float motion_detector_get_intensity(void)
{
    float ax, ay, az, gx, gy, gz;
    imu_read_accel(&ax, &ay, &az);
    imu_read_gyro(&gx, &gy, &gz);
    
    float accel_mag = sqrtf(ax * ax + ay * ay + az * az);
    float gyro_mag = sqrtf(gx * gx + gy * gy + gz * gz);
    
    // Normalize to 0.0-1.0 range
    float intensity = ((accel_mag - 9.81f) / s_config.accel_threshold + 
                      gyro_mag / s_config.gyro_threshold) / 2.0f;
    
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;
    
    return intensity;
}

/**
 * Get time since last motion
 */
uint32_t motion_detector_idle_time_ms(void)
{
    uint32_t current_time = esp_timer_get_time() / 1000;
    
    if (s_current_state == MOTION_IDLE) {
        return current_time - s_last_motion_time;
    }
    
    return 0;
}
