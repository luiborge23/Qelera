# ============================================================
# Frame Analyzer — OpenCV Frame Processing
# ============================================================
# Python 3.12 | OpenCV
# Real-time frame analysis with motion detection and QoE metrics
# ============================================================

import cv2
import numpy as np
import asyncio
import logging
from datetime import datetime
from dataclasses import dataclass, field
from typing import List, Optional, Dict

logger = logging.getLogger('FrameAnalyzer')


@dataclass
class FrameAnalysisResult:
    """Result of frame analysis."""
    timestamp: str = field(default_factory=lambda: datetime.utcnow().isoformat())
    frame_number: int = 0
    motion_detected: bool = False
    motion_intensity: float = 0.0
    brightness: float = 0.0
    contrast: float = 0.0
    sharpness: float = 0.0
    objects: List[Dict] = field(default_factory=list)
    qoe_metrics: Dict = field(default_factory=dict)
    processing_time_ms: float = 0.0


class FrameAnalyzer:
    """OpenCV-based frame analyzer."""
    
    def __init__(self, config: dict):
        """Initialize the frame analyzer."""
        self.config = config
        self.frame_number = 0
        self.prev_frame: Optional[np.ndarray] = None
        self.motion_history = []
        self.max_history = 30
        
        # Configuration
        self.motion_threshold = config.get('anomaly_detection', {}).get('motion_threshold', 0.02)
        
        logger.info("FrameAnalyzer initialized")
    
    async def analyze(self, frame: np.ndarray) -> FrameAnalysisResult:
        """Analyze a single frame."""
        start_time = datetime.utcnow()
        
        # Convert to grayscale for analysis
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY) if len(frame.shape) == 3 else frame
        
        # Calculate metrics
        brightness = float(np.mean(gray))
        contrast = float(np.std(gray))
        sharpness = self._calculate_sharpness(gray)
        
        # Detect motion
        motion_detected, motion_intensity = self._detect_motion(gray)
        
        # Detect objects
        objects = self._detect_objects(frame)
        
        # Calculate QoE metrics
        qoe_metrics = self._calculate_qoe_metrics(brightness, contrast, sharpness, motion_intensity)
        
        # Update history
        self.motion_history.append(motion_intensity)
        if len(self.motion_history) > self.max_history:
            self.motion_history.pop(0)
        
        # Create result
        result = FrameAnalysisResult(
            frame_number=self.frame_number,
            motion_detected=motion_detected,
            motion_intensity=motion_intensity,
            brightness=brightness,
            contrast=contrast,
            sharpness=sharpness,
            objects=objects,
            qoe_metrics=qoe_metrics
        )
        
        # Update processing time
        end_time = datetime.utcnow()
        result.processing_time_ms = (end_time - start_time).total_seconds() * 1000
        
        self.frame_number += 1
        
        return result
    
    def _calculate_sharpness(self, gray: np.ndarray) -> float:
        """Calculate image sharpness using variance of Laplacian."""
        laplacian = cv2.Laplacian(gray, cv2.CV_64F)
        return float(laplacian.var())
    
    def _detect_motion(self, gray: np.ndarray) -> tuple:
        """Detect motion using frame differencing."""
        if self.prev_frame is None:
            self.prev_frame = gray
            return False, 0.0
        
        # Calculate frame difference
        frame_diff = cv2.absdiff(self.prev_frame, gray)
        thresh = cv2.threshold(frame_diff, 25, 255, cv2.THRESH_BINARY)[1]
        
        # Count changed pixels
        changed_pixels = cv2.countNonZero(thresh)
        total_pixels = gray.shape[0] * gray.shape[1]
        motion_intensity = changed_pixels / total_pixels
        
        # Determine if motion is significant
        motion_detected = motion_intensity > self.motion_threshold
        
        # Update previous frame
        self.prev_frame = gray.copy()
        
        return motion_detected, motion_intensity
    
    def _detect_objects(self, frame: np.ndarray) -> List[Dict]:
        """Detect objects in frame using YOLOv8."""
        objects = []
        
        try:
            # Load YOLOv8 model
            from ultralytics import YOLO
            
            model_path = self.config.get('anomaly_detection', {}).get('yolo_model', 'yolov8n.pt')
            model = YOLO(model_path)
            
            # Run inference
            confidence_threshold = self.config.get('anomaly_detection', {}).get('confidence_threshold', 0.5)
            results = model(frame, conf=confidence_threshold, verbose=False)
            
            # Extract objects
            for result in results:
                boxes = result.boxes
                for box in boxes:
                    x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                    confidence = float(box.conf[0])
                    class_id = int(box.cls[0])
                    
                    objects.append({
                        'class_id': class_id,
                        'class_name': model.names[class_id],
                        'confidence': confidence,
                        'bbox': [int(x1), int(y1), int(x2), int(y2)]
                    })
        
        except Exception as e:
            logger.error(f"Object detection failed: {e}")
        
        return objects
    
    def _calculate_qoe_metrics(self, brightness: float, contrast: float, 
                               sharpness: float, motion_intensity: float) -> Dict:
        """Calculate Quality of Experience metrics."""
        # Brightness score (0-100, optimal around 128)
        brightness_score = max(0, 100 - abs(brightness - 128) / 128 * 100)
        
        # Contrast score (0-100, higher is better)
        contrast_score = min(100, contrast / 50 * 100)
        
        # Sharpness score (0-100, higher is better)
        sharpness_score = min(100, sharpness / 1000 * 100)
        
        # Overall QoE score
        qoe_score = (brightness_score * 0.3 + contrast_score * 0.3 + 
                    sharpness_score * 0.4)
        
        return {
            'brightness_score': round(brightness_score, 2),
            'contrast_score': round(contrast_score, 2),
            'sharpness_score': round(sharpness_score, 2),
            'overall_qoe': round(qoe_score, 2),
            'motion_intensity': round(motion_intensity, 4)
        }
    
    def get_status(self) -> dict:
        """Get analyzer status."""
        return {
            'frame_number': self.frame_number,
            'motion_history_length': len(self.motion_history),
            'motion_threshold': self.motion_threshold
        }
