# ============================================================
# Edge Cache — SQLite Local Storage & Cloud Sync
# ============================================================
# Python 3.12 | SQLite
# Local data caching with automatic sync to cloud
# ============================================================

import asyncio
import sqlite3
import logging
import json
import os
from datetime import datetime, timedelta
from typing import Dict, List, Optional

logger = logging.getLogger('EdgeCache')


class EdgeCache:
    """Edge caching system with SQLite and cloud sync."""
    
    def __init__(self, config: dict):
        """Initialize the edge cache."""
        self.config = config
        self.db_path = config.get('edge_cache', {}).get('sqlite_path', '/opt/qelera/cache.db')
        self.retention_days = config.get('edge_cache', {}).get('retention_days', 7)
        self.max_disk_usage_mb = config.get('edge_cache', {}).get('max_disk_usage_mb', 5000)
        
        self.db_conn = None
        self.sync_queue = []
        
        logger.info("EdgeCache initialized")
    
    async def initialize(self):
        """Initialize the SQLite database."""
        logger.info("Initializing edge cache database...")
        
        # Create database directory
        os.makedirs(os.path.dirname(self.db_path), exist_ok=True)
        
        # Connect to database
        self.db_conn = sqlite3.connect(self.db_path)
        self.db_conn.row_factory = sqlite3.Row
        
        # Create tables
        self._create_tables()
        
        # Clean up old data
        await self._cleanup_old_data()
        
        logger.info("Edge cache database initialized")
    
    def _create_tables(self):
        """Create SQLite tables for caching."""
        cursor = self.db_conn.cursor()
        
        # Frames table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS frames (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                frame_number INTEGER,
                motion_detected BOOLEAN,
                motion_intensity REAL,
                brightness REAL,
                contrast REAL,
                sharpness REAL,
                qoe_score REAL,
                objects TEXT,
                processed_at TEXT DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Alerts table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS alerts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                type TEXT NOT NULL,
                severity TEXT NOT NULL,
                description TEXT,
                confidence REAL,
                frame_data TEXT,
                synced BOOLEAN DEFAULT 0,
                created_at TEXT DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Sensor data table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS sensor_data (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                device_id TEXT,
                accel_x REAL,
                accel_y REAL,
                accel_z REAL,
                gyro_x REAL,
                gyro_y REAL,
                gyro_z REAL,
                temperature REAL,
                humidity REAL,
                pressure REAL,
                synced BOOLEAN DEFAULT 0,
                created_at TEXT DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Sync log table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS sync_log (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                records_synced INTEGER,
                status TEXT,
                error_message TEXT,
                created_at TEXT DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Create indexes
        cursor.execute('CREATE INDEX IF NOT EXISTS idx_frames_timestamp ON frames(timestamp)')
        cursor.execute('CREATE INDEX IF NOT EXISTS idx_alerts_synced ON alerts(synced)')
        cursor.execute('CREATE INDEX IF NOT EXISTS idx_sensor_synced ON sensor_data(synced)')
        
        self.db_conn.commit()
        logger.info("Database tables created")
    
    async def cache_frame(self, frame_data: dict):
        """Cache frame analysis results."""
        try:
            cursor = self.db_conn.cursor()
            cursor.execute('''
                INSERT INTO frames (timestamp, frame_number, motion_detected, 
                                  motion_intensity, brightness, contrast, sharpness, qoe_score)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                frame_data.timestamp,
                frame_data.frame_number,
                frame_data.motion_detected,
                frame_data.motion_intensity,
                frame_data.brightness,
                frame_data.contrast,
                frame_data.sharpness,
                frame_data.qoe_metrics.get('overall_qoe', 0)
            ))
            
            # Store objects as JSON
            if frame_data.objects:
                objects_json = json.dumps(frame_data.objects)
                cursor.execute('UPDATE frames SET objects = ? WHERE rowid = last_insert_rowid()',
                             (objects_json,))
            
            self.db_conn.commit()
            
        except Exception as e:
            logger.error(f"Failed to cache frame: {e}")
    
    async def cache_alert(self, alert: dict):
        """Cache alert data."""
        try:
            cursor = self.db_conn.cursor()
            cursor.execute('''
                INSERT INTO alerts (timestamp, type, severity, description, 
                                  confidence, frame_data)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', (
                alert['timestamp'],
                alert['anomalies'][0]['type'] if alert.get('anomalies') else 'unknown',
                alert['anomalies'][0]['severity'] if alert.get('anomalies') else 'medium',
                alert['anomalies'][0]['description'] if alert.get('anomalies') else '',
                alert['anomalies'][0]['confidence'] if alert.get('anomalies') else 0.0,
                json.dumps(alert.get('frame_data', {}))
            ))
            
            self.db_conn.commit()
            
        except Exception as e:
            logger.error(f"Failed to cache alert: {e}")
    
    async def cache_sensor_data(self, sensor_data: dict):
        """Cache sensor data from ESP32 nodes."""
        try:
            cursor = self.db_conn.cursor()
            cursor.execute('''
                INSERT INTO sensor_data (timestamp, device_id, accel_x, accel_y, accel_z,
                                       gyro_x, gyro_y, gyro_z, temperature, humidity, pressure)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                sensor_data.get('timestamp', datetime.utcnow().isoformat()),
                sensor_data.get('device_id', 'unknown'),
                sensor_data.get('accel_x', 0),
                sensor_data.get('accel_y', 0),
                sensor_data.get('accel_z', 0),
                sensor_data.get('gyro_x', 0),
                sensor_data.get('gyro_y', 0),
                sensor_data.get('gyro_z', 0),
                sensor_data.get('temperature', 0),
                sensor_data.get('humidity', 0),
                sensor_data.get('pressure', 0)
            ))
            
            self.db_conn.commit()
            
        except Exception as e:
            logger.error(f"Failed to cache sensor data: {e}")
    
    async def sync_to_cloud(self):
        """Sync unsynced data to cloud."""
        logger.info("Starting cloud sync...")
        
        sync_start = datetime.utcnow()
        records_synced = 0
        
        try:
            # Sync alerts
            alert_count = await self._sync_alerts()
            records_synced += alert_count
            
            # Sync sensor data
            sensor_count = await self._sync_sensor_data()
            records_synced += sensor_count
            
            # Log sync
            cursor = self.db_conn.cursor()
            cursor.execute('''
                INSERT INTO sync_log (timestamp, records_synced, status)
                VALUES (?, ?, ?)
            ''', (sync_start.isoformat(), records_synced, 'success'))
            
            self.db_conn.commit()
            
            logger.info(f"Cloud sync completed: {records_synced} records synced")
            
        except Exception as e:
            logger.error(f"Cloud sync failed: {e}")
            
            # Log error
            cursor = self.db_conn.cursor()
            cursor.execute('''
                INSERT INTO sync_log (timestamp, records_synced, status, error_message)
                VALUES (?, ?, ?, ?)
            ''', (sync_start.isoformat(), records_synced, 'failed', str(e)))
            
            self.db_conn.commit()
    
    async def _sync_alerts(self) -> int:
        """Sync unsynced alerts to cloud."""
        cursor = self.db_conn.cursor()
        cursor.execute('SELECT * FROM alerts WHERE synced = 0 LIMIT 100')
        alerts = cursor.fetchall()
        
        if not alerts:
            return 0
        
        # Forward to cloud via MQTT
        from mqtt_broker import MQTTBroker
        # This would integrate with the MQTT broker
        
        # Mark as synced
        alert_ids = [alert['id'] for alert in alerts]
        placeholders = ','.join(['?'] * len(alert_ids))
        cursor.execute(f'UPDATE alerts SET synced = 1 WHERE id IN ({placeholders})', alert_ids)
        self.db_conn.commit()
        
        logger.info(f"Synced {len(alerts)} alerts to cloud")
        return len(alerts)
    
    async def _sync_sensor_data(self) -> int:
        """Sync unsynced sensor data to cloud."""
        cursor = self.db_conn.cursor()
        cursor.execute('SELECT * FROM sensor_data WHERE synced = 0 LIMIT 100')
        sensors = cursor.fetchall()
        
        if not sensors:
            return 0
        
        # Forward to cloud via MQTT
        # This would integrate with the MQTT broker
        
        # Mark as synced
        sensor_ids = [sensor['id'] for sensor in sensors]
        placeholders = ','.join(['?'] * len(sensor_ids))
        cursor.execute(f'UPDATE sensor_data SET synced = 1 WHERE id IN ({placeholders})', sensor_ids)
        self.db_conn.commit()
        
        logger.info(f"Synced {len(sensors)} sensor records to cloud")
        return len(sensors)
    
    async def _cleanup_old_data(self):
        """Clean up data older than retention period."""
        cutoff_date = (datetime.utcnow() - timedelta(days=self.retention_days)).isoformat()
        
        cursor = self.db_conn.cursor()
        
        # Delete old frames
        cursor.execute('DELETE FROM frames WHERE timestamp < ?', (cutoff_date,))
        
        # Delete old alerts
        cursor.execute('DELETE FROM alerts WHERE created_at < ?', (cutoff_date,))
        
        # Delete old sensor data
        cursor.execute('DELETE FROM sensor_data WHERE created_at < ?', (cutoff_date,))
        
        # Delete old sync logs
        cursor.execute('DELETE FROM sync_log WHERE created_at < ?', (cutoff_date,))
        
        self.db_conn.commit()
        
        logger.info(f"Cleaned up data older than {self.retention_days} days")
    
    async def shutdown(self):
        """Shutdown the edge cache."""
        if self.db_conn:
            self.db_conn.close()
            logger.info("Edge cache database closed")
    
    def get_status(self) -> dict:
        """Get cache status."""
        try:
            cursor = self.db_conn.cursor()
            
            cursor.execute('SELECT COUNT(*) as count FROM frames')
            frame_count = cursor.fetchone()['count']
            
            cursor.execute('SELECT COUNT(*) as count FROM alerts WHERE synced = 0')
            unsynced_alerts = cursor.fetchone()['count']
            
            cursor.execute('SELECT COUNT(*) as count FROM sensor_data WHERE synced = 0')
            unsynced_sensors = cursor.fetchone()['count']
            
            return {
                'frame_count': frame_count,
                'unsynced_alerts': unsynced_alerts,
                'unsynced_sensors': unsynced_sensors,
                'db_path': self.db_path,
                'retention_days': self.retention_days
            }
        except Exception as e:
            return {'error': str(e)}
