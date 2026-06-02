# ============================================================
# Raspberry Pi Gateway — Qelera Edge Node
# ============================================================
# Python 3.12 | GStreamer | OpenCV | MQTT | Edge Caching
# Created: May 28, 2026
# ============================================================

## Overview

The Raspberry Pi gateway serves as the edge computing hub for the Qelera system. It ingests video streams via GStreamer, performs local frame analysis with OpenCV, runs lightweight anomaly detection models, and bridges sensor data to the cloud via MQTT. It also provides local dashboard relay for real-time monitoring.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│              Raspberry Pi 4/5 (Gateway)                     │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────────────┐   │
│  │              gstreamer_pipeline.py                   │   │
│  │         (Video Ingestion & RTSP/USB)                │   │
│  └──────────────────────┬───────────────────────────────┘   │
│                         │                                   │
│  ┌──────────────────────┴───────────────────────────────┐   │
│  │              frame_analyzer.py                       │   │
│  │         (OpenCV Frame Processing)                    │   │
│  └──────────────────────┬───────────────────────────────┘   │
│                         │                                   │
│  ┌──────────────────────┴───────────────────────────────┐   │
│  │           anomaly_detector.py                        │   │
│  │         (YOLOv8 + Statistical Anomalies)             │   │
│  └──────────────────────┬───────────────────────────────┘   │
│                         │                                   │
│  ┌──────────────────────┴───────────────────────────────┐   │
│  │              mqtt_broker.py                          │   │
│  │         (Mosquitto + MQTT Forwarding)                │   │
│  └──────────────────────┬───────────────────────────────┘   │
│                         │                                   │
│  ┌──────────────────────┴───────────────────────────────┐   │
│  │           edge_cache.py                              │   │
│  │         (SQLite + Local Storage)                     │   │
│  └──────────────────────┬───────────────────────────────┘   │
│                         │                                   │
│  ┌──────────────────────┴───────────────────────────────┐   │
│  │          dashboard_relay.py                          │   │
│  │         (WebSocket + Local HTTP API)                 │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## File Structure

```
software/firmware/rpi/
├── requirements.txt              # Python dependencies
├── Dockerfile                    # Containerized deployment
├── gateway.py                    # Main gateway orchestrator
├── gstreamer_pipeline.py         # Video ingestion pipeline
├── frame_analyzer.py             # OpenCV frame processing
├── anomaly_detector.py           # YOLOv8 + statistical detection
├── mqtt_broker.py                # MQTT broker integration
├── edge_cache.py                 # Edge caching with SQLite
├── dashboard_relay.py            # Local dashboard WebSocket relay
├── config.yaml                   # Configuration file
└── RPI_GATEWAY_IMPLEMENTATION.md # Full documentation
```

## Key Features

### 1. GStreamer Video Ingestion
- RTSP stream support (IP cameras)
- USB camera support (UVC compliant)
- Hardware-accelerated decoding (VPU)
- Frame extraction at configurable FPS

### 2. OpenCV Frame Analysis
- Real-time frame processing pipeline
- Motion detection via frame differencing
- Object detection with YOLOv8
- QoE metrics extraction

### 3. Local Anomaly Detection
- YOLOv8 model for object detection
- Statistical anomaly detection (z-score, IQR)
- Motion pattern analysis
- Real-time alerting

### 4. MQTT Broker Integration
- Mosquitto broker (local)
- MQTT forwarding to cloud
- Topic-based message routing
- QoS 1 message delivery

### 5. Edge Caching
- SQLite database for local storage
- Automatic sync to cloud
- Configurable retention policies
- Disk space management

### 6. Dashboard Relay
- WebSocket server for real-time updates
- Local HTTP API for dashboard
- Frame preview streaming
- System status monitoring

## Build and Deploy

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install dependencies
sudo apt install -y python3 python3-pip libgl1 libglib2.0-0
pip3 install -r requirements.txt

# Clone repository
git clone https://github.com/qelera/qelera-gateway.git
cd qelera-gateway

# Run gateway
python3 gateway.py

# Or run via Docker
docker build -t qelera-gateway .
docker run -d --name gateway \
  --device=/dev/video0 \
  -p 8080:8080 \
  -p 1883:1883 \
  qelera-gateway
```

## Configuration

Edit `config.yaml`:

```yaml
gateway:
  name: "rpi-gateway-01"
  mqtt_broker: "localhost"
  mqtt_port: 1883
  cloud_mqtt: "mqtt.qelera.io"
  cloud_mqtt_port: 8883
  ssl_enabled: true

video:
  source_type: "rtsp"  # rtsp, usb, file
  rtsp_url: "rtsp://camera/stream"
  usb_device: "/dev/video0"
  resolution: "1920x1080"
  fps: 30

anomaly_detection:
  yolo_model: "yolov8n.pt"
  confidence_threshold: 0.5
  motion_threshold: 0.02
  alert_interval: 60  # seconds

edge_cache:
  sqlite_path: "/opt/qelera/cache.db"
  retention_days: 7
  max_disk_usage_mb: 5000

dashboard:
  host: "0.0.0.0"
  port: 8080
  ws_port: 8081
```

## System Requirements

- Raspberry Pi 4 (4GB+) or Raspberry Pi 5
- 16GB+ microSD card (or SSD)
- USB camera or RTSP-compatible IP camera
- Ethernet or WiFi connection
- Python 3.12+

## Performance Benchmarks

| Model | FPS | GPU | RAM |
|-------|-----|-----|-----|
| YOLOv8n | 15-20 | VPU | ~500MB |
| YOLOv8s | 8-12 | VPU | ~800MB |
| Motion Only | 30-60 | CPU | ~200MB |

## Troubleshooting

### Video Stream Issues
- Check camera connection and permissions
- Verify RTSP URL and credentials
- Ensure GStreamer plugins installed: `sudo apt install gstreamer1.0-*`

### MQTT Connection Issues
- Verify Mosquitto running: `systemctl status mosquitto`
- Check firewall rules for ports 1883, 8883
- Verify broker URL in config.yaml

### High CPU Usage
- Switch to YOLOv8n (nano) model
- Enable hardware acceleration
- Reduce FPS to 15-20

### Disk Space Issues
- Check cache size: `sqlite3 cache.db "SELECT SUM(pagesize*pgsize) FROM dbstat;"`
- Adjust retention_days in config
- Enable auto-cleanup (default enabled)
