# ============================================================
# Anomaly Detector — YOLOv8 + Statistical Anomaly Detection
# ============================================================
# Python 3.12 | YOLOv8 | Statistical Analysis
# Real-time anomaly detection with multiple detection methods
# ============================================================

import asyncio
import logging
import numpy as np
from datetime import datetime, timedelta
from dataclasses import dataclass, field
from typing import List, Dict, Optional
from collections import deque

logger = logging.getLogger('AnomalyDetector')


@dataclass
class Anomaly:
    """Represents a detected anomaly."""
    timestamp: str = field(default_factory=lambda: datetime.utcnow().isoformat())
    type: str = ""
    severity: str = ""
    description: str = ""
    confidence: float = 0.0
    frame_data: Dict = field(default_factory=dict)


class AnomalyDetector:
    """Multi-method anomaly detector."""
    
    def __init__(self, config: dict):
        """Initialize the anomaly detector."""
        self.config = config
        self.alert_interval = config.get('anomaly_detection', {}).get('alert_interval', 60)
        self.last_alert_time = datetime.utcnow()
        
        # Historical data for statistical analysis
        self.qoe_history = deque(maxlen=300)  # 5 minutes at 1 FPS
        self.motion_history = deque(maxlen=300)
        self.brightness_history = deque(maxlen=300)
        
        # Statistical thresholds
        self.z_score_threshold = 2.5
        self.iqr_multiplier = 1.5
        
        logger.info("AnomalyDetector initialized")
    
    async def detect_anomalies(self, analysis_result) -> List[Anomaly]:
        """Detect anomalies in frame analysis results."""
        anomalies = []
        
        # Check for motion anomalies
        motion_anomaly = self._check_motion_anomaly(analysis_result)
        if motion_anomaly:
            anomalies.append(motion_anomaly)
        
        # Check for QoE anomalies
        qoe_anomaly = self._check_qoe_anomaly(analysis_result)
        if qoe_anomaly:
            anomalies.append(qoe_anomaly)
        
        # Check for brightness anomalies
        brightness_anomaly = self._check_brightness_anomaly(analysis_result)
        if brightness_anomaly:
            anomalies.append(brightness_anomaly)
        
        # Check for object anomalies
        object_anomaly = self._check_object_anomaly(analysis_result)
        if object_anomaly:
            anomalies.append(object_anomaly)
        
        # Update historical data
        self._update_history(analysis_result)
        
        return anomalies
    
    def _check_motion_anomaly(self, analysis_result) -> Optional[Anomaly]:
        """Check for motion-related anomalies."""
        if not analysis_result.motion_detected:
            return None
        
        # Check if motion intensity is abnormal
        if len(self.motion_history) < 10:
            return None
        
        motion_values = list(self.motion_history)
        mean_motion = np.mean(motion_values)
        std_motion = np.std(motion_values)
        
        if std_motion == 0:
            return None
        
        # Calculate z-score
        z_score = abs(analysis_result.motion_intensity - mean_motion) / std_motion
        
        if z_score > self.z_score_threshold:
            severity = "high" if z_score > 3.5 else "medium"
            return Anomaly(
                type="motion_anomaly",
                severity=severity,
                description=f"Abnormal motion detected (z-score: {z_score:.2f})",
                confidence=min(1.0, z_score / 5.0),
                frame_data={'motion_intensity': analysis_result.motion_intensity}
            )
        
        return None
    
    def _check_qoe_anomaly(self, analysis_result) -> Optional[Anomaly]:
        """Check for Quality of Experience anomalies."""
        if 'overall_qoe' not in analysis_result.qoe_metrics:
            return None
        
        qoe_score = analysis_result.qoe_metrics['overall_qoe']
        
        if len(self.qoe_history) < 10:
            return None
        
        qoe_values = list(self.qoe_history)
        mean_qoe = np.mean(qoe_values)
        std_qoe = np.std(qoe_values)
        
        if std_qoe == 0:
            return None
        
        # Check for significant QoE degradation
        if qoe_score < mean_qoe - 2 * std_qoe:
            severity = "critical" if qoe_score < 30 else "high"
            return Anomaly(
                type="qoe_degradation",
                severity=severity,
                description=f"QoE score dropped to {qoe_score:.1f} (mean: {mean_qoe:.1f})",
                confidence=min(1.0, (mean_qoe - qoe_score) / (2 * std_qoe)),
                frame_data={'qoe_score': qoe_score}
            )
        
        return None
    
    def _check_brightness_anomaly(self, analysis_result) -> Optional[Anomaly]:
        """Check for brightness-related anomalies."""
        brightness = analysis_result.brightness
        
        if len(self.brightness_history) < 10:
            return None
        
        brightness_values = list(self.brightness_history)
        mean_brightness = np.mean(brightness_values)
        std_brightness = np.std(brightness_values)
        
        if std_brightness == 0:
            return None
        
        # Check for significant brightness changes
        brightness_change = abs(brightness - mean_brightness) / 128.0
        
        if brightness_change > 0.3:  # 30% change from mean
            severity = "high" if brightness_change > 0.5 else "medium"
            direction = "overexposed" if brightness > mean_brightness else "underexposed"
            return Anomaly(
                type="brightness_anomaly",
                severity=severity,
                description=f"Camera {direction} (brightness: {brightness:.0f})",
                confidence=min(1.0, brightness_change),
                frame_data={'brightness': brightness}
            )
        
        return None
    
    def _check_object_anomaly(self, analysis_result) -> Optional[Anomaly]:
        """Check for object-related anomalies."""
        if not analysis_result.objects:
            return None
        
        # Check for suspicious objects (e.g., person in restricted area)
        suspicious_classes = ['person', 'bag', 'backpack']
        
        for obj in analysis_result.objects:
            if obj['class_name'] in suspicious_classes:
                # Check if object is near edges (possible tampering)
                bbox = obj['bbox']
                frame_width = 1920  # Assume standard width
                frame_height = 1080
                
                x1, y1, x2, y2 = bbox
                is_near_edge = (x1 < 50 or x2 > frame_width - 50 or 
                              y1 < 50 or y2 > frame_height - 50)
                
                if is_near_edge:
                    return Anomaly(
                        type="object_anomaly",
                        severity="medium",
                        description=f"Suspicious {obj['class_name']} near edge",
                        confidence=obj['confidence'],
                        frame_data={'object': obj}
                    )
        
        return None
    
    def _update_history(self, analysis_result):
        """Update historical data with new analysis results."""
        self.qoe_history.append(analysis_result.qoe_metrics.get('overall_qoe', 0))
        self.motion_history.append(analysis_result.motion_intensity)
        self.brightness_history.append(analysis_result.brightness)
    
    def get_status(self) -> dict:
        """Get detector status."""
        return {
            'qoe_history_length': len(self.qoe_history),
            'motion_history_length': len(self.motion_history),
            'brightness_history_length': len(self.brightness_history),
            'alert_interval': self.alert_interval
        }
