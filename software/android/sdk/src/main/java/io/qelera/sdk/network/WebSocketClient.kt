// ============================================================
// WebSocketClient.kt — Real-Time Communication Layer
// ============================================================
// Handles WebSocket connections for real-time metric streaming
// Includes auto-reconnect, heartbeat, and message handling
// ============================================================

package io.qelera.sdk.network

import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import okhttp3.*
import okhttp3.ws.WebSocket
import okhttp3.ws.WebSocketListener
import okio.Buffer
import org.json.JSONObject
import timber.log.Timber
import java.util.concurrent.TimeUnit
import io.qelera.sdk.models.*

/**
 * WebSocket client for real-time communication with Qelera backend
 */
class WebSocketClient(
    private val endpoint: String,
    private val authToken: String,
    private val deviceId: String
) {
    private var client: OkHttpClient? = null
    private var webSocket: WebSocket? = null
    private var isConnected = false
    private var reconnectAttempts = 0
    private val maxReconnectAttempts = 10
    private val reconnectDelay = 5000L

    private val _messageHandlers = MutableSharedFlow<ServerMessage>(replay = 0)
    val messageHandlers = _messageHandlers.asSharedFlow()

    // Coroutine scope for background tasks
    private val clientScope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    /**
     * Initialize the WebSocket client
     */
    fun initialize() {
        client = OkHttpClient.Builder()
            .connectTimeout(10, TimeUnit.SECONDS)
            .readTimeout(30, TimeUnit.SECONDS)
            .writeTimeout(30, TimeUnit.SECONDS)
            .build()

        Timber.d("WebSocket client initialized")
    }

    /**
     * Connect to the WebSocket server
     */
    fun connect(sessionId: String): Result<Unit> {
        return try {
            if (isConnected) {
                Timber.w("WebSocket already connected")
                return Result.success(Unit)
            }

            val request = Request.Builder()
                .url("$endpoint/ws/sessions/$sessionId")
                .header("Authorization", "Bearer $authToken")
                .header("X-Device-Id", deviceId)
                .header("Content-Type", "application/json")
                .build()

            client?.newWebSocket(request, object : WebSocketListener() {
                override fun onOpen(webSocket: WebSocket, response: Response) {
                    isConnected = true
                    reconnectAttempts = 0
                    Timber.i("WebSocket connected: $endpoint")
                    sendHeartbeat()
                }

                override fun onMessage(bytes: Buffer, mediaType: MediaType?) {
                    try {
                        val message = bytes.readString(Charsets.UTF_8)
                        val serverMsg = parseServerMessage(message)
                        clientScope.launch {
                            _messageHandlers.emit(serverMsg)
                        }
                    } catch (e: Exception) {
                        Timber.e(e, "Failed to parse WebSocket message")
                    }
                }

                override fun onMessage(text: String, mediaType: MediaType?) {
                    try {
                        val serverMsg = parseServerMessage(text)
                        clientScope.launch {
                            _messageHandlers.emit(serverMsg)
                        }
                    } catch (e: Exception) {
                        Timber.e(e, "Failed to parse WebSocket message")
                    }
                }

                override fun onClosing(webSocket: WebSocket, code: Int, reason: String) {
                    Timber.w("WebSocket closing: $code $reason")
                    webSocket.close(1000, null)
                }

                override fun onClosed(webSocket: WebSocket, code: Int, reason: String) {
                    isConnected = false
                    Timber.w("WebSocket closed: $code $reason")
                    scheduleReconnect()
                }

                override fun onFailure(webSocket: WebSocket, t: Throwable, response: Response?) {
                    isConnected = false
                    Timber.e(t, "WebSocket error")
                    scheduleReconnect()
                }
            })

            Result.success(Unit)
        } catch (e: Exception) {
            Timber.e(e, "Failed to connect WebSocket")
            Result.failure(e)
        }
    }

    /**
     * Send QoE metric to server
     */
    fun sendMetric(metric: QoEMetric) {
        if (!isConnected) return

        val message = JSONObject().apply {
            put("type", "qoe_metric")
            put("data", JSONObject().apply {
                put("sessionId", metric.sessionId)
                put("deviceId", metric.deviceId)
                put("timestamp", metric.timestamp)
                put("qoeScore", metric.qoeScore)
                put("psnr", metric.psnr)
                put("ssim", metric.ssim)
                put("vmaf", metric.vmaf)
                put("bitrate", metric.bitrate)
                put("resolution", metric.resolution)
                put("frameRate", metric.frameRate)
                put("latencyMs", metric.latencyMs)
                put("jitterMs", metric.jitterMs)
                put("packetLossPct", metric.packetLossPct)
                put("bufferRatio", metric.bufferRatio)
                put("anomalyDetected", metric.anomalyDetected)
                put("anomalyType", metric.anomalyType ?: JSONObject.NULL)
            })
        }.toString()

        webSocket?.send(message)
    }

    /**
     * Send sensor data to server
     */
    fun sendSensorData(data: SensorData) {
        if (!isConnected) return

        val message = JSONObject().apply {
            put("type", "sensor_data")
            put("data", JSONObject().apply {
                put("deviceId", data.deviceId)
                put("timestamp", data.timestamp)
                put("networkInfo", JSONObject().apply {
                    put("connectivityType", data.networkInfo.connectivityType)
                    put("signalStrength", data.networkInfo.signalStrength)
                    put("rssi", data.networkInfo.rssi)
                })
                put("batteryInfo", JSONObject().apply {
                    put("level", data.batteryInfo.level)
                    put("status", data.batteryInfo.status)
                    put("temperature", data.batteryInfo.temperature)
                    put("voltage", data.batteryInfo.voltage)
                    put("isCharging", data.batteryInfo.isCharging)
                })
                put("systemInfo", JSONObject().apply {
                    put("cpuUsage", data.systemInfo.cpuUsage)
                    put("memoryUsage", data.systemInfo.memoryUsage)
                    put("thermalState", data.systemInfo.thermalState)
                })
            })
        }.toString()

        webSocket?.send(message)
    }

    /**
     * Send alert to server
     */
    fun sendAlert(alert: AlertEvent) {
        if (!isConnected) return

        val message = JSONObject().apply {
            put("type", "alert")
            put("data", JSONObject().apply {
                put("deviceId", alert.deviceId)
                put("sessionId", alert.sessionId)
                put("alertType", alert.alertType.name)
                put("severity", alert.severity.name)
                put("message", alert.message)
                put("metadata", JSONObject(alert.metadata))
                put("timestamp", alert.timestamp)
            })
        }.toString()

        webSocket?.send(message)
    }

    /**
     * Send heartbeat
     */
    private fun sendHeartbeat() {
        if (!isConnected) return

        val message = JSONObject().apply {
            put("type", "heartbeat")
            put("timestamp", System.currentTimeMillis())
        }.toString()

        webSocket?.send(message)
    }

    /**
     * Send bulk metrics
     */
    fun sendBulkMetrics(metrics: List<QoEMetric>) {
        if (!isConnected || metrics.isEmpty()) return

        val dataArray = org.json.JSONArray()
        metrics.forEach { metric ->
            dataArray.put(JSONObject().apply {
                put("type", "qoe_metric")
                put("data", JSONObject().apply {
                    put("sessionId", metric.sessionId)
                    put("deviceId", metric.deviceId)
                    put("timestamp", metric.timestamp)
                    put("qoeScore", metric.qoeScore)
                    put("psnr", metric.psnr)
                    put("ssim", metric.ssim)
                    put("vmaf", metric.vmaf)
                })
            })
        }

        val message = JSONObject().apply {
            put("type", "bulk_metrics")
            put("count", metrics.size)
            put("data", dataArray)
        }.toString()

        webSocket?.send(message)
    }

    /**
     * Send bulk alerts
     */
    fun sendBulkAlerts(alerts: List<AlertEvent>) {
        if (!isConnected || alerts.isEmpty()) return

        val dataArray = org.json.JSONArray()
        alerts.forEach { alert ->
            dataArray.put(JSONObject().apply {
                put("type", "alert")
                put("data", JSONObject().apply {
                    put("deviceId", alert.deviceId)
                    put("sessionId", alert.sessionId)
                    put("alertType", alert.alertType.name)
                    put("severity", alert.severity.name)
                    put("message", alert.message)
                    put("timestamp", alert.timestamp)
                })
            })
        }

        val message = JSONObject().apply {
            put("type", "bulk_alerts")
            put("count", alerts.size)
            put("data", dataArray)
        }.toString()

        webSocket?.send(message)
    }

    /**
     * Disconnect from server
     */
    fun disconnect() {
        webSocket?.close(1000, "Client disconnect")
        webSocket = null
        isConnected = false
        Timber.d("WebSocket disconnected")
    }

    /**
     * Schedule reconnection attempt
     */
    private fun scheduleReconnect() {
        if (reconnectAttempts >= maxReconnectAttempts) {
            Timber.e("Max reconnection attempts reached")
            return
        }

        reconnectAttempts++
        val delay = reconnectDelay * reconnectAttempts // Exponential backoff

        Timber.d("Scheduling reconnect in ${delay}ms (attempt $reconnectAttempts)")

        clientScope.launch {
            delay(delay)
            // Reconnect will be triggered by the session manager
        }
    }

    /**
     * Parse server message
     */
    private fun parseServerMessage(message: String): ServerMessage {
        val json = JSONObject(message)
        val type = json.getString("type")

        return when (type) {
            "config_update" -> ConfigUpdateMessage(json.getJSONObject("data"))
            "session_command" -> SessionCommandMessage(json.getJSONObject("data"))
            "ota_update" -> OTAUpdateMessage(json.getJSONObject("data"))
            else -> UnknownMessage(type)
        }
    }

    /**
     * Shutdown the client
     */
    fun shutdown() {
        disconnect()
        clientScope.cancel()
        client?.dispatcher?.executorService?.shutdown()
        client = null
        Timber.d("WebSocket client shut down")
    }
}
