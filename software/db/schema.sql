-- VisionTrack Database Schema v1.0
-- TimescaleDB (PostgreSQL 16) for time-series QoE metrics
-- Created: May 28, 2026

-- Enable extensions
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
CREATE EXTENSION IF NOT EXISTS "pgcrypto";
CREATE EXTENSION IF NOT EXISTS timescaledb;

-- ============================================================
-- WORKSPACES & TENANCY
-- ============================================================

CREATE TABLE workspaces (
    id              UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    name            VARCHAR(255) NOT NULL,
    slug            VARCHAR(100) UNIQUE NOT NULL,
    plan            VARCHAR(50) NOT NULL DEFAULT 'free' CHECK (plan IN ('free','pro','enterprise')),
    max_devices     INTEGER NOT NULL DEFAULT 10,
    data_retention_days INTEGER NOT NULL DEFAULT 30,
    settings        JSONB NOT NULL DEFAULT '{}',
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_workspaces_slug ON workspaces(slug);

-- ============================================================
-- USERS & AUTH
-- ============================================================

CREATE TABLE users (
    id              UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    workspace_id    UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    email           VARCHAR(255) UNIQUE NOT NULL,
    password_hash   VARCHAR(255) NOT NULL,
    display_name    VARCHAR(255) NOT NULL,
    role            VARCHAR(50) NOT NULL DEFAULT 'member' 
                    CHECK (role IN ('owner','admin','member','viewer')),
    is_active       BOOLEAN NOT NULL DEFAULT true,
    last_login      TIMESTAMPTZ,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_users_workspace ON users(workspace_id);
CREATE INDEX idx_users_email ON users(email);

-- API keys for programmatic access
CREATE TABLE api_keys (
    id              UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    workspace_id    UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    key_hash        VARCHAR(255) NOT NULL,
    key_prefix      VARCHAR(20) NOT NULL,  -- first 10 chars for display (vt_live_xxxxx)
    name            VARCHAR(255) NOT NULL,
    permissions     JSONB NOT NULL DEFAULT '["read","write"]',
    is_active       BOOLEAN NOT NULL DEFAULT true,
    last_used_at    TIMESTAMPTZ,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    created_by      UUID REFERENCES users(id)
);

CREATE INDEX idx_api_keys_workspace ON api_keys(workspace_id);
CREATE INDEX idx_api_keys_hash ON api_keys(key_hash);

-- ============================================================
-- DEVICES
-- ============================================================

CREATE TABLE device_types (
    id              SERIAL PRIMARY KEY,
    code            VARCHAR(50) UNIQUE NOT NULL,  -- android, ios, esp32, rpi
    name            VARCHAR(100) NOT NULL
);

INSERT INTO device_types (code, name) VALUES
    ('android', 'Android Device'),
    ('ios', 'iOS Device'),
    ('esp32', 'ESP32 Sensor'),
    ('rpi', 'Raspberry Pi Gateway');

CREATE TABLE devices (
    id                      UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    workspace_id            UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    device_type_id          INTEGER NOT NULL REFERENCES device_types(id),
    device_id               VARCHAR(100) NOT NULL,  -- Android: device_id, ESP32: MAC, Pi: hostname
    name                    VARCHAR(255),
    model                   VARCHAR(255),
    os_version              VARCHAR(100),
    sdk_version             VARCHAR(50),
    firmware_version        VARCHAR(50),
    status                  VARCHAR(50) NOT NULL DEFAULT 'offline'
                            CHECK (status IN ('online','offline','maintenance','error')),
    capture_method          VARCHAR(50),  -- hdmi, screen_capture, native
    capture_card            VARCHAR(100),
    fps                     INTEGER,
    resolution              VARCHAR(20),  -- 1080p, 4k
    battery_pct             INTEGER CHECK (battery_pct >= 0 AND battery_pct <= 100),
    network_type            VARCHAR(20),  -- wifi, cellular_5g, cellular_4g, ethernet
    wifi_ssid               VARCHAR(255),
    wifi_signal_dbm         INTEGER,
    ip_address              INET,
    mac_address             MACADDR,
    location                GEOGRAPHY(POINT, 4326),
    config                  JSONB NOT NULL DEFAULT '{}',
    last_seen               TIMESTAMPTZ,
    ota_available           VARCHAR(50),
    ota_installed           VARCHAR(50),
    created_at              TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at              TIMESTAMPTZ NOT NULL DEFAULT now(),
    UNIQUE(workspace_id, device_id)
);

CREATE INDEX idx_devices_workspace ON devices(workspace_id);
CREATE INDEX idx_devices_status ON devices(status);
CREATE INDEX idx_devices_last_seen ON devices(last_seen);

-- ============================================================
-- TEST SESSIONS
-- ============================================================

CREATE TABLE test_sessions (
    id                      UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    workspace_id            UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    device_id               UUID NOT NULL REFERENCES devices(id),
    name                    VARCHAR(255) NOT NULL,
    description             TEXT,
    status                  VARCHAR(50) NOT NULL DEFAULT 'idle'
                            CHECK (status IN ('idle','scheduled','running','paused','completed','failed','cancelled')),
    app_package             VARCHAR(255) NOT NULL,
    app_version             VARCHAR(50) NOT NULL,
    test_type               VARCHAR(50) NOT NULL
                            CHECK (test_type IN ('streaming','live','vod','gaming','video_call')),
    protocol                VARCHAR(20) NOT NULL DEFAULT 'hls'
                            CHECK (protocol IN ('hls','dash','rtmp','webrtc','rtsp')),
    stream_url              TEXT,
    target_bitrate_kbps     INTEGER,
    target_resolution       VARCHAR(20),
    target_fps              INTEGER,
    scheduled_at            TIMESTAMPTZ,
    started_at              TIMESTAMPTZ,
    completed_at            TIMESTAMPTZ,
    duration_seconds        INTEGER,
    results_summary         JSONB,
    tags                    TEXT[],
    created_by              UUID REFERENCES users(id),
    created_at              TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at              TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_sessions_workspace ON test_sessions(workspace_id);
CREATE INDEX idx_sessions_device ON test_sessions(device_id);
CREATE INDEX idx_sessions_status ON test_sessions(status);
CREATE INDEX idx_sessions_created ON test_sessions(created_at DESC);
CREATE INDEX idx_sessions_scheduled ON test_sessions(scheduled_at) WHERE scheduled_at IS NOT NULL;

-- ============================================================
-- QOE METRICS — TimescaleDB Hypertables
-- ============================================================

-- Per-second QoE metrics (main hypertable)
CREATE TABLE qoe_metrics (
    time                TIMESTAMPTZ NOT NULL,
    session_id          UUID NOT NULL REFERENCES test_sessions(id) ON DELETE CASCADE,
    device_id           UUID NOT NULL REFERENCES devices(id),
    
    -- ITU-T P.1203 metrics
    itup1203_score      REAL,
    itup1203_ci         REAL,  -- Confidence Interval
    
    -- Video quality
    psnr                REAL,
    ssim                REAL,
    vmaf                REAL,
    
    -- Playback metrics
    bitrate_bps         BIGINT,
    resolution_width    INTEGER,
    resolution_height   INTEGER,
    fps_actual          REAL,
    fps_target          INTEGER,
    
    -- Buffer & stalling
    buffer_seconds      REAL,
    buffer_ratio        REAL,  -- 0.0-1.0
    rebuffer_count      INTEGER DEFAULT 0,
    rebuffer_duration_s REAL DEFAULT 0,
    first_buffer_ms     INTEGER,
    
    -- Startup
    startup_time_ms     INTEGER,
    time_to_first_frame_ms INTEGER,
    
    -- Audio
    a_bitrate_bps       BIGINT,
    audio_codec         VARCHAR(50),
    audio_sample_rate   INTEGER,
    
    -- Network
    rtt_ms              INTEGER,
    jitter_ms           REAL,
    packet_loss_pct     REAL,
    bandwidth_bps       BIGINT,
    
    -- Composite QoE score (0-100)
    qoe_score           REAL,
    qoe_grade           CHAR(1),  -- A/B/C/D/F
    
    metadata            JSONB NOT NULL DEFAULT '{}'
);

SELECT create_hypertable('qoe_metrics', 'time', if_not_exists => TRUE);
CREATE INDEX ON qoe_metrics (session_id, time DESC);
CREATE INDEX ON qoe_metrics (device_id, time DESC);

-- Downsampled metrics for dashboards (materialized views via continuous aggregates)
CREATE MATERIALIZED VIEW qoe_metrics_1m
WITH (timescaledb.continuous) AS
SELECT
    time_bucket('1 minute', time) AS bucket,
    session_id,
    device_id,
    avg(qoe_score) AS qoe_score,
    avg(psnr) AS psnr,
    avg(ssim) AS ssim,
    avg(vmaf) AS vmaf,
    avg(bitrate_bps) AS avg_bitrate_bps,
    sum(rebuffer_count) AS total_rebuffers,
    avg(buffer_seconds) AS avg_buffer,
    max(fps_actual) AS max_fps,
    min(fps_actual) AS min_fps,
    avg(rtt_ms) AS avg_rtt,
    avg(packet_loss_pct) AS avg_packet_loss
FROM qoe_metrics
GROUP BY bucket, session_id, device_id
WITH NO DATA;

SELECT create_continuous_aggregate_policy('qoe_metrics_1m',
    start_offset => INTERVAL '7 days',
    end_offset => INTERVAL '1 hour',
    schedule_interval => INTERVAL '10 minutes');

-- Per-frame detection data
CREATE TABLE frame_detections (
    time                TIMESTAMPTZ NOT NULL,
    session_id          UUID NOT NULL REFERENCES test_sessions(id) ON DELETE CASCADE,
    frame_number        BIGINT NOT NULL,
    model               VARCHAR(50) NOT NULL,
    inference_ms        REAL,
    detections          JSONB NOT NULL DEFAULT '[]',
    metadata            JSONB NOT NULL DEFAULT '{}'
);

SELECT create_hypertable('frame_detections', 'time', if_not_exists => TRUE);
CREATE INDEX ON frame_detections (session_id, frame_number DESC);

-- ============================================================
-- SENSOR DATA — TimescaleDB Hypertable
-- ============================================================

CREATE TABLE sensor_data (
    time                TIMESTAMPTZ NOT NULL,
    session_id          UUID NOT NULL REFERENCES test_sessions(id) ON DELETE CASCADE,
    device_id           UUID NOT NULL REFERENCES devices(id),
    sensor_type         VARCHAR(50) NOT NULL,  -- imu, gps, wifi, bluetooth, nfc, battery
    data                JSONB NOT NULL,
    quality_score       REAL
);

SELECT create_hypertable('sensor_data', 'time', if_not_exists => TRUE);
CREATE INDEX ON sensor_data (session_id, sensor_type, time DESC);

-- ============================================================
-- ALERTS
-- ============================================================

CREATE TABLE alert_rules (
    id              UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    workspace_id    UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    name            VARCHAR(255) NOT NULL,
    description     TEXT,
    metric          VARCHAR(100) NOT NULL,  -- qoe_score, rebuffer_count, packet_loss, etc.
    operator        VARCHAR(20) NOT NULL CHECK (operator IN ('lt','lte','gt','gte','eq','neq')),
    threshold       REAL NOT NULL,
    duration_sec    INTEGER DEFAULT 60,  -- how long condition must hold
    severity        VARCHAR(20) NOT NULL DEFAULT 'warning'
                    CHECK (severity IN ('info','warning','critical','emergency')),
    channels        JSONB NOT NULL DEFAULT '["email"]',  -- email, slack, webhook, pagerduty
    is_active       BOOLEAN NOT NULL DEFAULT true,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_alert_rules_workspace ON alert_rules(workspace_id);

CREATE TABLE alerts (
    id              UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    workspace_id    UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    rule_id         UUID NOT NULL REFERENCES alert_rules(id),
    session_id      UUID REFERENCES test_sessions(id),
    device_id       UUID REFERENCES devices(id),
    metric_name     VARCHAR(100) NOT NULL,
    metric_value    REAL NOT NULL,
    threshold       REAL NOT NULL,
    severity        VARCHAR(20) NOT NULL,
    status          VARCHAR(50) NOT NULL DEFAULT 'fired'
                    CHECK (status IN ('fired','acknowledged','resolved','suppressed')),
    message         TEXT,
    acknowledged_at TIMESTAMPTZ,
    acknowledged_by UUID REFERENCES users(id),
    resolved_at     TIMESTAMPTZ,
    resolved_by     UUID REFERENCES users(id),
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_alerts_workspace ON alerts(workspace_id);
CREATE INDEX idx_alerts_status ON alerts(status);
CREATE INDEX idx_alerts_created ON alerts(created_at DESC);
CREATE INDEX idx_alerts_severity ON alerts(severity) WHERE status = 'fired';

-- ============================================================
-- OTA UPDATES
-- ============================================================

CREATE TABLE ota_packages (
    id              UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    workspace_id    UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    device_type     VARCHAR(50) NOT NULL,
    version         VARCHAR(50) NOT NULL,
    min_version     VARCHAR(50),
    file_url        TEXT NOT NULL,
    file_size_bytes BIGINT NOT NULL,
    file_hash_sha256 VARCHAR(64) NOT NULL,
    changelog       TEXT,
    is_stable       BOOLEAN NOT NULL DEFAULT true,
    rollout_pct     REAL NOT NULL DEFAULT 100.0,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE ota_deployment_logs (
    id              UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    package_id      UUID NOT NULL REFERENCES ota_packages(id),
    device_id       UUID NOT NULL REFERENCES devices(id),
    status          VARCHAR(50) NOT NULL
                    CHECK (status IN ('queued','downloading','installing','success','failed','rolled_back')),
    current_version VARCHAR(50),
    target_version  VARCHAR(50),
    error_message   TEXT,
    started_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    completed_at    TIMESTAMPTZ
);

CREATE INDEX idx_ota_logs_device ON ota_deployment_logs(device_id);
CREATE INDEX idx_ota_logs_status ON ota_deployment_logs(status);

-- ============================================================
-- AUDIT LOG
-- ============================================================

CREATE TABLE audit_log (
    id              BIGSERIAL PRIMARY KEY,
    workspace_id    UUID NOT NULL REFERENCES workspaces(id),
    user_id         UUID REFERENCES users(id),
    action          VARCHAR(100) NOT NULL,
    resource_type   VARCHAR(50) NOT NULL,
    resource_id     UUID,
    changes         JSONB,
    ip_address      INET,
    user_agent      VARCHAR(500),
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_audit_workspace ON audit_log(workspace_id);
CREATE INDEX idx_audit_created ON audit_log(created_at DESC);

-- ============================================================
-- FUNCTIONS & TRIGGERS
-- ============================================================

-- Auto-update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = now();
    RETURN NEW;
END;
$$ language 'plpgsql';

CREATE TRIGGER update_workspaces_updated_at BEFORE UPDATE ON workspaces
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_devices_updated_at BEFORE UPDATE ON devices
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_sessions_updated_at BEFORE UPDATE ON test_sessions
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- ============================================================
-- VIEWS
-- ============================================================

-- Device health overview
CREATE OR REPLACE VIEW v_device_health AS
SELECT
    d.id,
    d.device_id,
    d.name,
    d.model,
    d.status,
    d.last_seen,
    d.battery_pct,
    d.wifi_signal_dbm,
    COUNT(ts.id) FILTER (WHERE ts.status = 'running') AS active_sessions,
    COUNT(ts.id) FILTER (WHERE ts.status = 'completed') AS completed_sessions,
    AVG(q.qoe_score) FILTER (WHERE ts.status = 'completed') AS avg_qoe_score,
    AVG(q.rebuffer_count) FILTER (WHERE ts.status = 'completed') AS avg_rebuffers
FROM devices d
LEFT JOIN test_sessions ts ON ts.device_id = d.id
LEFT JOIN qoe_metrics q ON q.session_id = ts.id
WHERE d.workspace_id = COALESCE(NULLIF(current_setting('app.current_workspace', true), ''), ''::text)::uuid
GROUP BY d.id;

-- ============================================================
-- GRANTS (run as app user)
-- ============================================================

-- GRANT SELECT ON ALL TABLES IN SCHEMA public TO app_reader;
-- GRANT SELECT, INSERT, UPDATE ON ALL TABLES IN SCHEMA public TO app_writer;
-- GRANT ALL ON ALL TABLES IN SCHEMA public TO app_admin;
