# ============================================================
# ESP32 Firmware Implementation — Qelera Edge Sensor Node
# ============================================================
# ESP-IDF 5.2+ | BLE 5.0 | MQTT | OTA | Low-Power
# Created: May 28, 2026
# ============================================================

## Overview

The ESP32 firmware implements a low-power edge sensor node for the Qelera system. It collects sensor data via BLE 5.0, detects motion using an IMU, and forwards data to the Qelera backend via MQTT (with HTTP fallback). The firmware supports OTA updates and deep sleep for extended battery life.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32-S3 Module                          │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────────┐   │
│  │   main.c    │  │ power_mgr.c  │  │  ota_update.c    │   │
│  │  (Orchestrator)│ │ (Sleep Modes)│  │  (Firmware Upd.) │   │
│  └──────┬──────┘  └──────┬───────┘  └────────┬─────────┘   │
│         │                │                   │             │
│  ┌──────┴──────┐  ┌─────┴──────┐  ┌─────────┴──────────┐  │
│  │sensor_coll.c│  │motion_det.c│  │  mqtt_client.c     │  │
│  │  (BLE 5.0)  │  │  (IMU)     │  │  (Data Forward)    │  │
│  └─────────────┘  └────────────┘  └─────────┬──────────┘  │
│                                              │             │
│  ┌───────────────────────────────────────────┼──────────┐  │
│  │                    http_client.c          │          │  │
│  │              (HTTP Fallback)              │          │  │
│  └───────────────────────────────────────────┴──────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## File Structure

```
software/firmware/esp32/
├── CMakeLists.txt              # Main project build file
├── sdkconfig                   # ESP-IDF configuration
├── main/
│   ├── CMakeLists.txt          # Component build config
│   ├── main.c                  # Main entry point
│   ├── sensor_collector.c      # BLE 5.0 sensor collection
│   ├── motion_detector.c       # IMU motion detection
│   ├── power_manager.c         # Low-power sleep modes
│   ├── mqtt_client.c           # MQTT data forwarding
│   ├── http_client.c           # HTTP fallback
│   └── ota_update.c            # OTA firmware updates
└── ESP32_FIRMWARE_IMPLEMENTATION.md
```

## Key Features

### 1. BLE 5.0 Sensor Collection
- Custom GATT service for sensor data
- Real-time BLE advertising and connections
- Sensor data: accelerometer, gyroscope, magnetometer, temperature, humidity, pressure

### 2. Motion Detection
- I2C IMU (BMP388 or similar)
- Configurable acceleration and gyro thresholds
- Debounce logic to prevent false triggers
- Motion intensity calculation

### 3. Low-Power Management
- Light sleep: 1-10ms wake intervals
- Deep sleep: timer-based wake (configurable)
- CPU frequency scaling (40-240MHz)
- Estimated battery life calculation

### 4. MQTT Data Forwarding
- QoS 1 message delivery
- Auto-reconnect with exponential backoff
- Message queuing for offline buffering (50 messages max)
- Retained messages for alerts

### 5. HTTP Fallback
- POST sensor data when MQTT unavailable
- Alert and heartbeat support
- Firmware download capability

### 6. OTA Updates
- HTTPS OTA with ESP-IDF native support
- Version checking and download
- Rollback support via dual partitions
- Progress monitoring

## Build and Flash

```bash
# Clone ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
source export.sh

# Build firmware
cd software/firmware/esp32
idf.py set-target esp32s3
idf.py build

# Flash to device
idf.py -p /dev/ttyUSB0 flash

# Monitor output
idf.py -p /dev/ttyUSB0 monitor
```

## Configuration

Edit `sdkconfig` to configure:
- WiFi SSID and password
- MQTT broker URL and credentials
- HTTP base URL and auth token
- OTA server URL
- Power management settings

## Power Consumption

| Mode | Current | Wake Interval | Est. Battery Life |
|------|---------|---------------|-------------------|
| Running | 80mA | - | - |
| Light Sleep | 3mA | 1s | ~6 months |
| Deep Sleep | 10µA | 10s | ~2 years |

## Testing

```bash
# Test BLE connection
bluetool connect Qelera-ESP32

# Test MQTT
mosquitto_sub -t "qelera/esp32/#"

# Test HTTP
curl -X POST http://localhost:8080/api/v1/sensors \
  -H "Content-Type: application/json" \
  -d '{"device_id":"test","timestamp":1234567890}'

# OTA update test
idf.py set-target esp32s3
idf.py set-flash-size 4MB
idf.py build
idf.py -p /dev/ttyUSB0 erase_flash
idf.py -p /dev/ttyUSB0 flash monitor
```

## Hardware Requirements

- ESP32-S3 module (or ESP32-WROOM)
- I2C IMU (BMP388, BNO055, or similar)
- External antenna (for BLE/WiFi)
- Li-ion battery (3.7V, 2000mAh+)
- USB-C for programming and power

## Troubleshooting

### WiFi Connection Issues
- Check SSID/password in sdkconfig
- Verify WiFi signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

### MQTT Connection Issues
- Verify broker URL and credentials
- Check firewall rules for port 8883 (SSL) or 1883 (plain)
- Ensure CA certificate is trusted

### Deep Sleep Not Working
- Check wake source configuration
- Ensure GPIO12 is not pulled high during sleep
- Verify RTC memory is not corrupted

### OTA Update Fails
- Check internet connectivity
- Verify firmware URL is accessible
- Ensure sufficient flash space (4MB+ recommended)
