# ============================================================
# MQTT Broker — Mosquitto Integration & Cloud Forwarding
# ============================================================
# Python 3.12 | paho-mqtt
# Local MQTT broker with cloud forwarding capabilities
# ============================================================

import asyncio
import json
import logging
import paho.mqtt.client as mqtt
from datetime import datetime
from typing import Dict, Optional

logger = logging.getLogger('MQTTBroker')


class MQTTBroker:
    """MQTT broker integration for Qelera."""
    
    def __init__(self, config: dict):
        """Initialize the MQTT broker."""
        self.config = config
        self.local_client = None
        self.cloud_client = None
        self.local_connected = False
        self.cloud_connected = False
        
        # MQTT configuration
        gateway_config = config.get('gateway', {})
        self.local_broker = gateway_config.get('mqtt_broker', 'localhost')
        self.local_port = gateway_config.get('mqtt_port', 1883)
        
        self.cloud_broker = gateway_config.get('cloud_mqtt', 'mqtt.qelera.io')
        self.cloud_port = gateway_config.get('cloud_mqtt_port', 8883)
        self.cloud_ssl = gateway_config.get('ssl_enabled', True)
        
        # Topic configuration
        self.topics = {
            'sensors': 'qelera/sensors/#',
            'alerts': 'qelera/alerts/#',
            'commands': 'qelera/commands/#',
            'status': 'qelera/status/#',
            'frame_data': 'qelera/frames/#'
        }
        
        logger.info("MQTTBroker initialized")
    
    async def start(self):
        """Start the MQTT broker and clients."""
        logger.info("Starting MQTT broker...")
        
        # Start local Mosquitto broker
        await self._start_local_broker()
        
        # Initialize local MQTT client
        self.local_client = mqtt.Client(client_id="rpi-gateway")
        self.local_client.on_connect = self._on_local_connect
        self.local_client.on_disconnect = self._on_local_disconnect
        self.local_client.on_message = self._on_message
        
        # Connect to local broker
        try:
            self.local_client.connect(self.local_broker, self.local_port, 60)
            self.local_client.loop_start()
            self.local_connected = True
            logger.info("Connected to local MQTT broker")
        except Exception as e:
            logger.error(f"Failed to connect to local broker: {e}")
        
        # Initialize cloud MQTT client
        self.cloud_client = mqtt.Client(client_id="rpi-gateway-cloud")
        self.cloud_client.on_connect = self._on_cloud_connect
        self.cloud_client.on_disconnect = self._on_cloud_disconnect
        
        # Configure SSL for cloud connection
        if self.cloud_ssl:
            self.cloud_client.tls_set()
        
        # Connect to cloud broker
        try:
            self.cloud_client.connect(self.cloud_broker, self.cloud_port, 60)
            self.cloud_client.loop_start()
            logger.info("Connected to cloud MQTT broker")
        except Exception as e:
            logger.error(f"Failed to connect to cloud broker: {e}")
        
        # Subscribe to topics
        await self._subscribe_to_topics()
    
    async def _start_local_broker(self):
        """Start local Mosquitto broker."""
        import subprocess
        
        try:
            # Check if Mosquitto is running
            result = subprocess.run(['systemctl', 'is-active', 'mosquitto'],
                                  capture_output=True, text=True)
            
            if result.stdout.strip() != 'active':
                # Start Mosquitto
                subprocess.run(['sudo', 'systemctl', 'start', 'mosquitto'],
                             check=True, capture_output=True)
                logger.info("Mosquitto broker started")
            else:
                logger.info("Mosquitto broker already running")
        except Exception as e:
            logger.error(f"Failed to start Mosquitto: {e}")
    
    def _on_local_connect(self, client, userdata, flags, rc):
        """Handle local MQTT connection."""
        if rc == 0:
            self.local_connected = True
            logger.info("Local MQTT connected")
        else:
            logger.error(f"Local MQTT connection failed: {rc}")
    
    def _on_local_disconnect(self, client, userdata, rc):
        """Handle local MQTT disconnection."""
        self.local_connected = False
        logger.warning("Local MQTT disconnected")
    
    def _on_cloud_connect(self, client, userdata, flags, rc):
        """Handle cloud MQTT connection."""
        if rc == 0:
            self.cloud_connected = True
            logger.info("Cloud MQTT connected")
        else:
            logger.error(f"Cloud MQTT connection failed: {rc}")
    
    def _on_cloud_disconnect(self, client, userdata, rc):
        """Handle cloud MQTT disconnection."""
        self.cloud_connected = False
        logger.warning("Cloud MQTT disconnected")
    
    def _on_message(self, client, userdata, msg):
        """Handle incoming MQTT messages."""
        try:
            payload = json.loads(msg.payload.decode())
            topic = msg.topic
            
            logger.info(f"Received message on {topic}: {payload}")
            
            # Route message based on topic
            if 'alerts' in topic:
                # Forward alerts to cloud
                asyncio.create_task(self._forward_to_cloud(msg.topic, payload))
            
            elif 'commands' in topic:
                # Process commands
                logger.info(f"Processing command: {payload}")
            
            elif 'sensors' in topic:
                # Cache sensor data
                asyncio.create_task(self._cache_sensor_data(payload))
        
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}")
    
    async def _subscribe_to_topics(self):
        """Subscribe to MQTT topics."""
        if not self.local_connected or not self.local_client:
            return
        
        for topic, qos in self.topics.items():
            try:
                self.local_client.subscribe(topic, qos=1)
                logger.info(f"Subscribed to {topic}")
            except Exception as e:
                logger.error(f"Failed to subscribe to {topic}: {e}")
    
    async def publish_alert(self, alert: dict):
        """Publish alert to MQTT."""
        if not self.local_connected or not self.local_client:
            logger.warning("Cannot publish alert: not connected to MQTT")
            return
        
        try:
            payload = json.dumps(alert)
            topic = "qelera/alerts/rpi-gateway"
            
            self.local_client.publish(topic, payload, qos=1)
            logger.info(f"Alert published to {topic}")
            
            # Forward to cloud if connected
            if self.cloud_connected and self.cloud_client:
                await self._forward_to_cloud(topic, alert)
        
        except Exception as e:
            logger.error(f"Failed to publish alert: {e}")
    
    async def _forward_to_cloud(self, topic: str, payload: dict):
        """Forward message to cloud MQTT broker."""
        if not self.cloud_connected or not self.cloud_client:
            logger.warning("Cannot forward to cloud: not connected")
            return
        
        try:
            cloud_topic = f"cloud/{topic}"
            self.cloud_client.publish(cloud_topic, json.dumps(payload), qos=1)
            logger.info(f"Forwarded to cloud: {cloud_topic}")
        except Exception as e:
            logger.error(f"Failed to forward to cloud: {e}")
    
    async def _cache_sensor_data(self, data: dict):
        """Cache sensor data for later sync."""
        # This would integrate with edge_cache.py
        logger.debug(f"Sensor data cached: {data}")
    
    async def is_cloud_connected(self) -> bool:
        """Check if connected to cloud MQTT broker."""
        return self.cloud_connected
    
    async def stop(self):
        """Stop the MQTT broker and clients."""
        logger.info("Stopping MQTT broker...")
        
        if self.local_client:
            self.local_client.loop_stop()
            self.local_client.disconnect()
        
        if self.cloud_client:
            self.cloud_client.loop_stop()
            self.cloud_client.disconnect()
        
        self.local_connected = False
        self.cloud_connected = False
        
        logger.info("MQTT broker stopped")
    
    def get_status(self) -> dict:
        """Get broker status."""
        return {
            'local_connected': self.local_connected,
            'cloud_connected': self.cloud_connected,
            'topics': list(self.topics.keys())
        }
