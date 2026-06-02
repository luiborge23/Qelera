# Qelera Raspberry Pi Gateway — Edge Processing Node v1.0
# Raspberry Pi 4/5, Python 3.12, GStreamer, OpenCV, MQTT
# Created: May 28, 2026

# ============================================================
# PROJECT STRUCTURE
# ============================================================
# rpi-gateway/
# ├── main.py                     # Entry point
# ├── requirements.txt
# ├── Dockerfile
# ├── config/
# │   ├── gateway_config.yaml     # Gateway configuration
# │   └── camera_config.yaml      # Camera settings
# ├── src/
# │   ├── __init__.py
# │   ├── gateway.py              # Main gateway class
# │   ├── camera/
# │   │   ├── __init__.py
# │   │   ├── camera_manager.py   # Camera initialization
# │   │   ├── gstreamer_pipeline.py  # GStreamer pipeline
# │   │   └── capture_modes.py    # HDMI/CSI capture modes
# │   ├── processing/
# │   │   ├── __init__.py
# │   │   ├── frame_processor.py  # Frame processing pipeline
# │   │   ├── quality_estimator.py  # QoE estimation
# │   │   └── anomaly_detector.py # Anomaly detection
# │   ├── network/
# │   │   ├── __init__.py
# │   │   ├── mqtt_client.py      # MQTT communication
# │   │   ├── websocket_server.py # WebSocket for local clients
# │   │   └── api_server.py       # REST API for local control
# │   ├── storage/
# │   │   ├── __init__.py
# │   │   ├── local_store.py      # Local SSD storage
# │   │   └── buffer_manager.py   # Ring buffer for frames
# │   ├── monitoring/
# │   │   ├── __init__.py
# │   │   ├── system_health.py    # System monitoring
# │   │   └── metrics_collector.py  # Performance metrics
# │   └── utils/
# │       ├── __init__.py
# │       ├── logger.py           # Structured logging
# │       ├── config_loader.py    # YAML config parsing
# │       └── helpers.py          # Utility functions
# ├── tests/
# │   ├── test_gateway.py
# │   ├── test_camera.py
# │   ├── test_processing.py
# │   └── test_network.py
# ├── scripts/
# │   ├── install.sh              # Installation script
# │   ├── start.sh                # Startup script
# │   └── stop.sh                 # Stop script
# └── README.md

# ============================================================
# REQUIREMENTS (requirements.txt)
# ============================================================
# paho-mqtt==2.1.0
# opencv-python-headless==4.9.0.80
# numpy==1.26.4
# Pillow==10.3.0
# gstreamer==1.0.0
# flask==3.0.2
# flask-cors==4.0.0
# websockets==13.0
# pyyaml==6.0.1
# psutil==5.9.8
# Pillow==10.3.0
# picamera2==0.3.22
# v4l2capture==5.3.0
# cryptography==42.0.5
# requests==2.31.0
# aiohttp==3.9.3
# prometheus-client==0.20.0
# structlog==24.1.0
# watchdog==3.0.0
# gpiozero==2.0.1

# ============================================================
# GATEWAY CONFIG (config/gateway_config.yaml)
# ============================================================
# gateway:
#   name: rpi-gateway-001
#   device_id: rpi-gw-001
#   version: "1.0.0"
#   log_level: INFO
#   log_format: json
#
# camera:
#   input_source: hdmi  # hdmi, csi, usb
#   resolution: "1920x1080"
#   framerate: 60
#   format: yuv420
#   exposure_mode: auto
#   awb_mode: auto
#   brightness: 0
#   contrast: 0
#   saturation: 0
#   sharpness: 0
#
# processing:
#   frame_interval_ms: 100  # Process every 100ms
#   model_path: models/anomaly_detector.tflite
#   max_queue_size: 100
#   use_gpu: true
#   use_nnp: false
#
# mqtt:
#   broker: mqtt://broker:1883
#   client_id: rpi-gateway-001
#   username: qelera
#   password: "***"
#   topic_prefix: qelera/gateway/001
#   qos: 1
#   retain: false
#   clean_session: true
#   keep_alive: 60
#
# storage:
#   local_path: /mnt/ssd/qelera
#   max_size_gb: 50
#   retention_days: 7
#   compression: true
#
# network:
#   websocket_port: 8765
#   api_port: 5000
#   cors_origins: "*"
#   api_key: "***"
#
# monitoring:
#   prometheus_enabled: true
#   prometheus_port: 9090
#   health_check_interval: 30
#   system_metrics: true
#
# ota:
#   enabled: true
#   update_url: https://updates.qelera.io/firmware/rpi
#   check_interval_hours: 24
#   auto_update: false
#   rollback_on_failure: true

# ============================================================
# MAIN GATEWAY (src/gateway.py)
# ============================================================
# #!/usr/bin/env python3
# """
# Qelera Raspberry Pi Gateway
# Main orchestrator for edge video processing and QoE monitoring
# """
#
# import os
# import sys
# import signal
# import threading
# import time
# import queue
# import structlog
# from pathlib import Path
# from typing import Optional, Dict, Any
# from dataclasses import dataclass, field
#
# import paho.mqtt.client as mqtt
# import cv2
# import numpy as np
# import psutil
# from PIL import Image
#
# from src.config.loader import load_config
# from src.camera.camera_manager import CameraManager
# from src.camera.gstreamer_pipeline import GStreamerPipeline
# from src.processing.frame_processor import FrameProcessor
# from src.processing.quality_estimator import QualityEstimator
# from src.processing.anomaly_detector import AnomalyDetector
# from src.network.mqtt_client import MQTTClient
# from src.network.websocket_server import WebSocketServer
# from src.network.api_server import APIServer
# from src.storage.local_store import LocalStore
# from src.storage.buffer_manager import RingBuffer
# from src.monitoring.system_health import SystemHealth
# from src.monitoring.metrics_collector import MetricsCollector
# from src.utils.logger import setup_logging
#
# logger = structlog.get_logger()
#
#
# @dataclass
# class GatewayState:
#     """Gateway runtime state"""
#     is_running: bool = False
#     is_camera_active: bool = False
#     is_processing: bool = False
#     frames_processed: int = 0
#     errors: int = 0
#     uptime_seconds: float = 0
#     start_time: Optional[float] = None
#     current_frame: Optional[np.ndarray] = None
#     qoe_score: float = 0.0
#     anomaly_detected: bool = False
#     anomaly_type: str = ""
#     battery_level: Optional[float] = None
#     temperature: float = 0.0
#     memory_usage: float = 0.0
#     cpu_usage: float = 0.0
#     disk_usage: float = 0.0
#
#
# class VisionTrackGateway:
#     """Main Qelera Gateway class"""
#
#     def __init__(self, config_path: str = "config/gateway_config.yaml"):
#         self.config = load_config(config_path)
#         self.state = GatewayState()
#         self.frame_queue: queue.Queue = queue.Queue(maxsize=100)
#         self.result_queue: queue.Queue = queue.Queue(maxsize=50)
#         self._stop_event = threading.Event()
#
#         # Initialize components
#         self._init_components()
#
#     def _init_components(self):
#         """Initialize all gateway components"""
#         # Camera
#         if self.config["camera"]["input_source"] == "hdmi":
#             self.camera = GStreamerPipeline(self.config["camera"])
#         else:
#             self.camera = CameraManager(self.config["camera"])
#
#         # Processing
#         self.frame_processor = FrameProcessor(self.config["processing"])
#         self.quality_estimator = QualityEstimator()
#         self.anomaly_detector = AnomalyDetector(
#             model_path=self.config["processing"]["model_path"]
#         )
#
#         # Network
#         self.mqtt_client = MQTTClient(self.config["mqtt"])
#         self.ws_server = WebSocketServer(
#             port=self.config["network"]["websocket_port"]
#         )
#         self.api_server = APIServer(
#             port=self.config["network"]["api_port"],
#             api_key=self.config["network"]["api_key"]
#         )
#
#         # Storage
#         self.local_store = LocalStore(self.config["storage"])
#         self.ring_buffer = RingBuffer(maxsize=100)
#
#         # Monitoring
#         self.system_health = SystemHealth()
#         self.metrics_collector = MetricsCollector()
#
#     def start(self):
#         """Start the gateway"""
#         logger.info("Starting Qelera Gateway")
#
#         # Set up signal handlers
#         signal.signal(signal.SIGINT, self._signal_handler)
#         signal.signal(signal.SIGTERM, self._signal_handler)
#
#         # Initialize components
#         self.mqtt_client.connect()
#         self.local_store.initialize()
#         self.system_health.start()
#         self.metrics_collector.start()
#
#         # Start camera
#         if self.camera.initialize():
#             self.state.is_camera_active = True
#             logger.info("Camera initialized successfully")
#         else:
#             logger.error("Failed to initialize camera")
#             return False
#
#         # Start processing threads
#         self._stop_event.clear()
#         self.state.is_running = True
#         self.state.start_time = time.time()
#
#         threading.Thread(target=self._camera_thread, daemon=True).start()
#         threading.Thread(target=self._processing_thread, daemon=True).start()
#         threading.Thread(target=self._result_thread, daemon=True).start()
#         threading.Thread(target=self._monitoring_thread, daemon=True).start()
#
#         # Start servers
#         self.ws_server.start()
#         self.api_server.start()
#
#         logger.info("Qelera Gateway started successfully")
#         return True
#
#     def stop(self):
#         """Stop the gateway"""
#         logger.info("Stopping Qelera Gateway")
#         self._stop_event.set()
#         self.state.is_running = False
#
#         # Stop components
#         self.camera.stop()
#         self.mqtt_client.disconnect()
#         self.ws_server.stop()
#         self.api_server.stop()
#         self.system_health.stop()
#         self.metrics_collector.stop()
#
#         logger.info("Qelera Gateway stopped")
#
#     def _camera_thread(self):
#         """Camera capture thread"""
#         while not self._stop_event.is_set():
#             try:
#                 frame = self.camera.capture()
#                 if frame is not None:
#                     self.frame_queue.put(frame)
#                     self.state.current_frame = frame.copy()
#                     self.state.frames_processed += 1
#             except Exception as e:
#                 logger.error("Camera capture error", error=str(e))
#                 self.state.errors += 1
#                 time.sleep(0.1)
#
#     def _processing_thread(self):
#         """Frame processing thread"""
#         while not self._stop_event.is_set():
#             try:
#                 if self.frame_queue.empty():
#                     time.sleep(0.01)
#                     continue
#
#                 frame = self.frame_queue.get(timeout=1)
#
#                 # Process frame
#                 processed = self.frame_processor.process(frame)
#
#                 # Estimate QoE
#                 qoe = self.quality_estimator.estimate(processed)
#                 self.state.qoe_score = qoe["overall_score"]
#
#                 # Detect anomalies
#                 anomaly = self.anomaly_detector.detect(processed)
#                 if anomaly["detected"]:
#                     self.state.anomaly_detected = True
#                     self.state.anomaly_type = anomaly["type"]
#                     self._send_alert(anomaly)
#                 else:
#                     self.state.anomaly_detected = False
#                     self.state.anomaly_type = ""
#
#                 # Store frame locally
#                 self.local_store.save_frame(processed)
#                 self.ring_buffer.push(frame)
#
#             except queue.Empty:
#                 continue
#             except Exception as e:
#                 logger.error("Processing error", error=str(e))
#                 self.state.errors += 1
#
#     def _result_thread(self):
#         """MQTT result publishing thread"""
#         while not self._stop_event.is_set():
#             try:
#                 # Publish QoE metrics
#                 metrics = {
#                     "timestamp": time.time(),
#                     "qoe_score": self.state.qoe_score,
#                     "anomaly_detected": self.state.anomaly_detected,
#                     "anomaly_type": self.state.anomaly_type,
#                     "frames_processed": self.state.frames_processed,
#                     "errors": self.state.errors
#                 }
#                 self.mqtt_client.publish("qoe/metrics", metrics)
#
#                 # Publish system health
#                 health = self.system_health.get_status()
#                 self.mqtt_client.publish("system/health", health)
#
#                 time.sleep(1)  # Every second
#
#             except Exception as e:
#                 logger.error("Result publishing error", error=str(e))
#
#     def _monitoring_thread(self):
#         """System monitoring thread"""
#         while not self._stop_event.is_set():
#             try:
#                 # Update system metrics
#                 self.state.cpu_usage = psutil.cpu_percent(interval=1)
#                 self.state.memory_usage = psutil.virtual_memory().percent
#                 self.state.disk_usage = psutil.disk_usage('/').percent
#                 self.state.temperature = self._get_cpu_temperature()
#
#                 # Collect Prometheus metrics
#                 self.metrics_collector.update(
#                     cpu=self.state.cpu_usage,
#                     memory=self.state.memory_usage,
#                     disk=self.state.disk_usage,
#                     temperature=self.state.temperature,
#                     uptime=self.state.uptime_seconds,
#                     frames=self.state.frames_processed
#                 )
#
#                 time.sleep(10)  # Every 10 seconds
#
#             except Exception as e:
#                 logger.error("Monitoring error", error=str(e))
#
#     def _send_alert(self, anomaly: Dict[str, Any]):
#         """Send alert for detected anomaly"""
#         alert = {
#             "timestamp": time.time(),
#             "gateway_id": self.config["gateway"]["device_id"],
#             "anomaly_type": anomaly["type"],
#             "confidence": anomaly["confidence"],
#             "severity": anomaly["severity"]
#         }
#         self.mqtt_client.publish("alerts/anomaly", alert)
#
#     def _get_cpu_temperature(self) -> float:
#         """Get CPU temperature"""
#         try:
#             with open("/sys/class/thermal/thermal_zone0/temp", "r") as f:
#                 return float(f.read().strip()) / 1000.0
#         except (FileNotFoundError, ValueError):
#             return 0.0
#
#     def _signal_handler(self, signum, frame):
#         """Handle shutdown signals"""
#         logger.info("Received signal", signum=signum)
#         self.stop()
#         sys.exit(0)
#
#
# if __name__ == "__main__":
#     gateway = VisionTrackGateway()
#     gateway.start()
#
#     # Keep main thread alive
#     try:
#         while gateway.state.is_running:
#             time.sleep(1)
#     except KeyboardInterrupt:
#         gateway.stop()
