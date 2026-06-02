// ============================================================
// power_manager.c — Low-Power Sleep Modes
// ============================================================
// Manages ESP32 power states: running, light sleep, deep sleep
// Configurable wake intervals and wake sources
// ============================================================

#include "power_manager.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rtc_io.h"

static const char *TAG = "POWER_MANAGER";

// Power management state
static power_state_t s_current_state = POWER_RUNNING;
static uint32_t s_wake_interval_ms = 1000; // Default 1 second
static bool s_deep_sleep_enabled = true;

// Wake sources
static esp_sleep_wakeup_src_t s_wake_source = ESP_SLEEP_WAKEUP_ALL;

/**
 * Initialize power manager
 */
esp_err_t power_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing power manager...");
    
    // Configure CPU frequency for power saving
    esp_pm_config_32k_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 40,
        .light_sleep_enable = true,
    };
    
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to configure PM: %s", esp_err_to_name(ret));
    }
    
    // Configure wake sources
    // Touch pad wake (optional)
    // rtc_gpio_init(GPIO_NUM_4);
    // esp_sleep_enable_touchpad_wakeup();
    
    // Timer wake (always enabled)
    ESP_LOGI(TAG, "Power manager initialized");
    return ESP_OK;
}

/**
 * Enter light sleep mode
 */
esp_err_t power_manager_enter_light_sleep(uint32_t sleep_time_us)
{
    if (s_current_state == POWER_DEEP_SLEEP) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGD(TAG, "Entering light sleep for %lu us", sleep_time_us);
    
    s_current_state = POWER_LIGHT_SLEEP;
    
    // Enter light sleep
    esp_light_sleep_start();
    
    s_current_state = POWER_RUNNING;
    ESP_LOGD(TAG, "Woke from light sleep");
    
    return ESP_OK;
}

/**
 * Enter deep sleep mode
 */
esp_err_t power_manager_enter_deep_sleep(uint32_t sleep_time_ms)
{
    if (!s_deep_sleep_enabled) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Entering deep sleep for %lu ms", sleep_time_ms);
    
    s_current_state = POWER_DEEP_SLEEP;
    
    // Configure wake timer
    uint64_t wake_time_us = (uint64_t)sleep_time_ms * 1000ULL;
    esp_sleep_enable_timer_wakeup(wake_time_us);
    
    // Configure wake sources
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 1); // Button wake
    // esp_sleep_enable_ext1_wakeup();
    
    // Prepare for deep sleep
    // Power down peripherals
    // uart_flush_all();
    // i2c_driver_delete(I2C_NUM_0);
    
    // Enter deep sleep
    esp_deep_sleep_start();
    
    // Never reaches here
    return ESP_OK;
}

/**
 * Exit deep sleep and restart
 */
void power_manager_exit_deep_sleep(void)
{
    ESP_LOGI(TAG, "Exiting deep sleep");
    
    s_current_state = POWER_RUNNING;
    s_wake_source = esp_sleep_get_wakeup_cause();
    
    // Log wake source
    switch (s_wake_source) {
        case ESP_SLEEP_WAKEUP_TIMER:
            ESP_LOGI(TAG, "Woke from timer");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            ESP_LOGI(TAG, "Woke from ext0");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            ESP_LOGI(TAG, "Woke from ext1");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            ESP_LOGI(TAG, "Woke from touchpad");
            break;
        default:
            ESP_LOGW(TAG, "Unknown wake source: %d", s_wake_source);
            break;
    }
}

/**
 * Get current power state
 */
power_state_t power_manager_get_state(void)
{
    return s_current_state;
}

/**
 * Get wake source (after deep sleep exit)
 */
esp_sleep_wakeup_src_t power_manager_get_wake_source(void)
{
    return s_wake_source;
}

/**
 * Set wake interval
 */
void power_manager_set_wake_interval(uint32_t interval_ms)
{
    s_wake_interval_ms = interval_ms;
    ESP_LOGI(TAG, "Wake interval set to %lu ms", interval_ms);
}

/**
 * Get current wake interval
 */
uint32_t power_manager_get_wake_interval(void)
{
    return s_wake_interval_ms;
}

/**
 * Enable/disable deep sleep
 */
void power_manager_set_deep_sleep_enabled(bool enabled)
{
    s_deep_sleep_enabled = enabled;
    ESP_LOGI(TAG, "Deep sleep %s", enabled ? "enabled" : "disabled");
}

/**
 * Calculate estimated battery life
 */
uint32_t power_manager_estimated_battery_life_hours(power_supply_t *supply)
{
    if (!supply || supply->capacity_mah == 0) {
        return 0;
    }
    
    // Calculate average current consumption
    float avg_current_ma = 0;
    
    // Running state current
    float running_current = supply->running_current_ma;
    float running_time_ratio = 0.1f; // 10% time in running state
    
    // Light sleep current
    float light_sleep_current = supply->light_sleep_current_ma;
    float light_sleep_time_ratio = 0.4f; // 40% time in light sleep
    
    // Deep sleep current
    float deep_sleep_current = supply->deep_sleep_current_ma;
    float deep_sleep_time_ratio = 0.5f; // 50% time in deep sleep
    
    avg_current_ma = running_current * running_time_ratio +
                    light_sleep_current * light_sleep_time_ratio +
                    deep_sleep_current * deep_sleep_time_ratio;
    
    if (avg_current_ma == 0) {
        return 0;
    }
    
    // Calculate battery life in hours
    return (uint32_t)(supply->capacity_mah / avg_current_ma);
}

/**
 * Monitor battery voltage
 */
float power_manager_read_battery_voltage(void)
{
    // Read from ADC channel
    // adc1_config_width(ADC_WIDTH_BIT_12);
    // adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    // int raw = adc1_get_reading(ADC1_CHANNEL_0, 100);
    // float voltage = raw * 3.3f / 4095.0f * 2.0f; // Voltage divider 2:1
    
    // Simulated for now
    return 3.7f + (rand() % 200) / 1000.0f; // 3.7-3.9V
}

/**
 * Get battery percentage
 */
float power_manager_get_battery_percentage(float voltage)
{
    // Simple linear approximation for Li-ion
    // 3.0V = 0%, 4.2V = 100%
    if (voltage <= 3.0f) return 0.0f;
    if (voltage >= 4.2f) return 100.0f;
    
    return (voltage - 3.0f) / 1.2f * 100.0f;
}
