# Technical Architecture — Qelera

## 1. System Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                         VISIONTRACK PLATFORM                        │
│                                                                     │
│  ┌─────────────┐    ┌──────────────┐    ┌──────────────────────┐   │
│  │  Device      │    │  Detection   │    │   Analytics &        │   │
│  │  Cluster     │───▶│  Pipeline    │───▶│   Cloud Platform     │   │
│  │              │    │              │    │                      │   │
│  │  • AVerMedia │    │  • Frame     │    │  • KPI Dashboard     │   │
│  │    GC573     │    │    Differencing│  │  • Alert Engine      │   │
│  │  • Basler    │    │  • Optical   │    │  • Test Runner       │   │
│  │    ace2      │    │    Flow      │    │  • API Gateway       │   │
│  │  • IR/BT     │    │  • Template  │    │  • Team Management   │   │
│  │    Control   │    │    Matching  │    │  • Storage           │   │
│  └─────────────┘    │  • MOG2+YOLO │    └──────────────────────┘   │
│                     │  • AI Fallback │                              │
│                     └──────────────┘                                │
└─────────────────────────────────────────────────────────────────────┘
```

## 2. Detection Pipeline (Core Algorithm)

### Step 1: Frame Differencing (Every Frame)
```python
prev_gray = cv2.cvtColor(prev_frame, cv2.COLOR_BGR2GRAY)
curr_gray = cv2.cvtColor(curr_frame, cv2.COLOR_BGR2GRAY)
diff = cv2.absdiff(prev_gray, curr_gray)
baseline = running_average(diff)
std_dev = rolling_std(diff)
if change > baseline + k * std_dev:
    event_candidate = True
```

### Step 2: Event Classification (Multi-Path)

| Path | Method | Purpose | Source |
|------|--------|---------|--------|
| A | Histogram Bhattacharyya distance | Cold start transitions | Meta Doc + PMC 2023 |
| B | Template matching (TM_CCOEFF_NORMED) | ALD — logo, nav, content | Meta Doc |
| C | Dense optical flow (Farneback) | FVILD — motion entropy | Meta Doc + Farneback study |
| D | Background subtraction (MOG2) | Spinner vs. video | MOG2+YOLO paper 2024 |

### Step 3: AI Fallback (If Confidence < 0.8)
```python
if confidence < 0.8:
    result = yolo_v8_nano.predict(frame)
    # MobileNetV3 for lightweight classification
    result = mobilenet_v3.predict(frame)
```

### Precision Targets by Metric

| Metric | Target | Implementation |
|--------|--------|---------------|
| Cold Start | 4-6ms | Frame diff + template match |
| ALD | 4-6ms | Template match (logo + nav) |
| FVILD | 2-6ms | Farneback optical flow entropy |
| OVILD | 4ms | FVILD per swipe loop |
| REBUF | 4ms | MOG2 + YOLOv8 Nano |
| Upload Latency | 4-8ms | OCR (Tesseract) + template match |

## 3. Hardware Capture Layer

### Android (HDMI Capture)
```
Android Device → HDMI Out → AVerMedia GC573 → USB 3.0 → Compute Node
```
- **Capture rate:** Up to 240fps at 1080p60
- **Latency:** ~4ms (capture card processing)
- **Sync:** Hardware GPIO trigger via Arduino Nano 33 BLE Sense

### iOS (Camera Capture)
```
iPhone Screen → Basler ace2 Camera → USB 3.0 → Compute Node
```
- **Capture rate:** Up to 155fps at 1920×1200
- **Shutter:** Global shutter (eliminates rolling distortion)
- **Sync:** NTP + hardware GPIO from same Arduino

### Device Control Layer
```
Compute Node → IR Blaster (BroadLink RM4) → Device IR Receiver
Compute Node → Bluetooth Dongle → BLE Remote / RF4CE
Compute Node → GPIO (Arduino) → Hardware trigger / sensor tap
```

## 4. Software Stack

### Core Processing (Python)
| Library | Purpose | Version |
|---------|---------|---------|
| OpenCV 4.8+ | Frame processing, optical flow, template matching | Latest stable |
| YOLOv8 (Ultralytics) | AI classification fallback | 8.x |
| TensorFlow Lite | On-device preprocessing | 2.16+ |
| NumPy/SciPy | Signal processing, statistics | Latest |
| scrcpy | Android screen mirroring (fallback) | Latest |
| Tesseract OCR | Progress bar / toast detection | 5.x |

### Backend Services
| Service | Purpose | Technology |
|---------|---------|-----------|
| API Gateway | REST API for device management | FastAPI + uvicorn |
| Event Queue | Real-time event distribution | Redis Streams |
| Metric Storage | Time-series metric storage | PostgreSQL + TimescaleDB |
| Task Scheduler | Test execution scheduling | Celery + Redis |
| WebSocket Server | Real-time dashboard updates | FastAPI WebSocket |

### Monitoring & Analytics
| Component | Purpose | Technology |
|-----------|---------|-----------|
| Dashboard | KPI views, trends, alerts | Grafana or custom React |
| Alert Engine | Threshold-based notifications | Python + Slack/Email/PD |
| Reporting | Custom reports, exports | Python + PDF generation |

### Deployment
| Component | Purpose | Technology |
|-----------|---------|-----------|
| Containerization | Isolated service deployment | Docker + docker-compose |
| Orchestration | Multi-node cluster management | Kubernetes (optional) |
| CI/CD | Automated testing and deployment | GitHub Actions |

## 5. Data Flow

```
Frame Capture → Preprocessing → Feature Extraction → Classification → Timestamp → Storage → API → Dashboard
     ↓              ↓                 ↓                  ↓              ↓          ↓        ↓         ↓
   120-240fps    Grayscale         Frame diff         Template       T0→T1    Postgres  FastAPI   React
   (Android)     + Normalization   + Optical flow     Match          Latency  + Redis   WS push   Dashboard
   155fps         ROI crop          MOG2 + YOLO        AI fallback    JSON     Grafana   REST      Mobile App
   (iOS)
```

## 6. Critical Guardrails (From Meta Doc)

> **CRITICAL GUARDRAIL: ALL detection is visual-only at core. Audio is supplementary, never required for any metric calculation.**

Test every detector against:
1. Muted video
2. Static image
3. Slideshow
4. Black screen with no audio

This ensures our metrics are robust and not dependent on audio tracks that may vary.

## 7. Special Challenges & Solutions

| Challenge | Solution | Source |
|-----------|----------|--------|
| Spinner vs. Video | Optical flow entropy (spinner = uniform rotational; video = spatially incoherent) | Meta Doc |
| Uniform Screens | Histogram variance (near-zero for loading, textured for dark video) | Meta Doc |
| App Quirks | Programmable state machine handles per-app flow variations | Meta Doc |
| Two-Phone Sync | Shared T0 via GPIO or NTP sync; both capture cards on same host PC | Meta Doc |
| Low Light | Basler ace2 global shutter + IR illumination for iOS capture | Camera spec |
| High Motion Blur | Global shutter + high FPS (120+) eliminates motion artifacts | Camera spec |
