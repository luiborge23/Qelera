# ============================================================
# GStreamer Pipeline — Video Ingestion
# ============================================================
# Python 3.12 | GStreamer | OpenCV
# Handles RTSP streams, USB cameras, and frame extraction
# ============================================================

import asyncio
import cv2
import gi
import numpy as np
from datetime import datetime
from typing import Optional, Callable

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

# Initialize GStreamer
Gst.init(None)


class GStreamerPipeline:
    """GStreamer pipeline for video ingestion."""
    
    def __init__(self, config: dict):
        """Initialize the GStreamer pipeline."""
        self.config = config
        self.pipeline = None
        self.bus = None
        self.running = False
        self.frame_queue = asyncio.Queue(maxsize=10)
        self.current_frame: Optional[np.ndarray] = None
        self.fps = config.get('video', {}).get('fps', 30)
        self.resolution = config.get('video', {}).get('resolution', '1920x1080')
        
        # Callback for frame processing
        self.frame_callback: Optional[Callable] = None
        
        logger = __import__('logging').getLogger('GStreamerPipeline')
        self.logger = logger
    
    def _create_pipeline(self) -> Gst.Pipeline:
        """Create the GStreamer pipeline based on source type."""
        source_type = self.config.get('video', {}).get('source_type', 'usb')
        
        if source_type == 'rtsp':
            return self._create_rtsp_pipeline()
        elif source_type == 'usb':
            return self._create_usb_pipeline()
        elif source_type == 'file':
            return self._create_file_pipeline()
        else:
            raise ValueError(f"Unknown source type: {source_type}")
    
    def _create_rtsp_pipeline(self) -> Gst.Pipeline:
        """Create pipeline for RTSP stream."""
        rtsp_url = self.config['video'].get('rtsp_url', '')
        
        # GStreamer pipeline for RTSP with hardware acceleration
        pipeline_str = (
            f"rtspsrc location={rtsp_url} latency=400 ! "
            "rtph264depay ! "
            "h264parse ! "
            "nvv4l2decoder ! "
            "nv3dsink sync=false"
        )
        
        # Fallback to software decoding if VPU not available
        if not self._has_vpu():
            pipeline_str = (
                f"rtspsrc location={rtsp_url} latency=400 ! "
                "rtph264depay ! "
                "h264parse ! "
                "nvv4l2decoder ! "
                "queue ! "
                "videoconvert ! "
                "video/x-raw,format=BGR ! "
                "appsink name=appsink sync=false max-buffers=10 drop=true"
            )
        
        self.logger.info(f"Creating RTSP pipeline: {pipeline_str}")
        return Gst.parse_launch(pipeline_str)
    
    def _create_usb_pipeline(self) -> Gst.Pipeline:
        """Create pipeline for USB camera."""
        usb_device = self.config['video'].get('usb_device', '/dev/video0')
        
        # GStreamer pipeline for USB camera
        pipeline_str = (
            f"v4l2src device={usb_device} ! "
            f"video/x-raw,width={self.resolution.split('x')[0]},height={self.resolution.split('x')[1]},framerate={self.fps}/1 ! "
            "videoconvert ! "
            "video/x-raw,format=BGR ! "
            "appsink name=appsink sync=false max-buffers=10 drop=true"
        )
        
        self.logger.info(f"Creating USB pipeline: {pipeline_str}")
        return Gst.parse_launch(pipeline_str)
    
    def _create_file_pipeline(self) -> Gst.Pipeline:
        """Create pipeline for video file."""
        file_path = self.config['video'].get('file_path', '')
        
        pipeline_str = (
            f"filesrc location={file_path} ! "
            "qtdemux ! "
            "h264parse ! "
            "nvv4l2decoder ! "
            "videoconvert ! "
            "video/x-raw,format=BGR ! "
            "appsink name=appsink sync=false max-buffers=10 drop=true"
        )
        
        self.logger.info(f"Creating file pipeline: {pipeline_str}")
        return Gst.parse_launch(pipeline_str)
    
    def _has_vpu(self) -> bool:
        """Check if VPU hardware acceleration is available."""
        try:
            # Check for NVIDIA VPU
            import subprocess
            result = subprocess.run(['lsmod'], capture_output=True, text=True)
            return 'nvidia' in result.stdout.lower()
        except:
            return False
    
    def _on_new_sample(self, appsink):
        """Callback for new frame from appsink."""
        sample = appsink.emit('pull-sample')
        if sample:
            buffer = sample.get_buffer()
            caps = sample.get_caps()
            
            # Get frame data
            success, buffer_data = buffer.map(Gst.MapFlags.READ)
            if success:
                # Get frame dimensions from caps
                structure = caps.get_structure(0)
                width = structure.get_value('width')
                height = structure.get_value('height')
                n_channels = 3  # BGR
                
                # Convert to numpy array
                frame = np.frombuffer(buffer_data, dtype=np.uint8)
                frame = frame.reshape((height, width, n_channels))
                
                # Store current frame
                self.current_frame = frame.copy()
                
                # Put in queue
                try:
                    self.frame_queue.put_nowait(frame)
                except asyncio.QueueFull:
                    # Drop frame if queue full
                    pass
                
                buffer.unmap(buffer_data)
        
        return Gst.FlowReturn.OK
    
    async def start(self):
        """Start the GStreamer pipeline."""
        self.logger.info("Starting GStreamer pipeline...")
        
        # Create pipeline
        self.pipeline = self._create_pipeline()
        
        # Get appsink element
        appsink = self.pipeline.get_by_name('appsink')
        if appsink:
            appsink.connect('new-sample', self._on_new_sample)
        
        # Set pipeline to PLAYING
        self.pipeline.set_state(Gst.State.PLAYING)
        self.bus = self.pipeline.get_bus()
        self.running = True
        
        # Start bus watch
        GLib.io_add_watch(self.bus, GLib.IO_IN, self._bus_callback)
        
        self.logger.info("GStreamer pipeline started")
    
    def _bus_callback(self, bus, message, loop):
        """Handle GStreamer bus messages."""
        if message.type == Gst.MessageType.ERROR:
            err, debug = message.parse_error()
            self.logger.error(f"GStreamer error: {err}, {debug}")
            self.running = False
        
        elif message.type == Gst.MessageType.EOS:
            self.logger.info("GStreamer pipeline reached end of stream")
            self.running = False
        
        return True
    
    async def get_frame(self) -> Optional[np.ndarray]:
        """Get the latest frame from the queue."""
        try:
            return self.frame_queue.get_nowait()
        except asyncio.QueueEmpty:
            return self.current_frame
    
    async def stop(self):
        """Stop the GStreamer pipeline."""
        self.logger.info("Stopping GStreamer pipeline...")
        
        if self.pipeline:
            self.pipeline.set_state(Gst.State.NULL)
            self.pipeline = None
        
        self.running = False
        self.logger.info("GStreamer pipeline stopped")
    
    def get_status(self) -> dict:
        """Get pipeline status."""
        return {
            'running': self.running,
            'fps': self.fps,
            'resolution': self.resolution,
            'source_type': self.config.get('video', {}).get('source_type'),
            'queue_size': self.frame_queue.qsize()
        }
