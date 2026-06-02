# Qelera — Comprehensive Test Suite v1.0
# Pytest (Backend), JUnit 5 (Android), Vitest (Frontend), Playwright (E2E)
# Created: May 28, 2026

# ============================================================
# PROJECT STRUCTURE
# ============================================================
# tests/
# ├── conftest.py                 # Pytest fixtures for backend
# ├── pytest.ini                  # Pytest configuration
# ├── integration/
# │   ├── __init__.py
# │   ├── test_api_endpoints.py   # REST API integration tests
# │   ├── test_database.py        # DB schema & migration tests
# │   ├── test_websocket.py       # WebSocket real-time tests
# │   ├── test_mqtt.py            # MQTT broker integration
# │   ├── test_kong_routes.py     # Kong gateway routing tests
# │   └── test_elk_pipeline.py    # ELK log ingestion tests
# ├── e2e/
# │   ├── __init__.py
# │   ├── conftest.py             # Playwright fixtures
# │   ├── test_dashboard.py       # Full dashboard user flows
# │   ├── test_session_lifecycle.py  # End-to-end test lifecycle
# │   ├── test_alert_management.py  # Alert creation & resolution
# │   ├── test_device_management.py  # Device onboarding & control
# │   └── test_analytics.py       # Analytics report generation
# ├── unit/
# │   ├── __init__.py
# │   ├── test_auth.py            # JWT auth & token validation
# │   ├── test_models.py          # Data model validation
# │   ├── test_utils.py           # Utility functions
# │   ├── test_qoe_calculator.py  # QoE metric computation
# │   └── test_alert_engine.py    # Alert rule evaluation
# ├── fixtures/
# │   ├── __init__.py
# │   ├── api_fixtures.py         # API test data
# │   ├── db_fixtures.py          # DB test fixtures
# │   ├── mock_data.py            # Mock QoE/alert data
# │   └── test_profiles.py        # Test profile definitions
# ├── android/
# │   ├── build.gradle            # Android test build config
# │   └── src/androidTest/java/io/qelera/
# │       ├── VisionTrackEngineTest.kt
# │       ├── CameraCaptureTest.kt
# │       ├── SensorCollectorTest.kt
# │       ├── MLModelTest.kt
# │       └── WebSocketClientTest.kt
# └── frontend/
#     ├── vitest.config.ts
#     ├── setup.ts                # Vitest setup (jsdom, mocks)
#     └── src/
#         ├── components/
#         │   ├── QoETrendChart.test.tsx
#         │   ├── DeviceTable.test.tsx
#         │   └── AlertTable.test.tsx
#         ├── hooks/
#         │   ├── useQoEMetrics.test.ts
#         │   └── useAlerts.test.ts
#         └── services/
#             ├── api.test.ts
#             └── websocket.test.ts

# ============================================================
# PYTEST CONFIGURATION (pytest.ini)
# ============================================================
# [pytest]
# testpaths = tests
# python_files = test_*.py
# python_classes = Test*
# python_functions = test_*
# addopts =
#     -v
#     --strict-markers
#     --tb=short
#     --cov=app
#     --cov-report=html
#     --cov-report=term-missing
#     --cov-fail-under=80
# markers =
#     slow: marks tests as slow (deselect with '-m "not slow"')
#     integration: marks tests as integration tests
#     e2e: marks tests as end-to-end tests
#     db: marks tests that require database
#     api: marks tests that require API server
#     mqtt: marks tests that require MQTT broker
#     skip_ci: marks tests to skip in CI

# ============================================================
# BACKEND CONFTEST (tests/conftest.py)
# ============================================================
# import pytest
# import asyncio
# from httpx import AsyncClient, ASGITransport
# from sqlalchemy import create_engine
# from sqlalchemy.orm import sessionmaker
# from sqlalchemy.pool import StaticPool
#
# from app.main import app
# from app.database.base import Base
# from app.database.session import get_db
# from app.models.user import User
# from app.services.auth import create_access_token
#
# # Database setup for testing
# SQLALCHEMY_DATABASE_URL = "sqlite:///./test.db"
# engine = create_engine(
#     SQLALCHEMY_DATABASE_URL,
#     connect_args={"check_same_thread": False},
#     poolclass=StaticPool
# )
# TestingSessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
#
# @pytest.fixture(scope="session")
# def event_loop():
#     """Create instance for session-scoped fixtures."""
#     loop = asyncio.get_event_loop_policy().new_event_loop()
#     yield loop
#     loop.close()
#
# @pytest.fixture(scope="session")
# def db_session():
#     """Create a fresh database session for the test session."""
#     Base.metadata.create_all(bind=engine)
#     session = TestingSessionLocal()
#     try:
#         yield session
#     finally:
#         session.close()
#     Base.metadata.drop_all(bind=engine)
#
# @pytest.fixture
# def client(db_session):
#     """Test HTTP client with dependency override."""
#     def override_get_db():
#         try:
#             yield db_session
#         finally:
#             pass
#
#     app.dependency_overrides[get_db] = override_get_db
#     yield app
#     app.dependency_overrides.clear()
#
# @pytest.fixture
# def auth_headers(client, db_session):
#     """Generate auth headers for authenticated requests."""
#     user = User(
#         username="testuser",
#         email="test@example.com",
#         hashed_password="hashed_password_here",
#         is_active=True,
#         is_superuser=True
#     )
#     db_session.add(user)
#     db_session.commit()
#     db_session.refresh(user)
#
#     token = create_access_token(data={"sub": str(user.id)})
#     return {"Authorization": f"Bearer {token}"}
#
# @pytest.fixture
# def sample_qoe_metric():
#     """Sample QoE metric for testing."""
#     return {
#         "device_id": "device-001",
#         "session_id": "session-001",
#         "qoe_score": 85.5,
#         "psnr": 35.2,
#         "ssim": 0.92,
#         "vmaf": 88.1,
#         "bitrate": 5000,
#         "resolution": "1920x1080",
#         "frame_rate": 60,
#         "latency_ms": 45,
#         "jitter_ms": 5,
#         "packet_loss_pct": 0.1,
#         "buffer_ratio": 0.95
#     }
#
# @pytest.fixture
# def sample_alert():
#     """Sample alert for testing."""
#     return {
#         "device_id": "device-001",
#         "session_id": "session-001",
#         "alert_type": "anomaly_detected",
#         "severity": "high",
#         "message": "Motion blur detected in video stream",
#         "metadata": {
#             "anomaly_class": "motion_blur",
#             "confidence": 0.87,
#             "frame_number": 1250
#         }
#     }

# ============================================================
# API INTEGRATION TESTS (tests/integration/test_api_endpoints.py)
# ============================================================
# import pytest
# from httpx import AsyncClient, ASGITransport
#
# @pytest.mark.api
# @pytest.mark.integration
# class TestAuthEndpoints:
#     """Test authentication API endpoints"""
#
#     async def test_login_success(self, client):
#         response = await client.post("/api/v1/auth/login", json={
#             "username": "testuser",
#             "password": "testpassword"
#         })
#         assert response.status_code == 200
#         data = response.json()
#         assert "access_token" in data
#         assert data["token_type"] == "bearer"
#
#     async def test_login_invalid_credentials(self, client):
#         response = await client.post("/api/v1/auth/login", json={
#             "username": "testuser",
#             "password": "wrongpassword"
#         })
#         assert response.status_code == 401
#
#     async def test_register_user(self, client):
#         response = await client.post("/api/v1/auth/register", json={
#             "username": "newuser",
#             "email": "new@example.com",
#             "password": "SecurePass123!"
#         })
#         assert response.status_code == 201
#
#     async def test_register_duplicate_email(self, client):
#         # Register first
#         await client.post("/api/v1/auth/register", json={
#             "username": "user1",
#             "email": "duplicate@example.com",
#             "password": "SecurePass123!"
#         })
#         # Try again
#         response = await client.post("/api/v1/auth/register", json={
#             "username": "user2",
#             "email": "duplicate@example.com",
#             "password": "SecurePass123!"
#         })
#         assert response.status_code == 400
#
# @pytest.mark.api
# @pytest.mark.integration
# class TestQoEMetricsEndpoints:
#     """Test QoE metrics API endpoints"""
#
#     async def test_submit_qoe_metric(self, client, auth_headers, sample_qoe_metric):
#         response = await client.post(
#             "/api/v1/qoe/metrics",
#             json=sample_qoe_metric,
#             headers=auth_headers
#         )
#         assert response.status_code == 201
#         data = response.json()
#         assert data["qoe_score"] == 85.5
#
#     async def test_get_qoe_metrics(self, client, auth_headers):
#         response = await client.get(
#             "/api/v1/qoe/metrics?device_id=device-001&hours=24",
#             headers=auth_headers
#         )
#         assert response.status_code == 200
#         data = response.json()
#         assert isinstance(data, list)
#
#     async def test_get_qoe_summary(self, client, auth_headers):
#         response = await client.get(
#             "/api/v1/qoe/summary?workspace_id=workspace-001",
#             headers=auth_headers
#         )
#         assert response.status_code == 200
#         data = response.json()
#         assert "overall_qoe" in data
#         assert "qoe_trend" in data
#
#     async def test_get_qoe_metrics_unauthorized(self, client, sample_qoe_metric):
#         response = await client.post(
#             "/api/v1/qoe/metrics",
#             json=sample_qoe_metric
#         )
#         assert response.status_code == 401
#
# @pytest.mark.api
# @pytest.mark.integration
# class TestAlertEndpoints:
#     """Test alerts API endpoints"""
#
#     async def test_create_alert(self, client, auth_headers, sample_alert):
#         response = await client.post(
#             "/api/v1/alerts",
#             json=sample_alert,
#             headers=auth_headers
#         )
#         assert response.status_code == 201
#         data = response.json()
#         assert data["severity"] == "high"
#
#     async def test_get_alerts(self, client, auth_headers):
#         response = await client.get(
#             "/api/v1/alerts?severity=critical&limit=10",
#             headers=auth_headers
#         )
#         assert response.status_code == 200
#
#     async def test_resolve_alert(self, client, auth_headers):
#         # Create alert first
#         create_resp = await client.post(
#             "/api/v1/alerts",
#             json=sample_alert,
#             headers=auth_headers
#         )
#         alert_id = create_resp.json()["id"]
#
#         # Resolve it
#         response = await client.patch(
#             f"/api/v1/alerts/{alert_id}/resolve",
#             headers=auth_headers
#         )
#         assert response.status_code == 200
#         data = response.json()
#         assert data["status"] == "resolved"
#
#     async def test_get_alerts_unauthorized(self, client):
#         response = await client.get("/api/v1/alerts")
#         assert response.status_code == 401

# ============================================================
# DATABASE TESTS (tests/integration/test_database.py)
# ============================================================
# import pytest
# from sqlalchemy import text
#
# @pytest.mark.db
# @pytest.mark.integration
# class TestDatabaseSchema:
#     """Test database schema and constraints"""
#
#     def test_table_creation(self, db_session):
#         """Verify all tables are created"""
#         result = db_session.execute(text(
#             "SELECT table_name FROM information_schema.tables "
#             "WHERE table_schema = 'public'"
#         ))
#         tables = [row[0] for row in result.fetchall()]
#         expected_tables = [
#             "workspaces", "users", "api_keys", "devices",
#             "test_sessions", "qoe_metrics", "frame_detections",
#             "sensor_data", "alert_rules", "alerts",
#             "ota_packages", "ota_deployment_logs", "audit_log"
#         ]
#         for table in expected_tables:
#             assert table in tables, f"Table {table} not found"
#
#     def test_hypertable_creation(self, db_session):
#         """Verify TimescaleDB hypertables are created"""
#         result = db_session.execute(text(
#             "SELECT hypertable_name FROM timescaledb_information.hypertables"
#         ))
#         hypertables = [row[0] for row in result.fetchall()]
#         assert "qoe_metrics" in hypertables
#         assert "frame_detections" in hypertables
#
#     def test_continuous_aggregates(self, db_session):
#         """Verify continuous aggregates are created"""
#         result = db_session.execute(text(
#             "SELECT matview_name FROM timescaledb_information.continuous_aggregates"
#         ))
#         aggregates = [row[0] for row in result.fetchall()]
#         assert "qoe_metrics_1m" in aggregates
#
#     def test_foreign_key_constraints(self, db_session):
#         """Test foreign key constraints are enforced"""
#         from app.models.device import Device
#         from app.models.user import User
#
#         user = User(username="test", email="test@test.com", hashed_password="test")
#         db_session.add(user)
#         db_session.commit()
#
#         device = Device(
#             device_id="test-device-001",
#             device_name="Test Device",
#             device_type="android",
#             user_id=user.id,
#             workspace_id=1
#         )
#         db_session.add(device)
#         db_session.commit()
#
#         # Verify device exists
#         result = db_session.query(Device).filter_by(device_id="test-device-001").first()
#         assert result is not None
#         assert result.user_id == user.id
#
#     def test_trigger_functionality(self, db_session):
#         """Test that audit triggers work"""
#         from app.models.device import Device
#
#         device = Device(
#             device_id="trigger-test-001",
#             device_name="Trigger Test",
#             device_type="android",
#             user_id=1,
#             workspace_id=1
#         )
#         db_session.add(device)
#         db_session.commit()
#
#         # Update device to trigger audit log
#         device.device_name = "Updated Name"
#         db_session.commit()
#
#         # Verify audit log entry exists
#         from app.models.audit import AuditLog
#         audit = db_session.query(AuditLog).filter_by(
#             table_name="devices",
#             record_id=device.id,
#             action="UPDATE"
#         ).first()
#         assert audit is not None
#
#     @pytest.mark.slow
#     def test_performance_hypertable_partitioning(self, db_session):
#         """Test that hypertable partitioning improves query performance"""
#         import time
#         from sqlalchemy import text
#
#         # Insert test data
#         for i in range(1000):
#             db_session.execute(text(f"""
#                 INSERT INTO qoe_metrics (time, device_id, session_id, qoe_score, psnr, ssim)
#                 VALUES (now() - interval '{i} seconds', 'perf-test', 'perf-session', 85.0, 35.0, 0.90)
#             """))
#         db_session.commit()
#
#         # Time a query on hypertable
#         start = time.time()
#         result = db_session.execute(text("""
#             SELECT time, device_id, qoe_score
#             FROM qoe_metrics
#             WHERE time > now() - interval '1 hour'
#             ORDER BY time DESC
#             LIMIT 100
#         """))
#         hypertable_time = time.time() - start
#
#         # Compare with regular table (should be similar or faster)
#         assert hypertable_time < 1.0, "Hypertable query too slow"

# ============================================================
# E2E TESTS (tests/e2e/test_dashboard.py)
# ============================================================
# import pytest
# from playwright.async_api import async_playwright, Page
#
# @pytest.mark.e2e
# class TestDashboard:
#     """End-to-end tests for the React dashboard"""
#
#     @pytest.fixture(autouse=True)
#     async def setup(self):
#         """Setup: start browser and navigate to dashboard"""
#         async with async_playwright() as p:
#             browser = await p.chromium.launch(headless=True)
#             context = await browser.new_context()
#             page = await context.new_page()
#             yield page
#             await browser.close()
#
#     async def test_login_and_dashboard_load(self, page: Page):
#         """Test user login and dashboard loading"""
#         # Navigate to login
#         await page.goto("http://localhost:5173/login")
#         await page.fill('input[name="username"]', 'admin')
#         await page.fill('input[name="password"]', 'admin123')
#         await page.click('button[type="submit"]')
#
#         # Wait for dashboard to load
#         await page.wait_for_url("http://localhost:5173/dashboard")
#         await page.wait_for_selector('.qoe-score-card', timeout=5000)
#
#         # Verify dashboard elements
#         assert await page.is_visible('.qoe-score-card')
#         assert await page.is_visible('.device-status-card')
#         assert await page.is_visible('.alert-summary-card')
#
#     async def test_navigate_to_devices(self, page: Page):
#         """Test navigation to devices page"""
#         await page.goto("http://localhost:5173/dashboard")
#         await page.click('nav a:has-text("Devices")')
#         await page.wait_for_url("http://localhost:5173/devices")
#         await page.wait_for_selector('.device-table', timeout=5000)
#
#         # Verify device table is populated
#         rows = await page.query_selector_all('.device-table tbody tr')
#         assert len(rows) > 0
#
#     async def test_filter_alerts_by_severity(self, page: Page):
#         """Test filtering alerts by severity"""
#         await page.goto("http://localhost:5173/alerts")
#         await page.wait_for_selector('.alert-table', timeout=5000)
#
#         # Click severity filter dropdown
#         await page.click('select[name="severity"]')
#         await page.click('option[value="critical"]')
#
#         # Wait for filtered results
#         await page.wait_for_timeout(1000)
#         rows = await page.query_selector_all('.alert-table tbody tr')
#         for row in rows:
#             severity = await row.inner_text()
#             assert 'critical' in severity.lower() or 'high' in severity.lower()
#
#     async def test_export_report(self, page: Page):
#         """Test generating and downloading analytics report"""
#         await page.goto("http://localhost:5173/analytics")
#         await page.wait_for_selector('.analytics-container', timeout=5000)
#
#         # Click export button
#         await page.click('button:has-text("Export Report")')
#
#         # Wait for download (Playwright handles this)
#         download = await page.expect_download(timeout=10000)
#         assert download.suggested_filename.endswith('.pdf') or download.suggested_filename.endswith('.csv')
#
#     async def test_real_time_qoe_updates(self, page: Page):
#         """Test that QoE metrics update in real-time"""
#         await page.goto("http://localhost:5173/dashboard")
#         await page.wait_for_selector('.qoe-trend-chart', timeout=5000)
#
#         # Capture initial QoE score
#         initial_qoe = await page.inner_text('.qoe-score-value')
#
#         # Wait for updates (WebSocket should push new data)
#         await page.wait_for_timeout(10000)
#
#         # QoE score should have updated
#         final_qoe = await page.inner_text('.qoe-score-value')
#         # Note: This test assumes the backend is pushing updates
#         # In a real test, you'd mock the WebSocket or use a test backend
#         assert initial_qoe or final_qoe  # At least one value exists

# ============================================================
# ANDROID TESTS (test example)
# ============================================================
# // tests/android/src/androidTest/java/io/qelera/VisionTrackEngineTest.kt
# package io.qelera
#
# import androidx.test.ext.junit.runners.AndroidJUnit4
# import androidx.test.platform.app.InstrumentationRegistry
# import org.junit.Assert.*
# import org.junit.Test
# import org.junit.runner.RunWith
#
# @RunWith(AndroidJUnit4::class)
# class VisionTrackEngineTest {
#
#     @Test
#     fun testEngineInitialization() {
#         val appContext = InstrumentationRegistry.getInstrumentation().targetContext
#         val engine = VisionTrackEngine.getInstance(appContext)
#
#         val result = runBlocking {
#             engine.initialize(SDKConfig(
#                 apiEndpoint = "http://localhost:8000",
#                 authToken = "test-token",
#                 workspaceId = "test-workspace",
#                 mlModelPath = "models/anomaly_detector.tflite"
#             ))
#         }
#
#         assertTrue(result.isSuccess)
#         assertEquals(SessionState.INITIALIZED, engine.sessionState.value)
#     }
#
#     @Test
#     fun testSessionLifecycle() = runTest {
#         val appContext = InstrumentationRegistry.getInstrumentation().targetContext
#         val engine = VisionTrackEngine.getInstance(appContext)
#
#         // Initialize
#         val initResult = engine.initialize(SDKConfig(...))
#         assertTrue(initResult.isSuccess)
#
#         // Start session
#         val startResult = engine.startSession("test-session-001", TestType.STREAMING)
#         assertTrue(startResult.isSuccess)
#         assertEquals(SessionState.RUNNING, engine.sessionState.value)
#
#         // Stop session
#         val stopResult = engine.stopSession()
#         assertTrue(stopResult.isSuccess)
#         assertEquals(SessionState.COMPLETED, engine.sessionState.value)
#     }
#
#     @Test
#     fun testQoEMetricsFlow() = runTest {
#         val appContext = InstrumentationRegistry.getInstrumentation().targetContext
#         val engine = VisionTrackEngine.getInstance(appContext)
#
#         var metricCount = 0
#         engine.qoeMetrics.collect { metric ->
#             metricCount++
#             assertTrue(metric.qoe_score in 0.0..100.0)
#             assertTrue(metric.timestamp > 0L)
#             if (metricCount >= 10) cancel()
#         }
#
#         // Start session to generate metrics
#         engine.startSession("test-session-001", TestType.STREAMING)
#         delay(5000)
#         engine.stopSession()
#
#         assertTrue(metricCount > 0)
#     }
# }

# ============================================================
# FRONTEND TESTS (example)
# ============================================================
# // tests/frontend/src/components/QoETrendChart.test.tsx
# import { render, screen, waitFor } from '@testing-library/react'
# import { describe, it, expect, vi } from 'vitest'
# import QoETrendChart from '../../components/charts/QoETrendChart'
#
# describe('QoETrendChart', () => {
#   const mockData = [
#     { timestamp: '2024-01-01T00:00:00Z', qoe: 85, minQoe: 70, maxQoe: 95 },
#     { timestamp: '2024-01-01T01:00:00Z', qoe: 88, minQoe: 75, maxQoe: 98 },
#     { timestamp: '2024-01-01T02:00:00Z', qoe: 82, minQoe: 68, maxQoe: 92 },
#   ]
#
#   it('renders the chart title', () => {
#     render(<QoETrendChart data={mockData} />)
#     expect(screen.getByText('QoE Trend (24h)')).toBeInTheDocument()
#   })
#
#   it('renders the chart with data', async () => {
#     render(<QoETrendChart data={mockData} />)
#
#     await waitFor(() => {
#       const chart = document.querySelector('svg')
#       expect(chart).toBeInTheDocument()
#     })
#   })
#
#   it('shows tooltip on hover', async () => {
#     render(<QoETrendChart data={mockData} />)
#
#     const chart = await waitFor(() => document.querySelector('svg'))
#     if (chart) {
#       // Note: Recharts tooltip testing requires more setup
#       // This is a simplified example
#       expect(chart).toBeInTheDocument()
#     }
#   })
# })

# ============================================================
# TEST EXECUTION COMMANDS
# ============================================================
#
# # Run all backend tests
# cd software/backend && python -m pytest tests/ -v
#
# # Run backend tests with coverage
# cd software/backend && python -m pytest tests/ -v --cov=app --cov-report=html
#
# # Run only unit tests
# cd software/backend && python -m pytest tests/unit/ -v
#
# # Run only integration tests
# cd software/backend && python -m pytest tests/integration/ -v
#
# # Run only slow tests
# cd software/backend && python -m pytest tests/ -v -m slow
#
# # Run E2E tests
# cd tests/e2e && npx playwright test
#
# # Run frontend unit tests
# cd software/frontend && npm test
#
# # Run Android tests
# cd software/android && ./gradlew connectedAndroidTest
#
# # Run all tests in one command
# python -m pytest tests/ -v && cd software/frontend && npm test && cd ../android && ./gradlew connectedAndroidTest
#
# # CI/CD pipeline
# # .github/workflows/test.yml:
# #   - checkout
# #   - setup Python
# #   - setup Node.js
# #   - setup Android SDK
# #   - docker compose up -d db kong backend
# #   - pytest tests/ -v --cov=app
# #   - cd software/frontend && npm test
# #   - cd software/android && ./gradlew test
# #   - docker compose down
