// ============================================================
// VisionTrackEngine.kt — Core SDK Orchestrator
// ============================================================
// Main entry point for the Qelera Android SDK
// Manages session lifecycle, camera, sensors, ML, and networking
// ============================================================

package io.qelera.sdk.core

import android.content.Context
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.*
import timber.log.Timber
import io.qelera.sdk.camera.CameraCaptureManager
import io.qelera.sdk.camera.CaptureMode
import io.qelera.sdk.sensors.SensorCollector
import io.qelera.sdk.ml.MLModelManager
import io.qelera.sdk.network.WebSocketClient
import io.qelera.sdk.network.MQTTClient
import io.qelera.sdk.models.*
import io.qelera.sdk.storage.LocalStorageManager

/**
 * Main orchestrator for the Qelera SDK.
 * Manages the complete test session lifecycle:
 * 1. Initialization → 2. Session Start → 3. Real-time Capture → 4. Session End
 */
class VisionTrackEngine private constructor(
    private val context: Context
) {
    // Session state management
    private val _sessionState = MutableLiveData<SessionState>(SessionState.IDLE)
    val sessionState: LiveData<SessionState> = _sessionState

    // QoE metrics stream (real-time)
    private val _qoeMetrics = MutableSharedFlow<QoEMetric>(replay = 0)
    val qoeMetrics: SharedFlow<QoEMetric> = _qoeMetrics.asSharedFlow()

    // Alert stream
    private val _alerts = MutableSharedFlow<AlertEvent>(replay = 0)
    val alerts: SharedFlow<AlertEvent> = _alerts.asSharedFlow()

    // Configuration
    private var sdkConfig: SDKConfig? = null

    // Component managers
    private lateinit var cameraManager: CameraCaptureManager
    private lateinit var sensorCollector: SensorCollector
    private lateinit var mlModelManager: MLModelManager
    private lateinit var webSocketClient: WebSocketClient
    private lateinit var mqttClient: MQTTClient
    private lateinit var localStorage: LocalStorageManager

    // Background scope for coroutines
    private val engineScope = CoroutineScope(Dispatchers.IO + SupervisorJob())
    private var sessionJob: Job? = null

    // Session metadata
    private var currentSessionId: String? = null
    private var currentDeviceId: String? = null

    /**
     * Initialize the SDK with configuration
     * Must be called before starting any sessions
     */
    suspend fun initialize(config: SDKConfig): Result<Unit> {
        return try {
            Timber.d("Initializing Qelera SDK v${BuildConfig.SDK_VERSION}")

            sdkConfig = config

            // Initialize storage
            localStorage = LocalStorageManager(context)
            localStorage.initialize(config.workspaceId)

            // Initialize WebSocket client
            webSocketClient = WebSocketClient(
                endpoint = config.apiEndpoint,
                authToken = config.authToken,
                deviceId = config.deviceId
            )
            webSocketClient.initialize()

            // Initialize MQTT client
            mqttClient = MQTTClient(
                brokerUrl = config.mqttBrokerUrl,
                deviceId = config.deviceId
            )
            mqttClient.initialize()

            // Initialize ML model manager
            mlModelManager = MLModelManager(context)
            val mlInitResult = mlModelManager.initialize(
                modelPath = config.mlModelPath,
                useGPUDelegate = config.useMLGPU
            )
            if (!mlInitResult.isSuccess) {
                Timber.w("ML model initialization failed: ${mlInitResult.exceptionOrNull()?.message}")
            }

            // Initialize camera manager
            cameraManager = CameraCaptureManager(context)

            // Initialize sensor collector
            sensorCollector = SensorCollector(context)

            _sessionState.value = SessionState.INITIALIZED
            Timber.i("Qelera SDK initialized successfully")

            Result.success(Unit)
        } catch (e: Exception) {
            Timber.e(e, "Failed to initialize Qelera SDK")
            Result.failure(e)
        }
    }

    /**
     * Start a new test session
     */
    suspend fun startSession(
        sessionId: String,
        testType: TestType,
        captureMode: CaptureMode = CaptureMode.CAMERA
    ): Result<SessionInfo> {
        return try {
            if (_sessionState.value != SessionState.INITIALIZED &&
                _sessionState.value != SessionState.COMPLETED) {
                return Result.failure(IllegalStateException(
                    "SDK must be INITIALIZED or COMPLETED to start a session. Current state: ${_sessionState.value}"
                ))
            }

            _sessionState.value = SessionState.STARTING

            // Generate device ID if not provided
            currentDeviceId = currentDeviceId ?: localStorage.getOrCreateDeviceId()
            currentSessionId = sessionId

            // Initialize camera
            val cameraResult = cameraManager.initialize(captureMode)
            if (!cameraResult.isSuccess) {
                return Result.failure(cameraResult.exceptionOrNull() ?: IllegalStateException("Camera init failed"))
            }

            // Start sensor collection
            sensorCollector.startCollection(currentDeviceId!!)

            // Connect to WebSocket
            val wsResult = webSocketClient.connect(sessionId)
            if (!wsResult.isSuccess) {
                Timber.w("WebSocket connection failed, continuing with local storage")
            }

            // Connect to MQTT
            mqttClient.connect()

            // Create session info
            val sessionInfo = SessionInfo(
                sessionId = sessionId,
                deviceId = currentDeviceId!!,
                testType = testType,
                captureMode = captureMode,
                startTime = System.currentTimeMillis(),
                status = SessionStatus.RUNNING
            )

            // Save session locally
            localStorage.saveSession(sessionInfo)

            // Start real-time capture loop
            sessionJob = engineScope.launch {
                runSessionLoop(sessionInfo)
            }

            _sessionState.value = SessionState.RUNNING
            Timber.i("Session $sessionId started (type: $testType, mode: $captureMode)")

            Result.success(sessionInfo)
        } catch (e: Exception) {
            Timber.e(e, "Failed to start session")
            _sessionState.value = SessionState.ERROR
            Result.failure(e)
        }
    }

    /**
     * Main session loop — captures frames, runs ML, streams metrics
     */
    private suspend fun runSessionLoop(sessionInfo: SessionInfo) {
        try {
            // Start camera capture
            cameraManager.startCapture { frame, timestamp ->
                engineScope.launch {
                    processFrame(frame, timestamp, sessionInfo)
                }
            }

            // Continuous sensor collection
            sensorCollector.sensors.collect { sensorData ->
                engineScope.launch {
                    sendSensorData(sensorData, sessionInfo.sessionId)
                }
            }

            // Keep session alive with heartbeat
            while (_sessionState.value == SessionState.RUNNING) {
                sendHeartbeat(sessionInfo)
                delay(HEARTBEAT_INTERVAL_MS)
            }
        } catch (e: CancellationException) {
            Timber.d("Session loop cancelled")
        } catch (e: Exception) {
            Timber.e(e, "Error in session loop")
            _sessionState.value = SessionState.ERROR
        } finally {
            // Cleanup
            cameraManager.stopCapture()
            sensorCollector.stopCollection()
            mqttClient.disconnect()
        }
    }

    /**
     * Process each captured frame
     */
    private suspend fun processFrame(
        frame: android.graphics.ImageFormat,
        timestamp: Long,
        sessionInfo: SessionInfo
    ) {
        // Run ML inference
        val inferenceResult = mlModelManager.processFrame(frame)

        // Calculate QoE metrics
        val qoeMetric = QoEMetric(
            sessionId = sessionInfo.sessionId,
            deviceId = currentDeviceId!!,
            timestamp = timestamp,
            qoeScore = inferenceResult.qoeScore,
            psnr = inferenceResult.psnr,
            ssim = inferenceResult.ssim,
            vmaf = inferenceResult.vmaf,
            bitrate = frame.bitrate ?: 0,
            resolution = "${frame.width}x${frame.height}",
            frameRate = frame.frameRate?.toDouble() ?: 30.0,
            latencyMs = inferenceResult.inferenceTimeMs,
            jitterMs = 0.0, // Will be calculated from frame timing
            packetLossPct = 0.0,
            bufferRatio = 1.0,
            anomalyDetected = inferenceResult.anomalyDetected,
            anomalyType = inferenceResult.anomalyType
        )

        // Emit QoE metric
        _qoeMetrics.emit(qoeMetric)

        // Send to server
        webSocketClient.sendMetric(qoeMetric)
        mqttClient.publishMetric(qoeMetric)

        // Check for alerts
        if (qoeMetric.anomalyDetected) {
            val alert = AlertEvent(
                deviceId = currentDeviceId!!,
                sessionId = sessionInfo.sessionId,
                alertType = AlertType.ANOMALY_DETECTED,
                severity = if (qoeMetric.qoeScore < 50) Severity.CRITICAL else Severity.HIGH,
                message = "Anomaly detected: ${qoeMetric.anomalyType}",
                metadata = mapOf(
                    "anomalyClass" to qoeMetric.anomalyType,
                    "confidence" to inferenceResult.anomalyConfidence,
                    "qoeScore" to qoeMetric.qoeScore
                ),
                timestamp = System.currentTimeMillis()
            )
            _alerts.emit(alert)
            webSocketClient.sendAlert(alert)
        }

        // Store locally
        localStorage.saveQoEMetric(qoeMetric)
    }

    /**
     * Send sensor data to server
     */
    private suspend fun sendSensorData(sensorData: SensorData, sessionId: String) {
        webSocketClient.sendSensorData(sensorData)
        mqttClient.publishSensorData(sensorData)
        localStorage.saveSensorData(sensorData)
    }

    /**
     * Send heartbeat to keep session alive
     */
    private suspend fun sendHeartbeat(sessionInfo: SessionInfo) {
        val heartbeat = HeartbeatEvent(
            deviceId = currentDeviceId!!,
            sessionId = sessionInfo.sessionId,
            timestamp = System.currentTimeMillis(),
            batteryLevel = sensorCollector.batteryLevel,
            cpuUsage = sensorCollector.cpuUsage,
            memoryUsage = sensorCollector.memoryUsage
        )
        webSocketClient.sendHeartbeat(heartbeat)
    }

    /**
     * Stop the current session
     */
    suspend fun stopSession(): Result<SessionSummary> {
        return try {
            if (_sessionState.value != SessionState.RUNNING) {
                return Result.failure(IllegalStateException("No active session to stop"))
            }

            _sessionState.value = SessionState.STOPPING

            // Cancel session loop
            sessionJob?.cancel()

            // Disconnect clients
            webSocketClient.disconnect()
            mqttClient.disconnect()

            // Create session summary
            val summary = SessionSummary(
                sessionId = currentSessionId!!,
                deviceId = currentDeviceId!!,
                startTime = System.currentTimeMillis(), // Should be from sessionInfo
                endTime = System.currentTimeMillis(),
                status = SessionStatus.COMPLETED,
                totalMetrics = localStorage.getQoEMetricCount(currentSessionId!!),
                totalAlerts = localStorage.getAlertCount(currentSessionId!!)
            )

            // Save summary
            localStorage.saveSessionSummary(summary)

            _sessionState.value = SessionState.COMPLETED
            Timber.i("Session ${summary.sessionId} completed")

            Result.success(summary)
        } catch (e: Exception) {
            Timber.e(e, "Error stopping session")
            _sessionState.value = SessionState.ERROR
            Result.failure(e)
        }
    }

    /**
     * Get local QoE metrics for a session
     */
    fun getLocalQoEMetrics(sessionId: String): List<QoEMetric> {
        return localStorage.getQoEMetrics(sessionId)
    }

    /**
     * Get local alerts for a session
     */
    fun getLocalAlerts(sessionId: String): List<AlertEvent> {
        return localStorage.getAlerts(sessionId)
    }

    /**
     * Sync local data to server
     */
    suspend fun syncDataToServer(): Result<Unit> {
        return try {
            val metrics = localStorage.getPendingMetrics()
            val alerts = localStorage.getPendingAlerts()

            webSocketClient.sendBulkMetrics(metrics)
            webSocketClient.sendBulkAlerts(alerts)

            localStorage.markAsSynced(metrics.map { it.id })
            localStorage.markAsSynced(alerts.map { it.id })

            Result.success(Unit)
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    /**
     * Shutdown the SDK completely
     */
    fun shutdown() {
        engineScope.cancel()
        cameraManager.release()
        sensorCollector.release()
        mlModelManager.release()
        webSocketClient.shutdown()
        mqttClient.shutdown()
        _sessionState.value = SessionState.IDLE
        Timber.i("Qelera SDK shut down")
    }

    companion object {
        private const val HEARTBEAT_INTERVAL_MS = 30000L // 30 seconds

        @Volatile
        private var instance: VisionTrackEngine? = null

        fun getInstance(context: Context): VisionTrackEngine {
            return instance ?: synchronized(this) {
                instance ?: VisionTrackEngine(context).also { instance = it }
            }
        }
    }
}
