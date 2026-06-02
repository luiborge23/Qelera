# Qelera ML Pipeline — YOLOv8 → TFLite v1.0
# PyTorch, YOLOv8, ONNX, TensorFlow Lite
# Created: May 28, 2026

# ============================================================
# PROJECT STRUCTURE
# ============================================================
# ml-pipeline/
# ├── config/
# │   ├── training.yaml             # Training hyperparameters
# │   ├── model_config.yaml         # Model architecture config
# │   └── export_config.yaml        # Export settings
# ├── data/
# │   ├── annotations/              # Ground truth labels
# │   ├── splits/                   # Train/val/test splits
# │   └── preprocessing/            # Data augmentation scripts
# ├── notebooks/
# │   ├── 01_data_exploration.ipynb
# │   ├── 02_model_training.ipynb
# │   ├── 03_model_evaluation.ipynb
# │   └── 04_model_export.ipynb
# ├── src/
# │   ├── __init__.py
# │   ├── dataset.py                # Dataset loaders
# │   ├── models/
# │   │   ├── __init__.py
# │   │   ├── base_model.py         # Base model class
# │   │   ├── yolo_v8_custom.py     # Custom YOLOv8 variant
# │   │   ├── anomaly_detector.py   # Anomaly detection head
# │   │   └── quality_estimator.py  # Quality score estimator
# │   ├── training/
# │   │   ├── __init__.py
# │   │   ├── trainer.py            # Training loop
# │   │   ├── callbacks.py          # Custom callbacks
# │   │   └── augmentation.py       # Data augmentation
# │   ├── evaluation/
# │   │   ├── __init__.py
# │   │   ├── metrics.py            # Custom QoE metrics
# │   │   ├── confusion_matrix.py   # Classification eval
# │   │   └── benchmark.py          # Performance benchmarking
# │   ├── export/
# │   │   ├── __init__.py
# │   │   ├── onnx_export.py        # ONNX conversion
# │   │   ├── tflite_export.py      # TFLite conversion
# │   │   └── quantization.py       # INT8/FP16 quantization
# │   ├── inference/
# │   │   ├── __init__.py
# │   │   ├── predictor.py          # Inference engine
# │   │   ├── post_processing.py    # NMS, filtering
# │   │   └── streaming.py          # Real-time inference
# │   └── utils/
# │       ├── __init__.py
# │       ├── logger.py             # Structured logging
# │       ├── config.py             # Config management
# │       └── visualization.py      # Plotting utilities
# ├── tests/
# │   ├── test_dataset.py
# │   ├── test_models.py
# │   ├── test_training.py
# │   ├── test_export.py
# │   └── test_inference.py
# ├── requirements.txt
# ├── requirements-dev.txt
# ├── Dockerfile
# └── Makefile

# ============================================================
# REQUIREMENTS (requirements.txt)
# ============================================================
# torch==2.3.0
# torchvision==0.18.0
# ultralytics==8.2.0
# onnx==1.16.0
# onnxruntime==1.18.0
# tensorflow==2.16.1
# tensorflow-lite==2.16.1
# openvino==2024.1.0
# numpy==1.26.4
# opencv-python==4.9.0.80
# Pillow==10.3.0
# scikit-learn==1.5.1
# pandas==2.2.2
# matplotlib==3.8.4
# seaborn==0.13.2
# tensorboard==2.16.2
# wandb==0.17.0
# pyyaml==6.0.1
# tqdm==4.66.4
# albumentations==1.4.10
# pycocotools==2.0.7
# imageio==2.34.1
# scikit-image==0.23.2
# joblib==1.4.0
# huggingface-hub==0.23.0

# ============================================================
# TRAINING CONFIG (config/training.yaml)
# ============================================================
# dataset:
#   train_path: data/splits/train.txt
#   val_path: data/splits/val.txt
#   test_path: data/splits/test.txt
#   num_classes: 8  # no_anomaly, motion_blur, freeze_frame, color_distortion, audio_video_desync, resolution_drop, bitrate_spike, buffering
#   image_size: 640
#   batch_size: 32
#   workers: 8
#
# model:
#   name: yolo11n
#   pretrained: true
#   backbone: "cspdarknet53"
#   neck: "pan"
#   head: "yolo_head"
#   freeze_epochs: 10
#   anchor_scale: 1.0
#   iou_threshold: 0.45
#   obj_threshold: 0.25
#
# training:
#   epochs: 150
#   lr_initial: 0.01
#   lr_final: 0.0001
#   weight_decay: 0.0005
#   momentum: 0.937
#   warmup_epochs: 3
#   warmup_momentum: 0.8
#   warmup_bias_lr: 0.1
#   early_stopping_patience: 20
#   checkpoint_interval: 10
#   save_best_only: true
#
# augmentation:
#   hsv_h: 0.015
#   hsv_s: 0.7
#   hsv_v: 0.4
#   degrees: 0.0
#   translate: 0.1
#   scale: 0.5
#   shear: 0.0
#   perspective: 0.0
#   flipud: 0.0
#   fliplr: 0.5
#   mosaic: 1.0
#   mixup: 0.15
#   copy_paste: 0.0
#
# export:
#   format: tflite
#   int8_quantize: true
#   fp16_quantize: true
#   optimization: speed
#   input_shape: [1, 640, 640, 3]
#   representative_dataset: data/splits/representative_samples/
#
# hardware_targets:
#   android_gpu:
#     backend: "GPU_DELEGATE"
#     num_threads: 4
#   android_nnapi:
#     backend: "NNAPI"
#     num_threads: 2
#   edge_tpu:
#     backend: "EDGETPU"
#     num_threads: 2

# ============================================================
# YOLOv8 CUSTOM MODEL (src/models/yolo_v8_custom.py)
# ============================================================
# import torch
# import torch.nn as nn
# from ultralytics import YOLO
#
# class QualityEstimationHead(nn.Module):
#     """Custom head for QoE score estimation"""
#     def __init__(self, num_classes=8, num_qoe_bins=10):
#         super().__init__()
#         self.classification = nn.Sequential(
#             nn.Conv2d(256, 128, 3, padding=1),
#             nn.ReLU(),
#             nn.Conv2d(128, 64, 3, padding=1),
#             nn.ReLU(),
#             nn.AdaptiveAvgPool2d(1),
#             nn.Flatten(),
#             nn.Linear(64, num_classes)
#         )
#         self.qoe_regression = nn.Sequential(
#             nn.Conv2d(256, 128, 3, padding=1),
#             nn.ReLU(),
#             nn.Conv2d(128, 64, 3, padding=1),
#             nn.ReLU(),
#             nn.AdaptiveAvgPool2d(1),
#             nn.Flatten(),
#             nn.Linear(64, num_qoe_bins)
#         )
#
#     def forward(self, x):
#         cls_out = self.classification(x)
#         qoe_out = self.qoe_regression(x)
#         return cls_out, qoe_out
#
# class VisionTrackYOLO(YOLO):
#     """Custom YOLOv8 for Qelera anomaly detection"""
#     def __init__(self, cfg, num_classes=8):
#         super().__init__(cfg)
#         self.num_classes = num_classes
#         self.model.head = QualityEstimationHead(num_classes=num_classes)
#
#     def train_step(self, batch):
#         """Custom training step with QoE loss"""
#         predictions = self.model(batch["image"])
#         cls_loss = self.cls_criterion(predictions[0], batch["class_labels"])
#         qoe_loss = self.qoe_criterion(predictions[1], batch["qoe_score"])
#         total_loss = cls_loss + 0.5 * qoe_loss
#         return total_loss, cls_loss, qoe_loss
#
#     def evaluate(self, dataloader):
#         """Custom evaluation with QoE metrics"""
#         results = super().evaluate(dataloader)
#         # Add QoE-specific metrics
#         results["qoe_accuracy"] = self.compute_qoe_accuracy()
#         results["qoe_mae"] = self.compute_qoe_mae()
#         return results

# ============================================================
# TFLITE EXPORT (src/export/tflite_export.py)
# ============================================================
# import tensorflow as tf
# import onnx
# import onnxruntime as ort
# import numpy as np
# from pathlib import Path
#
# class TFLiteExporter:
#     """Export models to TensorFlow Lite format"""
#
#     def __init__(self, model_path: str, config: dict):
#         self.model_path = model_path
#         self.config = config
#         self.output_dir = Path(config["output_dir"])
#         self.output_dir.mkdir(parents=True, exist_ok=True)
#
#     def export_onnx(self) -> str:
#         """Convert PyTorch model to ONNX"""
#         import torch
#         from torch.onnx import export
#
#         model = self._load_pytorch_model()
#         model.eval()
#
#         dummy_input = torch.randn(1, 3, 640, 640)
#         onnx_path = self.output_dir / "model.onnx"
#
#         export(
#             model,
#             dummy_input,
#             str(onnx_path),
#             input_names=["input"],
#             output_names=["class_output", "qoe_output"],
#             dynamic_axes={
#                 "input": {0: "batch_size"},
#                 "class_output": {0: "batch_size"},
#                 "qoe_output": {0: "batch_size"}
#             },
#             opset_version=17
#         )
#
#         # Verify ONNX model
#         onnx_model = onnx.load(str(onnx_path))
#         onnx.checker.check_model(onnx_model)
#
#         return str(onnx_path)
#
#     def export_tflite_int8(self, representative_data_gen) -> str:
#         """Convert to TFLite with INT8 quantization"""
#         onnx_path = self.export_onnx()
#
#         # Convert ONNX to TFLite
#         converter = tf.lite.TFLiteConverter.from_onnx_file(onnx_path)
#         converter.optimizations = [tf.lite.Optimize.DEFAULT]
#         converter.representative_dataset = representative_data_gen
#         converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
#         converter.inference_input_type = tf.int8
#         converter.inference_output_type = tf.int8
#
#         tflite_path = self.output_dir / "model_int8.tflite"
#         tflite_model = converter.convert()
#
#         with open(tflite_path, "wb") as f:
#             f.write(tflite_model)
#
#         return str(tflite_path)
#
#     def export_tflite_fp16(self) -> str:
#         """Convert to TFLite with FP16 quantization"""
#         onnx_path = self.export_onnx()
#
#         converter = tf.lite.TFLiteConverter.from_onnx_file(onnx_path)
#         converter.optimizations = [tf.lite.Optimize.DEFAULT]
#         converter.target_spec.supported_types = [tf.float16]
#
#         tflite_path = self.output_dir / "model_fp16.tflite"
#         tflite_model = converter.convert()
#
#         with open(tflite_path, "wb") as f:
#             f.write(tflite_model)
#
#         return str(tflite_path)
#
#     def benchmark_models(self) -> dict:
#         """Benchmark all exported models"""
#         results = {}
#         for model_file in self.output_dir.glob("*.tflite"):
#             interpreter = tf.lite.Interpreter(model_path=str(model_file))
#             interpreter.allocate_tensors()
#
#             input_details = interpreter.get_input_details()
#             output_details = interpreter.get_output_details()
#
#             # Benchmark inference
#             num_runs = 100
#             times = []
#             for _ in range(num_runs):
#                 input_data = np.random.randn(*input_details[0]["shape"]).astype(input_details[0]["dtype"])
#                 interpreter.set_tensor(input_details[0]["index"], input_data)
#                 start = time.time()
#                 interpreter.invoke()
#                 end = time.time()
#                 times.append(end - start)
#
#             results[model_file.name] = {
#                 "avg_inference_ms": np.mean(times) * 1000,
#                 "min_inference_ms": np.min(times) * 1000,
#                 "max_inference_ms": np.max(times) * 1000,
#                 "model_size_mb": model_file.stat().st_size / (1024 * 1024),
#                 "quantization": "INT8" if "int8" in model_file.name else "FP16"
#             }
#
#         return results

# ============================================================
# INFERENCE ENGINE (src/inference/predictor.py)
# ============================================================
# import numpy as np
# import cv2
# from pathlib import Path
# from typing import List, Dict, Optional
#
# class VisionTrackPredictor:
#     """Real-time inference engine for edge devices"""
#
#     def __init__(self, model_path: str, backend: str = "GPU_DELEGATE"):
#         self.model_path = model_path
#         self.backend = backend
#         self.interpreter = tf.lite.Interpreter(
#             model_path=model_path,
#             experimental_delegates=[tf.lite.experimental.load_delegate(backend)]
#         )
#         self.interpreter.allocate_tensors()
#
#         self.input_details = self.interpreter.get_input_details()
#         self.output_details = self.interpreter.get_output_details()
#
#         self.input_shape = self.input_details[0]["shape"]
#         self.input_mean = self.input_details[0].get("mean", 127.5)
#         self.input_std = self.input_details[0].get("std", 127.5)
#
#     def preprocess(self, frame: np.ndarray) -> np.ndarray:
#         """Preprocess frame for model input"""
#         # Resize and normalize
#         resized = cv2.resize(frame, (640, 640))
#         normalized = (resized - self.input_mean) / self.input_std
#         normalized = normalized.astype(np.float32)
#         return np.expand_dims(normalized, axis=0)
#
#     def predict(self, frame: np.ndarray) -> Dict:
#         """Run inference on a single frame"""
#         input_data = self.preprocess(frame)
#         self.interpreter.set_tensor(self.input_details[0]["index"], input_data)
#
#         start_time = time.time()
#         self.interpreter.invoke()
#         inference_time = (time.time() - start_time) * 1000
#
#         class_output = self.interpreter.get_tensor(self.output_details[0]["index"])
#         qoe_output = self.interpreter.get_tensor(self.output_details[1]["index"])
#
#         # Post-processing
#         anomaly_class = np.argmax(class_output[0])
#         confidence = np.max(class_output[0])
#         qoe_score = np.argmax(qoe_output[0]) * 10  # Map to 0-100
#
#         return {
#             "anomaly_class": anomaly_class,
#             "confidence": float(confidence),
#             "qoe_score": float(qoe_score),
#             "inference_time_ms": inference_time
#         }
#
#     def predict_stream(self, video_source: str, callback: callable):
#         """Real-time streaming inference"""
#         cap = cv2.VideoCapture(video_source)
#
#         frame_count = 0
#         while cap.isOpened():
#             ret, frame = cap.read()
#             if not ret:
#                 break
#
#             result = self.predict(frame)
#             callback(frame, result)
#
#             frame_count += 1
#             if frame_count % 30 == 0:
#                 print(f"Processed {frame_count} frames")
#
#         cap.release()

# ============================================================
# DATALOADER (src/dataset.py)
# ============================================================
# import torch
# from torch.utils.data import Dataset, DataLoader
# import cv2
# import numpy as np
# import json
# from pathlib import Path
#
# class VisionTrackDataset(Dataset):
#     """Dataset for Qelera anomaly detection"""
#
#     def __init__(self, img_dir: str, annotations_file: str, transform=None):
#         self.img_dir = Path(img_dir)
#         with open(annotations_file, "r") as f:
#             self.annotations = json.load(f)
#         self.transform = transform
#
#     def __len__(self):
#         return len(self.annotations)
#
#     def __getitem__(self, idx):
#         ann = self.annotations[idx]
#         img_path = self.img_dir / ann["image"]
#         image = cv2.imread(str(img_path))
#         image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
#
#         if self.transform:
#             image = self.transform(image)
#
#         label = ann["class"]
#         qoe_score = ann["qoe_score"]
#
#         return {
#             "image": image,
#             "class_label": label,
#             "qoe_score": qoe_score
#         }
#
# def get_dataloaders(config: dict) -> tuple:
#     """Create train/val/test dataloaders"""
#     from torchvision import transforms
#
#     train_transform = transforms.Compose([
#         transforms.RandomResizedCrop(640),
#         transforms.RandomHorizontalFlip(),
#         transforms.ColorJitter(brightness=0.2, contrast=0.2, saturation=0.2),
#         transforms.ToTensor(),
#         transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
#     ])
#
#     val_transform = transforms.Compose([
#         transforms.Resize(640),
#         transforms.CenterCrop(640),
#         transforms.ToTensor(),
#         transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
#     ])
#
#     train_dataset = VisionTrackDataset(
#         config["dataset"]["train_path"],
#         transform=train_transform
#     )
#     val_dataset = VisionTrackDataset(
#         config["dataset"]["val_path"],
#         transform=val_transform
#     )
#
#     train_loader = DataLoader(
#         train_dataset,
#         batch_size=config["dataset"]["batch_size"],
#         shuffle=True,
#         num_workers=config["dataset"]["workers"],
#         pin_memory=True
#     )
#     val_loader = DataLoader(
#         val_dataset,
#         batch_size=config["dataset"]["batch_size"],
#         shuffle=False,
#         num_workers=config["dataset"]["workers"],
#         pin_memory=True
#     )
#
#     return train_loader, val_loader

# ============================================================
# DOCKERFILE (ML Pipeline)
# ============================================================
# FROM nvidia/cuda:12.2.0-base-ubuntu22.04
#
# WORKDIR /app
#
# # Install system dependencies
# RUN apt-get update && apt-get install -y \
#     python3 python3-pip git wget \
#     libgl1-mesa-glx libglib2.0-0 \
#     && rm -rf /var/lib/apt/lists/*
#
# # Install Python dependencies
# COPY requirements.txt .
# RUN pip install --no-cache-dir -r requirements.txt
#
# # Copy application
# COPY . .
#
# # Create non-root user
# RUN useradd -m -u 1000 appuser
# USER appuser
#
# # Default command
# CMD ["python", "src/train.py"]
