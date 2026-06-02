# Qelera

Distributed real-time vision and QoE monitoring platform for video streaming quality assessment across Android devices, ESP32 sensors, and Raspberry Pi gateways.

## Architecture Overview

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Android SDK    │    │  ESP32 Sensors  │    │  Raspberry Pi   │
│  (Qelera   │    │  (BLE/GPIO      │    │  Gateway        │
│   Engine)       │    │   Sensors)      │    │  (MQTT Broker)  │
└────────┬────────┘    └────────┬────────┘    └────────┬────────┘
         │                      │                      │
         │   HTTPS/TLS          │   BLE                │   MQTT
         ▼                      ▼                      ▼
┌─────────────────────────────────────────────────────────────────┐
│                        Kong Gateway                             │
│                 (API Gateway + Auth + Rate Limit)               │
└──────────────────────────┬──────────────────────────────────────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                   │                │
         ▼                   ▼                ▼
┌────────────────┐  ┌────────────────┐  ┌────────────────┐
│  FastAPI       │  │  Kafka         │  │  Redis         │
│  Backend       │  │  (Event Bus)   │  │  (Cache/Queue) │
└────────┬───────┘  └────────┬───────┘  └────────┬───────┘
         │                    │                    │
         ▼                    ▼                    ▼
┌────────────────┐  ┌────────────────┐  ┌────────────────┐
│  TimescaleDB   │  │  ML Pipeline   │  │  Kibana/ELK    │
│  (PostgreSQL)  │  │  (YOLOv8→TF)   │  │  (Logging)     │
└────────────────┘  └────────────────┘  └────────────────┘
         ▲
         │
┌────────────────┐
│  HashiCorp     │
│  Vault         │
│  (Secrets)     │
└────────────────┘
```

## Tech Stack

| Component | Technology |
|-----------|-----------|
| API Gateway | Kong Gateway 3.6+ |
| Backend | FastAPI + Python 3.12 |
| Database | TimescaleDB (PostgreSQL 16) |
| Cache/Queue | Redis 7.2 |
| Event Bus | Apache Kafka 3.6 |
| Logging | ELK Stack (Elasticsearch 8.x, Logstash, Kibana) |
| Secrets | HashiCorp Vault 1.15+ |
| Mobile SDK | Android (Kotlin) |
| Edge ML | YOLOv8 → TensorFlow Lite |
| IoT Firmware | ESP-IDF (ESP32), Raspberry Pi OS |
| Frontend | React 18 + TypeScript + Vite |
| Container | Docker Compose (local), Kubernetes (prod) |

## Project Structure

```
qelera/
├── docker-compose.yml              # Full orchestration
├── kong/
│   ├── kong.yml                    # Gateway config
│   └── plugins/                    # Custom plugins
├── backend/
│   ├── app/
│   │   ├── main.py                 # FastAPI app
│   │   ├── api/                    # Route handlers
│   │   ├── models/                 # SQLAlchemy models
│   │   ├── schemas/                # Pydantic schemas
│   │   ├── services/               # Business logic
│   │   ├── auth/                   # JWT auth
│   │   └── config.py               # Settings
│   ├── tests/
│   ├── requirements.txt
│   └── Dockerfile
├── db/
│   ├── migrations/                 # SQL migrations
│   └── seed.sql                    # Seed data
├── android-sdk/
│   ├── VisionTrackCore/
│   ├── VisionTrackEngine/
│   └── VisionTrackSensors/
├── ml-pipeline/
│   ├── yolo_training/
│   ├── conversion/
│   └── edge_detection/
├── firmware/
│   ├── esp32/                      # ESP32 firmware (ESP-IDF)
│   └── raspberry-pi/               # Pi gateway
├── frontend/
│   ├── src/
│   ├── public/
│   └── Dockerfile
├── vault/
│   ├── policies/
│   └── init.sh
└── docs/
    ├── architecture.md
    └── api-reference.md
```

## Quick Start

```bash
# Start all services
docker compose up -d

# Run migrations
docker compose exec backend alembic upgrade head

# Run tests
docker compose exec backend pytest

# Access dashboard
open http://localhost:3000

# Access Kibana
open http://localhost:5601

# Access Vault
open http://localhost:8200
```

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/api/v1/auth/register` | Register new user |
| POST | `/api/v1/auth/login` | Login (JWT) |
| POST | `/api/v1/auth/refresh` | Refresh token |
| GET | `/api/v1/sessions` | List sessions |
| POST | `/api/v1/sessions` | Create session |
| GET | `/api/v1/sessions/{id}` | Get session details |
| POST | `/api/v1/sessions/{id}/start` | Start recording |
| POST | `/api/v1/sessions/{id}/stop` | Stop recording |
| GET | `/api/v1/alerts` | List alerts |
| POST | `/api/v1/alerts` | Create alert |
| GET | `/api/v1/devices` | List devices |
| POST | `/api/v1/devices/register` | Register device |
| POST | `/api/v1/qoe/report` | Submit QoE metrics |
| POST | `/api/v1/detections` | Submit detections |

## Phases

### Phase 1: Foundation (Current)
- [x] DB Schema
- [ ] Kong Gateway
- [ ] ELK Stack
- [ ] Vault
- [ ] Backend API
- [ ] Android SDK
- [ ] ML Pipeline
- [ ] ESP32 Firmware
- [ ] Raspberry Pi Firmware
- [ ] React Dashboard
- [ ] Docker Compose
- [ ] Test Suite

### Phase 2: Production Hardening
- [ ] Kubernetes manifests
- [ ] CI/CD pipelines
- [ ] Monitoring & alerting
- [ ] Load testing
- [ ] Security audit

### Phase 3: Scale & Optimize
- [ ] Federated learning
- [ ] Multi-tenant support
- [ ] Edge caching
- [ ] Performance tuning

## License

MIT
