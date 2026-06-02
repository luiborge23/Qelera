# ============================================================
# Raspberry Pi Gateway — Main Gateway Orchestrator
# ============================================================
# Python 3.12 | GStreamer | OpenCV | MQTT | Edge Caching
# Entry point for all gateway components
# ============================================================

import asyncio
import logging
import signal
import sys
import os
import yaml
from pathlib import Path
from datetime import datetime

# Import gateway components
from gstreamer_pipeline import GStreamerPipeline
from frame_analyzer import FrameAnalyzer
from anomaly_detector import AnomalyDetector
from mqtt_broker import MQTTBroker
from edge_cache import EdgeCache
from dashboard_relay import DashboardRelay

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler('/var/log/qelera/gateway.log')
    ]
)
logger = logging.getLogger('Gateway')


class VisionTrackGateway:
    """Main Qelera Gateway orchestrator."""
    
    def __init__(self, config_path: str = "config.yaml"):
        """Initialize the gateway with configuration."""
        self.config = self._load_config(config_path)
        self.running = False
        
        # Initialize components
        self.pipeline = GStreamerPipeline(self.config)
        self.frame_analyzer = FrameAnalyzer(self.config)
        self.anomaly_detector = AnomalyDetector(self.config)
        self.mqtt_broker = MQTTBroker(self.config)
        self.edge_cache = EdgeCache(self.config)
        self.dashboard_relay = DashboardRelay(self.config)
        
        # Event handlers
        self._setup_signal_handlers()
        
        logger.info("Qelera Gateway initialized")
    
    def _load_config(self, config_path: str) -> dict:
        """Load configuration from YAML file."""
        config_file = Path(config_path)
        
        if not config_file.exists():
            logger.warning(f"Config file {config_path} not found, using defaults")
            return self._default_config()
        
        with open(config_file, 'r') as f:
            config = yaml.safe_load(f)
        
        logger.info(f"Configuration loaded from {config_path}")
        return config
    
    def _default_config(self) -> dict:
        """Return default configuration."""
        return {
            'gateway': {
                'name': 'rpi-gateway-01',
                'mqtt_broker': 'localhost',
                'mqtt_port': 1883,
                'cloud_mqtt': 'mqtt.qelera.io',
                'cloud_mqtt_port': 8883,
                'ssl_enabled': True
            },
            'video': {
                'source_type': 'usb',
                'rtsp_url': None,
                'usb_device': '/dev/video0',
                'resolution': '1920x1080',
                'fps': 30
            },
            'anomaly_detection': {
                'yolo_model': 'yolov8n.pt',
                'confidence_threshold': 0.5,
                'motion_threshold': 0.02,
                'alert_interval': 60
            },
            'edge_cache': {
                'sqlite_path': '/opt/qelera/cache.db',
                'retention_days': 7,
                'max_disk_usage_mb': 5000
            },
            'dashboard': {
                'host': '0.0.0.0',
                'port': 8080,
                'ws_port': 8081
            }
        }
    
    def _setup_signal_handlers(self):
        """Setup signal handlers for graceful shutdown."""
        loop = asyncio.new_event_loop()
        
        for sig in (signal.SIGINT, signal.SIGTERM):
            loop.add_signal_handler(sig, lambda s=sig: asyncio.create_task(self.shutdown(s)))
        
        self._shutdown_loop = loop
    
    async def start(self):
        """Start all gateway components."""
        logger.info("Starting Qelera Gateway...")
        self.running = True
        
        try:
            # Initialize MQTT broker
            await self.mqtt_broker.start()
            logger.info("MQTT broker started")
            
            # Initialize edge cache
            await self.edge_cache.initialize()
            logger.info("Edge cache initialized")
            
            # Start dashboard relay
            await self.dashboard_relay.start()
            logger.info("Dashboard relay started")
            
            # Start video pipeline
            await self.pipeline.start()
            logger.info("Video pipeline started")
            
            # Start processing loop
            asyncio.create_task(self._processing_loop())
            
            logger.info("Qelera Gateway started successfully")
            
            # Keep running
            while self.running:
                await asyncio.sleep(1)
                
        except Exception as e:
            logger.error(f"Gateway failed to start: {e}")
            await self.shutdown(signal.SIGTERM)
    
    async def _processing_loop(self):
        """Main processing loop for frame analysis and anomaly detection."""
        logger.info("Processing loop started")
        
        while self.running:
            try:
                # Get frame from pipeline
                frame = await self.pipeline.get_frame()
                
                if frame is None:
                    await asyncio.sleep(0.01)
                    continue
                
                # Analyze frame
                analysis_result = await self.frame_analyzer.analyze(frame)
                
                # Check for anomalies
                anomalies = await self.anomaly_detector.detect_anomalies(analysis_result)
                
                if anomalies:
                    # Create alert
                    alert = {
                        'timestamp': datetime.utcnow().isoformat(),
                        'gateway_id': self.config['gateway']['name'],
                        'anomalies': anomalies,
                        'frame_data': analysis_result
                    }
                    
                    # Publish alert via MQTT
                    await self.mqtt_broker.publish_alert(alert)
                    
                    # Cache alert
                    await self.edge_cache.cache_alert(alert)
                    
                    # Relay to dashboard
                    await self.dashboard_relay.broadcast_alert(alert)
                    
                    logger.info(f"Anomaly detected: {len(anomalies)} events")
                
                # Cache frame data
                await self.edge_cache.cache_frame(analysis_result)
                
                # Sync to cloud if connected
                if await self.mqtt_broker.is_cloud_connected():
                    await self.edge_cache.sync_to_cloud()
                
                await asyncio.sleep(0.01)  # 10ms processing interval
                
            except Exception as e:
                logger.error(f"Processing loop error: {e}")
                await asyncio.sleep(1)
    
    async def shutdown(self, signum=None):
        """Gracefully shutdown all gateway components."""
        logger.info(f"Shutting down gateway (signal={signum})...")
        self.running = False
        
        # Stop components in reverse order
        await self.pipeline.stop()
        await self.dashboard_relay.stop()
        await self.edge_cache.shutdown()
        await self.mqtt_broker.stop()
        
        logger.info("Qelera Gateway stopped")
        sys.exit(0)
    
    def get_status(self) -> dict:
        """Get current gateway status."""
        return {
            'running': self.running,
            'components': {
                'pipeline': self.pipeline.get_status(),
                'frame_analyzer': self.frame_analyzer.get_status(),
                'anomaly_detector': self.anomaly_detector.get_status(),
                'mqtt_broker': self.mqtt_broker.get_status(),
                'edge_cache': self.edge_cache.get_status(),
                'dashboard_relay': self.dashboard_relay.get_status()
            },
            'timestamp': datetime.utcnow().isoformat()
        }


async def main():
    """Main entry point."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Qelera Gateway')
    parser.add_argument('--config', type=str, default='config.yaml',
                       help='Path to configuration file')
    
    args = parser.parse_args()
    
    # Create gateway
    gateway = VisionTrackGateway(args.config)
    
    # Start gateway
    await gateway.start()


if __name__ == '__main__':
    asyncio.run(main())
