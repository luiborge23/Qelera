// ============================================================
// Data Models — Qelera Android SDK
// ============================================================
// Core data classes for QoE metrics, alerts, sessions, and configuration
// ============================================================

package io.qelera.sdk.models

import java.util.UUID

// ============================================================
// Session Models
// ============================================================

enum class SessionState {
    IDLE,
    INITIALIZED,
    STARTING,
    RUNNING,
    STOPPING,
    COMPLETED,
    ERROR
}

enum class SessionStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    CANCELLED
}

enum class TestType {
    STREAMING,
    BROWSING,
    VIDEO_CALL,
    GAMING,
    CUSTOM
}

data class SDKConfig(
    val apiEndpoint: String,
    val authToken: String,
    val deviceId: String = UUID.randomUUID().toString(),
    val workspaceId: String,
    val mlModelPath: String = "models/anomaly_detector.tflite",
    val useMLGPU: Boolean = true,
    val mqttBrokerUrl: String = "tcp://mqtt.qelera.io:1883",
    val syncIntervalMs: Long = 60000,
    val enableLocalStorage: Boolean = true
)

data class SessionInfo(
    val sessionId: String = UUID.randomUUID().toString(),
    val deviceId: String,
    val testType: TestType,
    val captureMode: CaptureMode,
    val startTime: Long = System.currentTimeMillis(),
    val status: SessionStatus = SessionStatus.PENDING
)

data class SessionSummary(
    val sessionId: String,
    val deviceId: String,
    val startTime: Long,
    val endTime: Long,
    val status: SessionStatus,
    val totalMetrics: Int = 0,
    val totalAlerts: Int = 0,
    val avgQoEScore: Double = 0.0,
    val minQoEScore: Double = 0.0,
    val maxQoEScore: Double = 0.0
)

// ============================================================
// QoE Metric Models
// ============================================================

data class QoEMetric(
    val sessionId: String,
    val deviceId: String,
    val timestamp: Long,
    val qoeScore: Double,
    val psnr: Double,
    val ssim: Double,
    val vmaf: Double,
    val bitrate: Int,
    val resolution: String,
    val frameRate: Double,
    val latencyMs: Double,
    val jitterMs: Double,
    val packetLossPct: Double,
    val bufferRatio: Double,
    val anomalyDetected: Boolean = false,
    val anomalyType: String? = null,
    val id: String = UUID.randomUUID().toString()
)

// ============================================================
// Alert Models
// ============================================================

enum class AlertType {
    ANOMALY_DETECTED,
    QOE_LOW,
    NETWORK_ISSUE,
    DEVICE_OVERHEAT,
    BATTERY_LOW,
    CONNECTION_LOST
}

enum class Severity {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
}

enum class AlertStatus {
    OPEN,
    ACKNOWLEDGED,
    RESOLVED,
    CLOSED
}

data class AlertEvent(
    val deviceId: String,
    val sessionId: String,
    val alertType: AlertType,
    val severity: Severity,
    val message: String,
    val metadata: Map<String, Any> = emptyMap(),
    val timestamp: Long = System.currentTimeMillis(),
    val status: AlertStatus = AlertStatus.OPEN,
    val id: String = UUID.randomUUID().toString()
)

// ============================================================
// Server Message Models
// ============================================================

sealed class ServerMessage {
    abstract val type: String
}

data class ConfigUpdateMessage(
    val data: JSONObject
) : ServerMessage() {
    override val type = "config_update"
}

data class SessionCommandMessage(
    val data: JSONObject
) : ServerMessage() {
    override val type = "session_command"
}

data class OTAUpdateMessage(
    val data: JSONObject
) : ServerMessage() {
    override val type = "ota_update"
}

data class UnknownMessage(
    override val type: String
) : ServerMessage()

// ============================================================
// Heartbeat Model
// ============================================================

data class HeartbeatEvent(
    val deviceId: String,
    val sessionId: String,
    val timestamp: Long = System.currentTimeMillis(),
    val batteryLevel: Int = -1,
    val cpuUsage: Double = 0.0,
    val memoryUsage: Double = 0.0
)

// ============================================================
// Network Imports for JSON
// ============================================================

import org.json.JSONObject
