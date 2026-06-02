# Qelera

### Universal Social Media QoE Benchmarking & UX Platform

> **Measure the Experience, Not Just the Network.**

Qelera is a next-generation Quality of Experience (QoE) monitoring platform designed to isolate and measure the true performance of social media applications, independent of network conditions. By decoupling app-level metrics from network noise (WiFi, 4G, 5G), Qelera provides actionable insights for social platforms, device OEMs, and network operators.

---

## 🚀 Key Features

*   **App-Agnostic QoE:** Measures performance across any social media app (Meta, Snap, TikTok, X, etc.) without requiring deep integration with each platform.
*   **Network-Agnostic:** Isolates app performance from network latency, jitter, and packet loss to reveal the true user experience.
*   **Real-Time ML Scoring:** Uses YOLOv8-based ML pipelines to analyze frame drops, UI freezes, and rendering delays in real-time.
*   **Cross-Platform:** Comprehensive Android SDK for instrumentation, paired with a robust FastAPI backend and React dashboard.

## 🛠️ Tech Stack

| Component | Technology |
| :--- | :--- |
| **Mobile SDK** | Kotlin (Android), YOLOv8 (ML Inference) |
| **Backend** | FastAPI, Python, TimescaleDB |
| **Frontend** | React, TypeScript, D3.js (Data Viz) |
| **Infrastructure** | Docker, Kong (API Gateway), Vault |
| **Monitoring** | ELK Stack (Elasticsearch, Logstash, Kibana) |

## 🏗️ Architecture

```
[Android SDK] → (Instrumentation) → [FastAPI Backend] → [TimescaleDB]
       ↓                              ↓                       ↓
[ML Pipeline] ← (QoE Scoring) ← [Event Stream] ← [React Dashboard]
```

1.  **Android SDK:** Captures sensor data, network metrics, and UI performance events.
2.  **ML Pipeline:** Processes data using YOLOv8 to detect rendering anomalies and frame drops.
3.  **Backend:** Aggregates data, scores QoE (0-100), and provides RESTful APIs.
4.  **Dashboard:** Visualizes real-time QoE trends and historical benchmarks.

## 🚀 Getting Started

### Prerequisites
*   Python 3.10+
*   Android Studio (for SDK development)
*   Docker & Docker Compose

### 1. Clone the Repository
```bash
git clone https://github.com/luiborge23/qelera.git
cd qelera
```

### 2. Setup Backend
```bash
cd software/backend
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
uvicorn main:app --reload
```

### 3. Setup Android SDK
Open the `software/android/sdk` module in Android Studio and sync Gradle files.

### 4. Run Infrastructure
```bash
docker-compose up -d
```

## 📅 Roadmap

*   [x] **Project Rebranding:** Migrate from VisionTrack to Qelera.
*   [ ] **Backend Core:** Implement FastAPI endpoints and TimescaleDB schema.
*   [ ] **Android SDK:** Finalize instrumentation and ML inference integration.
*   [ ] **ML Pipeline:** Train and export YOLOv8 QoE scoring models.
*   [ ] **Dashboard:** Build React frontend for real-time visualization.

## 📄 License

This project is proprietary. Unauthorized use, distribution, or modification is prohibited.

## 📞 Contact

For inquiries, partnerships, or support:
*   **Website:** [qelera.com](https://qelera.com)
*   **Email:** contact@qelera.com

---

*Qelera — Measuring the Experience.*
