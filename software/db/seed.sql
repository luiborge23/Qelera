-- VisionTrack Seed Data v1.0

-- Seed device types
INSERT INTO device_types (code, name) ON CONFLICT (code) DO NOTHING
VALUES
    ('android', 'Android Device'),
    ('ios', 'iOS Device'),
    ('esp32', 'ESP32 Sensor'),
    ('rpi', 'Raspberry Pi Gateway');

-- Seed default workspace
INSERT INTO workspaces (id, name, slug, plan, max_devices, data_retention_days, settings)
VALUES (
    '00000000-0000-0000-0000-000000000001',
    'VisionTrack Default',
    'default',
    'enterprise',
    1000,
    90,
    '{"theme":"dark","timezone":"UTC","locale":"en-US"}'::jsonb
) ON CONFLICT (id) DO NOTHING;

-- Seed default admin user (password: visiontrack2026)
-- Hash: bcrypt $2b$12$... (use actual bcrypt in production)
INSERT INTO users (id, workspace_id, email, password_hash, display_name, role)
VALUES (
    '00000000-0000-0000-0000-000000000001',
    '00000000-0000-0000-0000-000000000001',
    'admin@visiontrack.io',
    '$2b$12$LhKdDqZ3xP5qG8wN2vYj7eR4tU6oI9pA1sD3fG5hJ7kL0mN2qR4sT',
    'System Admin',
    'owner'
) ON CONFLICT (id) DO NOTHING;

-- Seed default API key for development
-- Key prefix: vt_live_dev001
-- Hash the key for storage
INSERT INTO api_keys (workspace_id, key_hash, key_prefix, name, permissions, is_active, created_by)
VALUES (
    '00000000-0000-0000-0000-000000000001',
    md5('vt_live_dev001_secret_key'),
    'vt_live_dev0',
    'Development Key',
    '["read","write","admin"]'::jsonb,
    true,
    '00000000-0000-0000-0000-000000000001'
) ON CONFLICT DO NOTHING;

-- Seed default alert rules
INSERT INTO alert_rules (workspace_id, name, description, metric, operator, threshold, duration_sec, severity, channels, is_active)
VALUES
    ('00000000-0000-0000-0000-000000000001', 'Low QoE Score', 'QoE score drops below 70', 'qoe_score', 'lt', 70, 120, 'warning', '["email"]', true),
    ('00000000-0000-0000-0000-000000000001', 'High Rebuffer Rate', 'More than 5 rebuffers in 1 minute', 'rebuffer_count', 'gt', 5, 60, 'critical', '["email","slack"]', true),
    ('00000000-0000-0000-0000-000000000001', 'High Packet Loss', 'Packet loss exceeds 2%', 'packet_loss_pct', 'gt', 2, 60, 'warning', '["email"]', true),
    ('00000000-0000-0000-0000-000000000001', 'Device Offline', 'Device not seen for 10 minutes', 'device_status', 'eq', 0, 600, 'info', '["email"]', true);
