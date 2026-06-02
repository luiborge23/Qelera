# VISIONTRACK — TECHNICAL SPECIFICATIONS

**Version:** 1.0  
**Date:** May 28, 2026  
**Author:** Qelera Engineering Team  
**Status:** Draft  

---

## 1. SYSTEM ARCHITECTURE OVERVIEW

### 1.1 Architecture Pattern
Qelera uses a **hybrid edge-cloud microservices architecture**:
- **Edge layer**: Android SDK runs on-device, processes video/audio/sensor data locally
- **Cloud layer**: Microservices handle aggregation, analytics, and dashboard serving
- **Communication**: TLS 1.3 + mTLS with protocol buffering (Protobuf) for efficiency

### 1.2 Technology Stack

| Layer | Technology | Version | Rationale |
|-------|-----------|---------|-----------|
| Android SDK | Kotlin + NDK | 1.9+ / NDK r26 | Native performance for AI inference |
| Edge AI | TensorFlow Lite | 2.15+ | Industry-standard mobile ML runtime |
| Object Detection | YOLOv8-nano | INT8 quantized | 30+ FPS on mobile, smallest model size |
| Tracking | DeepSORT | Custom | Multi-object tracking with Re-ID |
| Backend | FastAPI + Python | 3.14 | Async-native, high-performance |
| API Gateway | Kong | 3.5+ | Open-source, plugin ecosystem |
| Auth | OAuth 2.0 + JWT | RFC 6749 | Industry standard |
| Message Queue | Apache Kafka | 3.6+ | Distributed event streaming |
| Stream Processing | Apache Flink | 1.18+ | Real-time windowed processing |
| Cache | Redis Cluster | 7.2+ | Sub-millisecond reads |
| Primary DB | PostgreSQL + TimescaleDB | 16+ | Time-series optimized |
| Search | Elasticsearch | 8.11+ | Full-text and structured search |
| Object Store | AWS S3 | - | Video frame snapshots |
| CDN | AWS CloudFront | - | Global edge caching |
| Frontend | React + TypeScript | 18+ | Component-based, type-safe |
| Real-time | WebSocket (Socket.IO) | 4.7+ | Bidirectional communication |
| Container | Docker | 24+ | Reproducible environments |
| Orchestration | Kubernetes | 1.28+ | Auto-scaling, self-healing |
| IaC | Terraform | 1.6+ | Declarative infrastructure |
| CI/CD | GitHub Actions | - | Native GitHub integration |
| Monitoring | Prometheus + Grafana | 2.47+ / 10+ | Metrics and visualization |
| Logging | ELK Stack | 8.11+ | Centralized log management |
| Secrets | HashiCorp Vault | 1.15+ | Secret management |

---

## 2. ANDROID SDK SPECIFICATIONS

### 2.1 Package Structure

```
io.qelera.sdk/
├── core/
│   ├── VisionTrackEngine.kt          # Main entry point
│   ├── SessionManager.kt             # Session lifecycle
│   ├── Config.kt                     # Configuration model
│   └── Result.kt                     # Result types
├── capture/
│   ├── CameraCapture.kt              # Camera2 API integration
│   ├── AudioCapture.kt               # AudioRecord integration
│   ├── ScreenCapture.kt              # MediaProjection API
│   └── CapturePipeline.kt            # Orchestrates all capture
├── sensors/
│   ├── IMUSensor.kt                  # SensorManager accelerometer/gyro
│   ├── GPSSensor.kt                  # FusedLocationProvider
│   ├── WiFiSensor.kt                 # WifiManager RSSI scanning
│   ├── BluetoothSensor.kt            # BLE scanning (BLE 5.3)
│   └── NFCReader.kt                  # NFC reading (optional)
├── ai/
│   ├── YOLOv8Detector.kt             # TFLite object detection
│   ├── DeepSORTTracker.kt            # Multi-object tracking
│   ├── FrameProcessor.kt             # Frame preprocessing
│   └── ModelManager.kt               # Model loading/caching
├── qoe/
│   ├── QoECalculator.kt              # ITU-T P.1203 implementation
│   ├── PSNRCalculator.kt             # Peak Signal-to-Noise Ratio
│   ├── SSIMCalculator.kt             # Structural Similarity
│   ├── VMAFCalculator.kt             # Video Multimethod Assessment
│   └── QoEScore.kt                   # Composite QoE score
├── network/
│   ├── ConnectionManager.kt          # Network state monitoring
│   ├── DataUploader.kt               # Batched upload with retry
│   ├── WebSocketClient.kt            # Real-time dashboard updates
│   └── OfflineQueue.kt               # Local queue for offline
├── privacy/
│   ├── DifferentialPrivacy.kt        # Laplace mechanism
│   ├── DataAnonymizer.kt             # K-anonymity, L-diversity
│   └── SecureStorage.kt              # EncryptedSharedPreferences
├── federated/
│   ├── FederatedClient.kt            # FL training client
│   ├── ModelUpdater.kt               # Download model updates
│   └── SecAggProtocol.kt             # Secure Aggregation
├── reporting/
│   ├── ReportGenerator.kt            # PDF/CSV report generation
│   ├── AlertManager.kt               # Threshold-based alerts
│   └── DashboardPush.kt              # WebSocket push events
└── utils/
    ├── Logger.kt                     # Structured logging
    ├── PerformanceMonitor.kt         # FPS, memory, CPU tracking
    └── Extensions.kt                 # Kotlin extensions
```

### 2.2 Core API

```kotlin
// Initialization
val config = VisionTrackConfig(
    apiKey = "your-api-key",
    deviceId = "device-001",
    sessionMode = SessionMode.REALTIME,
    qoeMetrics = setOf(QoEMetric.ITU_P1203, QoEMetric.PSNR),
    aiEnabled = true,
    privacyLevel = PrivacyLevel.DIFFERENTIAL_PRIVACY
)

val engine = VisionTrackEngine.create(config)

// Start session
engine.startSession { sessionId ->
    // Session started, receiving QoE events
}

// QoE event callback
engine.onQoEEvent { event ->
    log("QoE: score=${event.score}, latency=${event.latency}")
}

// Stop session
engine.stopSession { result ->
    log("Session completed: ${result.summary}")
}
```

### 2.3 Camera Capture Specification

**API**: Camera2 (Android API 21+)
**Frame rate**: 30 FPS (configurable: 15, 30, 60)
**Resolution**: Auto-adaptive (1080p default, scales based on device capability)
**Format**: YUV_420_888 (optimized for TFLite)
**Pipeline**:
1. CameraCaptureSession captures frames
2. ImageReader converts to ByteBuffer
3. FrameProcessor resizes to 640x640 (YOLOv8 input)
4. ByteBuffer passed to YOLOv8Detector
5. Detection results queued for upload

**Performance targets**:
- Frame capture latency: <16ms (30 FPS)
- Preprocessing time: <2ms
- Total pipeline: <33ms per frame

### 2.4 Sensor Fusion Specification

**Sensor types and rates**:
| Sensor | Rate | Precision | Power Impact |
|--------|------|-----------|-------------|
| Accelerometer | 200 Hz | ±0.005g | Low |
| Gyroscope | 200 Hz | ±0.01 deg/s | Low |
| Magnetometer | 100 Hz | ±0.3 uT | Low |
| GPS | 1 Hz | ±2.5m | Medium |
| WiFi RSSI | 10 Hz | ±3 dB | Low |
| Bluetooth RSSI | 10 Hz | ±3 dB | Low |

**Fusion algorithm**:
- Extended Kalman Filter (EKF) for IMU + GPS fusion
- Transformer-based multi-modal fusion for cross-modal correlation
- Window size: 1 second (200 IMU samples, 10 WiFi samples)

### 2.5 Edge AI Specification

**Model**: YOLOv8-nano (Ultralytics)
**Input**: 640x640 RGB, 3 channels
**Output**: Bounding boxes + class probabilities
**Quantization**: INT8 (Post-training quantization with calibration set)
**Runtime**: TensorFlow Lite with NNAPI delegate
**Target FPS**: 30+ on mid-range devices (Snapdragon 860+)

**DeepSORT integration**:
- Re-ID network: MobileNetV2 (64-dim embedding)
- Association metric: Cosine distance + Mahalanobis
- Kalman filter: 8-state (x, y, aspect ratio, height, vx, vy, v_ratio, v_h)
- Track management: max_age=5, min_hits=3

### 2.6 QoE Calculation Specification

**ITU-T P.1203 (2017) Implementation**:
- Input: Video stream characteristics (bitrate, resolution, frame rate, codec)
- Network metrics: Latency, jitter, packet loss, bandwidth
- Output: Predicted MOS (Mean Opinion Score) 1.0-5.0
- Model: Pre-trained regression model (from ITU-T specification)

**PSNR Calculation**:
- Formula: PSNR = 10 * log10(MAX^2 / MSE)
- MAX = 255 (8-bit video)
- Computed per-frame between reference and distorted frames
- Typical range: 20-50 dB

**SSIM Calculation**:
- Sliding window: 11x11 pixels
- Constants: C1=(0.01*255)^2, C2=(0.03*255)^2
- Output: [-1, 1] where 1 = identical
- Computed per-frame, aggregated over session

**VMAF Calculation**:
- Netflix's Video Multimethod Assessment Fusion
- Combines multiple metrics: VIF, AD-SSIM, TV-PSNR, etc.
- Output: 0-100 scale
- Uses pre-trained model from Netflix research

**Composite QoE Score**:
```
QoE = 0.35 * P1203_normalized + 0.25 * PSNR_normalized + 
      0.20 * SSIM_normalized + 0.20 * Network_score
```

### 2.7 Network Communication Specification

**Protocol**: TLS 1.3 with mTLS
**Data format**: Protobuf (binary, compact)
**Transport**: HTTP/2 (multiplexed) + WebSocket (real-time push)

**Message types** (Protobuf):
```protobuf
message QoEReport {
    string session_id = 1;
    int64 timestamp = 2;
    float qoe_score = 3;
    float psnr = 4;
    float ssim = 5;
    float latency_ms = 6;
    float jitter_ms = 7;
    float packet_loss_pct = 8;
    repeated Detection detections = 9;
    repeated SensorReading sensors = 10;
}

message Detection {
    int32 track_id = 1;
    float x = 2;
    float y = 3;
    float width = 4;
    float height = 5;
    string class_name = 6;
    float confidence = 7;
}

message SensorReading {
    string type = 1; // "accelerometer", "gyroscope", "gps", "wifi", "bluetooth"
    float value1 = 2;
    float value2 = 3;
    float value3 = 4;
    int64 timestamp = 5;
}
```

**Upload strategy**:
- Real-time: WebSocket push for QoE events (every 1 second)
- Batch: HTTP POST for reports (every 30 seconds, batched)
- Offline: Local SQLite queue, sync when connection restored
- Retry: Exponential backoff (1s, 2s, 4s, 8s, 16s, max 30s)
- Max retry: 5 attempts per batch

### 2.8 Privacy Specification

**Differential Privacy** (Laplace mechanism):
- Epsilon (ε): 0.1 (high privacy)
- Delta (δ): 1e-5
- Sensitivity: 1.0 (normalized QoE score)
- Mechanism: Add Laplace(1/ε) noise to each QoE value before upload

**Data anonymization**:
- K-anonymity: k=10 (each record indistinguishable from 9 others)
- L-diversity: l=3 (at least 3 diverse sensitive values per equivalence class)
- Pseudonymization: Device ID hashed with HMAC-SHA256 (key rotated monthly)

**Secure storage**:
- EncryptedSharedPreferences for configuration
- SQLCipher for local SQLite database
- Android Keystore for cryptographic keys
- Key rotation: Every 30 days

### 2.9 Federated Learning Specification

**Algorithm**: Federated Averaging (FedAvg) with Secure Aggregation (SecAgg+)
**Training**:
- Local dataset: Last 1000 frames processed
- Batch size: 32
- Epochs: 3 per aggregation round
- Learning rate: 0.01 (Adam optimizer)
- Model: YOLOv8-nano (quantized)

**Aggregation**:
- Round interval: Every 24 hours
- Secure aggregation: SecAgg+ protocol (Bonawitz et al. 2019)
- Differential privacy: Clip gradients (norm=1.0), add Gaussian noise (σ=0.01)
- Model update: Download from cloud, merge with local model

**Privacy guarantees**:
- Individual contributions untraceable
- No raw data leaves device
- Model updates encrypted in transit and at rest

---

## 3. CLOUD SERVICES SPECIFICATIONS

### 3.1 API Gateway (Kong)

**Endpoints**:
| Method | Path | Description | Auth |
|--------|------|-------------|------|
| POST | /api/v1/sessions | Create session | JWT |
| GET | /api/v1/sessions/{id} | Get session details | JWT |
| GET | /api/v1/sessions/{id}/reports | Get session reports | JWT |
| POST | /api/v1/devices/register | Register device | API Key |
| GET | /api/v1/devices/{id} | Get device info | JWT |
| POST | /api/v1/devices/{id}/ota | OTA update | JWT + Admin |
| GET | /api/v1/dashboard/metrics | Real-time metrics | JWT |
| POST | /api/v1/reports/generate | Generate report | JWT |
| GET | /api/v1/health | Health check | None |

**Rate limiting**:
- Authenticated users: 1000 requests/minute
- API key users: 500 requests/minute
- Public endpoints: 100 requests/minute

**Plugins enabled**:
- rate-limiting
- jwt
- cors
- request-transformer
- response-transformer
- ip-restriction

### 3.2 Backend Services (FastAPI)

**Service: Auth Service**
- JWT token generation (access: 15 min, refresh: 7 days)
- OAuth 2.0 authorization code flow
- MFA support (TOTP, WebAuthn)
- Role-based access control (RBAC)

**Service: Device Manager**
- Device registration and lifecycle management
- OTA update distribution
- Health monitoring (heartbeat every 60s)
- Configuration management

**Service: Task Scheduler**
- Scheduled test execution (Celery + Redis)
- Test orchestration (parallel execution across devices)
- Result aggregation and reporting

**Service: Analytics Engine**
- Real-time stream processing (Kafka consumer)
- ML model inference (XGBoost, LSTM)
- Anomaly detection (Isolation Forest)
- Digital twin simulation

**Service: Federated Learning Orchestrator**
- Model versioning (MLflow)
- Aggregation coordination
- Privacy budget tracking
- Model evaluation and rollback

### 3.3 Database Schema

**PostgreSQL + TimescaleDB**:

```sql
-- Sessions table (hypertable for time-series)
CREATE TABLE sessions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    device_id VARCHAR(64) NOT NULL,
    user_id UUID NOT NULL,
    started_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    ended_at TIMESTAMPTZ,
    status VARCHAR(20) NOT NULL, -- 'running', 'completed', 'failed'
    qoe_avg FLOAT,
    qoe_min FLOAT,
    qoe_max FLOAT,
    frames_processed BIGINT,
    detections_count BIGINT
);

-- QoE metrics (hypertable, partitioned by time)
SELECT create_hypertable('qoe_metrics', 'timestamp');
CREATE TABLE qoe_metrics (
    id BIGSERIAL,
    session_id UUID NOT NULL REFERENCES sessions(id),
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    qoe_score FLOAT,
    psnr FLOAT,
    ssim FLOAT,
    vmaf FLOAT,
    latency_ms FLOAT,
    jitter_ms FLOAT,
    packet_loss_pct FLOAT,
    bitrate_kbps FLOAT,
    resolution_width INT,
    resolution_height INT,
    frame_rate FLOAT
);

-- Devices table
CREATE TABLE devices (
    id VARCHAR(64) PRIMARY KEY,
    user_id UUID NOT NULL,
    model VARCHAR(128),
    os_version VARCHAR(64),
    sdk_version VARCHAR(32),
    last_heartbeat TIMESTAMPTZ,
    status VARCHAR(20), -- 'online', 'offline', 'error'
    firmware_version VARCHAR(32),
    config JSONB
);

-- Detections table
CREATE TABLE detections (
    id BIGSERIAL,
    session_id UUID NOT NULL REFERENCES sessions(id),
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    track_id INT,
    class_name VARCHAR(64),
    confidence FLOAT,
    bbox_x FLOAT,
    bbox_y FLOAT,
    bbox_width FLOAT,
    bbox_height FLOAT,
    frame_number BIGINT
);

-- Sensor readings table (hypertable)
SELECT create_hypertable('sensor_readings', 'timestamp');
CREATE TABLE sensor_readings (
    id BIGSERIAL,
    session_id UUID NOT NULL REFERENCES sessions(id),
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    sensor_type VARCHAR(32),
    value1 FLOAT,
    value2 FLOAT,
    value3 FLOAT,
    accuracy FLOAT
);

-- Federated learning updates
CREATE TABLE fl_updates (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    device_id VARCHAR(64) NOT NULL,
    model_version VARCHAR(32) NOT NULL,
    uploaded_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    gradient_hash VARCHAR(64),
    privacy_epsilon FLOAT,
    samples_count INT,
    status VARCHAR(20) -- 'pending', 'processing', 'aggregated', 'rejected'
);
```

### 3.4 Kafka Topics

| Topic | Partitions | Retention | Description |
|-------|-----------|-----------|-------------|
| qoe-events | 12 | 7 days | Real-time QoE metrics from devices |
| detection-events | 12 | 3 days | Object detection results |
| sensor-events | 12 | 7 days | Sensor readings |
| fl-updates | 6 | 30 days | Federated learning model updates |
| device-heartbeats | 6 | 1 day | Device health status |
| alerts | 6 | 30 days | Alert events |
| dashboard-push | 6 | 1 hour | Real-time dashboard events |

### 3.5 Docker Compose (Local Development)

```yaml
version: '3.8'
services:
  postgres:
    image: timescale/timescaledb:latest-pg16
    environment:
      POSTGRES_DB: qelera
      POSTGRES_USER: qelera
      POSTGRES_PASSWORD: [REDACTED]
    ports:
      - "5432:5432"
    volumes:
      - postgres_data:/var/lib/postgresql/data

  redis:
    image: redis:7.2-alpine
    ports:
      - "6379:6379"
    command: redis-server --appendonly yes

  kafka:
    image: confluentinc/cp-kafka:7.6.0
    depends_on:
      - zookeeper
    environment:
      KAFKA_BROKER_ID: 1
      KAFKA_ZOOKEEPER_CONNECT: zookeeper:2181
      KAFKA_ADVERTISED_LISTENERS: PLAINTEXT://kafka:9092
      KAFKA_OFFSETS_TOPIC_REPLICATION_FACTOR: 1
    ports:
      - "9092:9092"

  zookeeper:
    image: confluentinc/cp-zookeeper:7.6.0
    environment:
      ZOOKEEPER_CLIENT_PORT: 2181
    ports:
      - "2181:2181"

  elasticsearch:
    image: elasticsearch:8.11.0
    environment:
      discovery.type: single-node
      ES_JAVA_OPTS: -Xms512m -Xmx512m
    ports:
      - "9200:9200"

  kong:
    image: kong:3.5
    depends_on:
      - postgres
    environment:
      KONG_DATABASE: "off"
      KONG_PROXY_ACCESS_LOG: /dev/stdout
      KONG_ADMIN_ACCESS_LOG: /dev/stdout
      KONG_PROXY_ERROR_LOG: /dev/stderr
      KONG_ADMIN_ERROR_LOG: /dev/stderr
      KONG_PROXY_LISTEN: "0.0.0.0:8000"
      KONG_ADMIN_LISTEN: "0.0.0.0:8001"
    ports:
      - "8000:8000"
      - "8001:8001"

  backend:
    build: ./backend
    depends_on:
      - postgres
      - redis
      - kafka
    environment:
      DATABASE_URL: postgresql://qelera:[REDACTED]@postgres:5432/qelera
      REDIS_URL: redis://redis:6379
      KAFKA_BOOTSTRAP_SERVERS: kafka:9092
      BIZSIMAI_JWT_SECRET: [REDACTED]
    ports:
      - "8000:8000"

  prometheus:
    image: prom/prometheus:latest
    volumes:
      - ./monitoring/prometheus.yml:/etc/prometheus/prometheus.yml
    ports:
      - "9090:9090"

  grafana:
    image: grafana/grafana:latest
    depends_on:
      - prometheus
    environment:
      GF_SECURITY_ADMIN_PASSWORD: [REDACTED]
    ports:
      - "3000:3000"
    volumes:
      - grafana_data:/var/lib/grafana

volumes:
  postgres_data:
  grafana_data:
```

---

## 4. SECURITY SPECIFICATIONS

### 4.1 Authentication Flow

1. Client sends credentials (email/password) to Auth Service
2. Auth Service validates against database
3. Auth Service issues JWT (access token: 15 min, refresh token: 7 days)
4. Client stores tokens in Android Keystore / iOS Keychain
5. All API requests include Bearer token in Authorization header
6. API Gateway validates JWT signature and expiration
7. Refresh token used to obtain new access token without re-auth

### 4.2 Encryption

**In transit**:
- TLS 1.3 for all HTTP/WebSocket connections
- mTLS for device-to-cloud communication (client certificates)
- Certificate pinning on Android SDK

**At rest**:
- AES-256-GCM for database encryption
- AES-256-GCM for object storage (S3 SSE-S3)
- SQLCipher for local SQLite database on device
- Android Keystore for cryptographic key storage

### 4.3 Access Control

**Roles**:
- `admin`: Full access to all resources
- `manager`: Access to assigned devices and sessions
- `viewer`: Read-only access to reports and dashboards
- `device`: Device-level access (limited API)

**Permissions matrix**:
| Action | admin | manager | viewer | device |
|--------|-------|---------|--------|--------|
| Create session | ✅ | ✅ | ❌ | ✅ |
| View session | ✅ | ✅ | ✅ | ✅ |
| Delete session | ✅ | ❌ | ❌ | ❌ |
| Manage devices | ✅ | ✅ | ❌ | ❌ |
| View reports | ✅ | ✅ | ✅ | ❌ |
| Generate reports | ✅ | ✅ | ❌ | ❌ |
| OTA updates | ✅ | ✅ | ❌ | ❌ |

---

## 5. API SPECIFICATIONS

### 5.1 REST API

**Request/Response format**: JSON
**Content-Type**: application/json
**Authentication**: Bearer token in Authorization header

**Error response format**:
```json
{
    "error": {
        "code": "VALIDATION_ERROR",
        "message": "Invalid session ID format",
        "details": {
            "field": "session_id",
            "constraint": "UUID format required"
        }
    }
}
```

### 5.2 WebSocket API

**Connection**: `wss://api.qelera.com/ws/realtime`
**Authentication**: Include JWT in URL query parameter: `?token=<jwt>`

**Client messages**:
```json
{
    "type": "subscribe",
    "session_id": "abc-123",
    "metrics": ["qoe_score", "psnr", "ssim"]
}
```

**Server messages**:
```json
{
    "type": "qoe_update",
    "session_id": "abc-123",
    "timestamp": 1716892800,
    "metrics": {
        "qoe_score": 4.2,
        "psnr": 35.5,
        "ssim": 0.92
    }
}
```

---

**END OF TECHNICAL SPECIFICATIONS**
