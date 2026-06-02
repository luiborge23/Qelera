# REST API Specification — Qelera

## 1. API Overview

Qelera exposes a RESTful API for device management, test execution, and metric retrieval. Designed for developer integration with CI/CD pipelines.

**Base URL:** `https://api.qelera.io/v1`
**Auth:** Bearer token (API key per workspace)

## 2. Authentication

```bash
# Login (API key issued from dashboard)
curl -X POST https://api.qelera.io/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"api_key": "vt_live_xxxxx"}'

# Response
{
  "access_token": "eyJhbGciOiJIUzI1NiIs...",
  "expires_in": 3600,
  "workspace_id": "ws_abc123"
}
```

## 3. Endpoints

### 3.1 Devices

```
GET    /devices                    # List all devices
POST   /devices                    # Register new device
GET    /devices/:id                # Get device details
PUT    /devices/:id                # Update device config
DELETE /devices/:id                # Remove device
POST   /devices/:id/control        # Send remote control command
GET    /devices/:id/stream         # Get live stream URL (WebSocket)
```

**Device Object:**
```json
{
  "id": "dev_android_001",
  "type": "android",
  "model": "Pixel 8",
  "os_version": "14",
  "status": "online",
  "capture_method": "hdmi",
  "capture_card": "AVerMedia GC573",
  "fps": 120,
  "resolution": "1080p",
  "last_seen": "2026-05-27T10:30:00Z",
  "battery": 85,
  "network": "wifi",
  "app_version": "3.2.1"
}
```

### 3.2 Tests

```
POST   /tests                      # Create test run
GET    /tests                      # List test runs
GET    /tests/:id                  # Get test details
POST   /tests/:id/start            # Start test execution
POST   /tests/:id/stop             # Stop test execution
GET    /tests/:id/results          # Get test results
```

**Test Object:**
```json
{
  "id": "test_xyz789",
  "workspace_id": "ws_abc123",
  "device_id": "dev_android_001",
  "app_package": "com.example.app",
  "app_version": "3.2.1",
  "status": "running",
  "metrics": {
    "cold_start_ms": 450,
    "ald_ms": 380,
    "fvild_ms": 120,
    "rebuffer_count": 2,
    "spinner_count": 5
  },
  "created_at": "2026-05-27T10:00:00Z",
  "completed_at": null
}
```

### 3.3 Metrics (Real-time)

```
GET    /metrics/live               # WebSocket stream of live metrics
GET    /metrics/historical         # Query historical metrics
POST   /metrics/alerts             # Configure alert thresholds
GET    /metrics/alerts             # List active alerts
```

**Live Metric Stream (WebSocket):**
```json
{
  "event": "metric_update",
  "device_id": "dev_android_001",
  "timestamp": "2026-05-27T10:30:05.123Z",
  "metrics": {
    "frame_timestamp_ms": 1234567,
    "flow_entropy": 3.42,
    "motion_ratio": 0.12,
    "state": "video_playing",
    "confidence": 0.92
  }
}
```

### 3.4 Analytics

```
GET    /analytics/benchmarks       # App version comparison
GET    /analytics/trends           # Metric trends over time
GET    /analytics/export           # Export to CSV/JSON
POST   /analytics/reports          # Generate custom report
```

## 4. Webhooks

```json
{
  "event": "test.completed",
  "timestamp": "2026-05-27T10:35:00Z",
  "data": {
    "test_id": "test_xyz789",
    "device_id": "dev_android_001",
    "result": "passed",
    "metrics": {
      "cold_start_ms": 450,
      "ald_ms": 380,
      "fvild_ms": 120
    },
    "alerts": [
      {
        "type": "cold_start_slow",
        "threshold_ms": 400,
        "actual_ms": 450,
        "severity": "warning"
      }
    ]
  }
}
```

## 5. Rate Limits

| Tier | Requests/min | Concurrent Tests | Storage |
|------|-------------|------------------|---------|
| Free | 60 | 1 | 100MB |
| Startup | 300 | 4 | 1GB |
| Pro | 1,000 | 10 | 10GB |
| Team | 5,000 | 50 | 100GB |
| Enterprise | Custom | Unlimited | Unlimited |

## 6. SDKs

- **Python:** `pip install qelera`
- **Node.js:** `npm install @qelera/sdk`
- **CLI:** `npm install -g @qelera/cli`

**Python SDK Example:**
```python
from qelera import Client

vt = Client(api_key="vt_live_xxxxx")

# Run a test
test = vt.tests.create(
    device_id="dev_android_001",
    app_package="com.example.app",
    metrics=["cold_start", "ald", "fvild", "rebuf"]
)

# Wait for completion
result = test.wait(timeout=300)
print(f"Cold start: {result.metrics.cold_start_ms}ms")
print(f"ALD: {result.metrics.ald_ms}ms")
print(f"FVILD: {result.metrics.fvild_ms}ms")
```
