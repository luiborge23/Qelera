# Qelera Android SDK — Core Architecture v1.0
# Android 8.0+ (API 26+), Kotlin, CameraX, ML Kit
# Created: May 28, 2026

# ============================================================
# PROJECT STRUCTURE
# ============================================================
# android-sdk/
# ├── qelera-core/
# │   ├── build.gradle.kts
# │   ├── src/main/
# │   │   ├── AndroidManifest.xml
# │   │   └── java/io/qelera/core/
# │   │       ├── VisionTrackEngine.kt        # Main orchestrator
# │   │       ├── config/
# │   │       │   ├── SDKConfig.kt            # Configuration model
# │   │       │   └── TestProfile.kt          # Test profiles (HLS, DASH, etc.)
# │   │       ├── capture/
# │   │       │   ├── VideoCaptureManager.kt  # CameraX/HDMI capture
# │   │       │   ├── ScreenCaptureManager.kt # Screen recording API
# │   │       │   └── FrameProcessor.kt       # Frame processing pipeline
# │   │       ├── sensors/
# │   │       │   ├── SensorCollector.kt      # Unified sensor interface
# │   │       │   ├── NetworkMetrics.kt       # Network performance
# │   │       │   ├── BatteryMonitor.kt       # Battery tracking
# │   │       │   └── IMUData.kt              # Accelerometer/Gyroscope
# │   │       ├── ml/
# │   │       │   ├── MLModelManager.kt       # TFLite model loading
# │   │       │   ├── AnomalyDetector.kt      # Anomaly detection engine
# │   │       │   ├── FrameAnalyzer.kt        # Per-frame analysis
# │   │       │   └── QualityMetrics.kt       # PSNR, SSIM, VMAF
# │   │       ├── network/
# │   │       │   ├── WebSocketClient.kt      # Real-time data sync
# │   │       │   ├── APIManager.kt           # REST API client
# │   │       │   └── BufferManager.kt        # Data buffering & retry
# │   │       ├── session/
# │   │       │   ├── TestSessionManager.kt   # Session lifecycle
# │   │       │   ├── SessionRecorder.kt      # Local recording
# │   │       │   └── SessionMetrics.kt       # Session-level aggregation
# │   │       ├── alert/
# │   │       │   ├── AlertManager.kt         # Alert evaluation
# │   │       │   ├── AlertRule.kt            # Rule definition
# │   │       │   └── NotificationService.kt  # Push notifications
# │   │       ├── utils/
# │   │       │   ├── Logger.kt               # Structured logging
# │   │       │   ├── CryptoUtils.kt          # Encryption helpers
# │   │       │   └── DeviceInfo.kt           # Device fingerprinting
# │   │       └── models/
# │   │           ├── QoEMetric.kt            # QoE data model
# │   │           ├── AlertEvent.kt           # Alert model
# │   │           └── DeviceReport.kt         # Device status report
# │   └── proguard-rules.pro
# ├── qelera-sample/
# │   ├── build.gradle.kts
# │   └── src/main/
# │       ├── java/io/qelera/sample/
# │       │   ├── MainActivity.kt             # Sample app entry
# │       │   ├── DashboardActivity.kt        # Live dashboard
# │       │   └── TestRunner.kt               # Test execution UI
# │       └── res/
# │           └── layout/
# ├── build.gradle.kts
# ├── settings.gradle.kts
# └── README.md

# ============================================================
# BUILD CONFIGURATION (build.gradle.kts)
# ============================================================
# plugins {
#     id("com.android.library")
#     id("org.jetbrains.kotlin.android")
#     id("kotlin-kapt")
#     id("com.google.devtools.ksp")
#     id("org.jetbrains.kotlin.plugin.serialization")
# }
#
# android {
#     namespace = "io.qelera.core"
#     compileSdk = 34
#
#     defaultConfig {
#         minSdk = 26
#         targetSdk = 34
#         versionCode = 1
#         versionName = "1.0.0"
#
#         testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
#     }
#
#     buildTypes {
#         release {
#             isMinifyEnabled = true
#             proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
#         }
#         debug {
#             isMinifyEnabled = false
#             isDebuggable = true
#         }
#     }
#
#     compileOptions {
#         sourceCompatibility = JavaVersion.VERSION_17
#         targetCompatibility = JavaVersion.VERSION_17
#     }
#
#     kotlinOptions {
#         jvmTarget = "17"
#     }
#
#     buildFeatures {
#         buildConfig = true
#     }
# }
#
# dependencies {
#     // AndroidX
#     implementation("androidx.core:core-ktx:1.12.0")
#     implementation("androidx.appcompat:appcompat:1.6.1")
#     implementation("com.google.android.material:material:1.11.0")
#
#     // CameraX
#     implementation("androidx.camera:camera-core:1.3.3")
#     implementation("androidx.camera:camera-camera2:1.3.3")
#     implementation("androidx.camera:camera-lifecycle:1.3.3")
#     implementation("androidx.camera:camera-video:1.3.3")
#     implementation("androidx.camera:camera-view:1.3.3")
#     implementation("androidx.camera:camera-extensions:1.3.3")
#
#     // ML Kit / TFLite
#     implementation("org.tensorflow:tensorflow-lite:2.15.0")
#     implementation("org.tensorflow:tensorflow-lite-support:0.4.4")
#     implementation("org.tensorflow:tensorflow-lite-gpu-delegate-plugin:2.15.0")
#     implementation("com.google.mlkit:vision-common:17.3.0")
#     implementation("com.google.mlkit:image-labeling:17.0.7")
#
#     // Networking
#     implementation("com.squareup.okhttp3:okhttp:4.12.0")
#     implementation("com.squareup.retrofit2:retrofit:2.9.0")
#     implementation("com.squareup.retrofit2:converter-gson:2.9.0")
#     implementation("com.squareup.okhttp3:logging-interceptor:4.12.0")
#
#     // WebSocket
#     implementation("com.squareup.okhttp3:okhttp-ws:4.12.0")
#
#     // Serialization
#     implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.6.3")
#
#     // Coroutines
#     implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.7.3")
#     implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:1.7.3")
#
#     // Sensors
#     implementation("androidx.core:core-ktx:1.12.0")
#
#     // Storage
#     implementation("androidx.room:room-runtime:2.6.1")
#     implementation("androidx.room:room-ktx:2.6.1")
#     kapt("androidx.room:room-compiler:2.6.1")
#
#     // Notifications
#     implementation("androidx.work:work-runtime-ktx:2.9.0")
#
#     // Testing
#     testImplementation("junit:junit:4.13.2")
#     testImplementation("org.mockito:mockito-core:5.11.0")
#     testImplementation("org.jetbrains.kotlinx:kotlinx-coroutines-test:1.7.3")
#     androidTestImplementation("androidx.test.ext:junit:1.1.5")
#     androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")
# }

# ============================================================
# CORE ENGINE (VisionTrackEngine.kt)
# ============================================================
# package io.qelera.core
#
# import android.content.Context
# import kotlinx.coroutines.*
# import kotlinx.coroutines.flow.*
#
# class VisionTrackEngine private constructor(private val context: Context) {
#
#     companion object {
#         @Volatile
#         private var instance: VisionTrackEngine? = null
#
#         fun getInstance(context: Context): VisionTrackEngine {
#             return instance ?: synchronized(this) {
#                 instance ?: VisionTrackEngine(context.applicationContext).also { instance = it }
#             }
#         }
#     }
#
#     // Core components
#     private val config: SDKConfig
#     private val captureManager: VideoCaptureManager
#     private val sensorCollector: SensorCollector
#     private val mlManager: MLModelManager
#     private val sessionManager: TestSessionManager
#     private val alertManager: AlertManager
#     private val networkClient: WebSocketClient
#
#     // State
#     private val _sessionState = MutableStateFlow(SessionState.IDLE)
#     val sessionState: StateFlow<SessionState> = _sessionState.asStateFlow()
#
#     private val _qoeMetrics = MutableSharedFlow<QoEMetric>()
#     val qoeMetrics: SharedFlow<QoEMetric> = _qoeMetrics
#
#     private val _alerts = MutableSharedFlow<AlertEvent>()
#     val alerts: SharedFlow<AlertEvent> = _alerts
#
#     private var engineJob: Job? = null
#
#     // Lifecycle
#     suspend fun initialize(config: SDKConfig): Result<Unit> = withContext(Dispatchers.IO) {
#         try {
#             this@VisionTrackEngine.config = config
#             captureManager.initialize(config.captureMode)
#             sensorCollector.initialize(context)
#             mlManager.initialize(context, config.mlModelPath)
#             networkClient.initialize(config.apiEndpoint, config.authToken)
#             sessionManager.initialize(config.workspaceId)
#             alertManager.initialize(config.alertRules)
#             _sessionState.value = SessionState.INITIALIZED
#             Result.success(Unit)
#         } catch (e: Exception) {
#             _sessionState.value = SessionState.ERROR
#             Result.failure(e)
#         }
#     }
#
#     suspend fun startSession(sessionId: String, testType: TestType): Result<Unit> =
#         withContext(Dispatchers.IO) {
#             try {
#                 sessionManager.startSession(sessionId, testType)
#                 captureManager.startCapture()
#                 sensorCollector.startCollection()
#                 engineJob = launch { runInferencePipeline() }
#                 _sessionState.value = SessionState.RUNNING
#                 Result.success(Unit)
#             } catch (e: Exception) {
#                 _sessionState.value = SessionState.ERROR
#                 Result.failure(e)
#             }
#         }
#
#     suspend fun stopSession(): Result<Unit> = withContext(Dispatchers.IO) {
#         try {
#             engineJob?.cancel()
#             captureManager.stopCapture()
#             sensorCollector.stopCollection()
#             sessionManager.endSession()
#             _sessionState.value = SessionState.COMPLETED
#             Result.success(Unit)
#         } catch (e: Exception) {
#             Result.failure(e)
#         }
#     }
#
#     private suspend fun runInferencePipeline() {
#         captureManager.frameStream.collect { frame ->
#             val analysis = mlManager.analyzeFrame(frame)
#             val metrics = computeQoEMetrics(frame, analysis)
#             _qoeMetrics.emit(metrics)
#             alertManager.evaluate(metrics)
#             networkClient.send(metrics)
#         }
#     }
#
#     enum class SessionState { IDLE, INITIALIZED, RUNNING, PAUSED, COMPLETED, ERROR }
#     enum class CaptureMode { CAMERA, SCREEN_RECORD, HDMI }
#     enum class TestType { STREAMING, LIVE, VOD, GAMING, VIDEO_CALL }
# }

# ============================================================
# VIDEO CAPTURE (VideoCaptureManager.kt)
# ============================================================
# package io.qelera.core.capture
#
# import androidx.camera.core.*
# import androidx.camera.lifecycle.ProcessCameraProvider
# import kotlinx.coroutines.flow.*
# import java.util.concurrent.ExecutorService
#
# class VideoCaptureManager {
#
#     private var cameraProvider: ProcessCameraProvider? = null
#     private var preview: Preview? = null
#     private var imageAnalysis: ImageAnalysis? = null
#     private val frameStream = MutableSharedFlow<Frame>()
#     private lateinit var cameraExecutor: ExecutorService
#
#     suspend fun initialize(mode: CaptureMode): Result<Unit> = withContext(Dispatchers.IO) {
#         try {
#             cameraProvider = ProcessCameraProvider.getInstance(context).get()
#             cameraExecutor = Executors.newSingleThreadExecutor()
#
#             when (mode) {
#                 CaptureMode.CAMERA -> setupCameraCapture()
#                 CaptureMode.SCREEN_RECORD -> setupScreenCapture()
#                 CaptureMode.HDMI -> setupHDMICapture()  // External capture card
#             }
#             Result.success(Unit)
#         } catch (e: Exception) {
#             Result.failure(e)
#         }
#     }
#
#     private fun setupCameraCapture() {
#         val previewBuilder = Preview.Builder().build()
#         preview = previewBuilder
#         preview?.setSurfaceProvider(surfaceProvider)
#
#         val analysisBuilder = ImageAnalysis.Builder()
#             .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
#             .setOutputImageFormat(ImageAnalysis.OUTPUT_IMAGE_FORMAT_RGBA_8888)
#             .build()
#
#         analysisBuilder.setAnalyzer(cameraExecutor) { imageProxy ->
#             val frame = Frame.fromImageProxy(imageProxy)
#             frameStream.tryEmit(frame)
#         }
#
#         cameraProvider?.bindToLifecycle(
#             lifecycleOwner,
#             CameraSelector.DEFAULT_BACK_CAMERA,
#             preview,
#             analysisBuilder
#         )
#     }
#
#     fun startCapture() { /* Resume camera */ }
#     fun stopCapture() { /* Release camera */ }
# }

# ============================================================
# SENSOR COLLECTOR (SensorCollector.kt)
# ============================================================
# package io.qelera.core.sensors
#
# import android.content.Context
# import android.hardware.Sensor
# import android.hardware.SensorManager
# import android.net.wifi.WifiManager
# import kotlinx.coroutines.flow.*
#
# class SensorCollector(private val context: Context) {
#
#     private val sensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
#     private val wifiManager = context.applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
#
#     private val _networkMetrics = MutableSharedFlow<NetworkMetrics>()
#     val networkMetrics: SharedFlow<NetworkMetrics> = _networkMetrics
#
#     private val _batteryData = MutableSharedFlow<BatteryData>()
#     val batteryData: SharedFlow<BatteryData> = _batteryData
#
#     private val _imuData = MutableSharedFlow<IMUData>()
#     val imuData: SharedFlow<IMUData> = _imuData
#
#     private var networkCollectorJob: Job? = null
#     private var batteryCollectorJob: Job? = null
#     private var imuCollectorJob: Job? = null
#
#     suspend fun initialize(context: Context) {
#         // Setup sensor listeners
#         val accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)
#         val gyroscope = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE)
#
#         if (accelerometer != null) {
#             sensorManager.registerListener(accelerometerListener, accelerometer, SensorManager.SENSOR_DELAY_FASTEST)
#         }
#         if (gyroscope != null) {
#             sensorManager.registerListener(gyroscopeListener, gyroscope, SensorManager.SENSOR_DELAY_FASTEST)
#         }
#     }
#
#     fun startCollection() {
#         networkCollectorJob = coroutineScope.launch {
#             while (isActive) {
#                 val metrics = collectNetworkMetrics()
#                 _networkMetrics.emit(metrics)
#                 delay(1000)  // Every second
#             }
#         }
#
#         batteryCollectorJob = coroutineScope.launch {
#             while (isActive) {
#                 val data = collectBatteryData()
#                 _batteryData.emit(data)
#                 delay(5000)  // Every 5 seconds
#             }
#         }
#     }
#
#     fun stopCollection() {
#         networkCollectorJob?.cancel()
#         batteryCollectorJob?.cancel()
#         imuCollectorJob?.cancel()
#         sensorManager.unregisterAllListeners()
#     }
#
#     private suspend fun collectNetworkMetrics(): NetworkMetrics {
#         val wifiInfo = wifiManager.connectionInfo
#         return NetworkMetrics(
#             ssid = wifiInfo.ssid?.trim('"'),
#             bssid = wifiInfo.bssid,
#             signalStrength = wifiInfo.rssi,
#             linkSpeed = wifiInfo.linkSpeed,
#             ipAddress = wifiInfo.ipAddress,
#             timestamp = System.currentTimeMillis()
#         )
#     }
# }

# ============================================================
# ML MODEL MANAGER (MLModelManager.kt)
# ============================================================
# package io.qelera.core.ml
#
# import org.tensorflow.lite.Interpreter
# import org.tensorflow.lite.support.common.FileUtil
# import org.tensorflow.lite.support.image.TensorImage
# import org.tensorflow.lite.support.tensorbuffer.TensorBuffer
# import android.content.Context
# import kotlinx.coroutines.Dispatchers
# import kotlinx.coroutines.withContext
#
# class MLModelManager {
#
#     private var interpreter: Interpreter? = null
#     private val modelOptions = Interpreter.Options().setNumThreads(4)
#
#     suspend fun initialize(context: Context, modelPath: String) = withContext(Dispatchers.IO) {
#         val modelBuffer = FileUtil.loadMappedFile(context, modelPath)
#         interpreter = Interpreter(modelBuffer, modelOptions)
#     }
#
#     fun analyzeFrame(frame: Frame): FrameAnalysis {
#         val input = TensorImage.fromBitmap(frame.bitmap)
#         val outputBuffer = TensorBuffer.createFixedSize(intArrayOf(1, 1000), "FLOAT32")
#
#         interpreter?.run(input.buffer, outputBuffer.buffer.rewind())
#
#         val probabilities = outputBuffer.floatArray
#         val maxIndex = probabilities.indexOf(probabilities.max())
#
#         return FrameAnalysis(
#             anomalyClass = anomalyClasses[maxIndex],
#             confidence = probabilities[maxIndex],
#             inferenceTimeMs = frame.processingTimeMs,
#             frameNumber = frame.frameNumber
#         )
#     }
#
#     fun close() {
#         interpreter?.close()
#         interpreter = null
#     }
#
#     private val anomalyClasses = listOf(
#         "no_anomaly",
#         "motion_blur",
#         "freeze_frame",
#         "color_distortion",
#         "audio_video_desync",
#         "resolution_drop",
#         "bitrate_spike",
#         "buffering"
#     )
# }

# ============================================================
# NETWORK CLIENT (WebSocketClient.kt)
# ============================================================
# package io.qelera.core.network
#
# import okhttp3.*
# import okhttp3.ws.WebSocket
# import okhttp3.ws.WebSocketListener
# import okio.Buffer
# import kotlinx.coroutines.*
# import kotlinx.coroutines.channels.awaitClose
# import kotlinx.coroutines.flow.Flow
# import kotlinx.coroutines.flow.callbackFlow
#
# class WebSocketClient(private val endpoint: String) {
#
#     private var webSocket: WebSocket? = null
#     private val client = OkHttpClient.Builder()
#         .readTimeout(30, TimeUnit.SECONDS)
#         .writeTimeout(30, TimeUnit.SECONDS)
#         .build()
#
#     private val _messageFlow = callbackFlow<Message> {
#         val listener = object : WebSocketListener() {
#             override fun onMessage(webSocket: WebSocket, text: String) {
#                 try {
#                     val message = parseMessage(text)
#                     trySend(message)
#                 } catch (e: Exception) {
#                     // Log parse error
#                 }
#             }
#
#             override fun onClosed(webSocket: WebSocket, code: Int, reason: String) {
#                 close()
#             }
#         }
#
#         val request = Request.Builder()
#             .url(endpoint)
#             .build()
#
#         webSocket = client.newWebSocket(request, listener)
#
#         awaitClose {
#             webSocket?.close(1000, "Client disconnect")
#         }
#     }
#
#     suspend fun send(metric: QoEMetric) {
#         val json = serializeMetric(metric)
#         webSocket?.let { ws ->
#             val buffer = Buffer().writeUtf8(json)
#             ws.write(WebSocket.CLOSE, buffer)
#         }
#     }
#
#     fun disconnect() {
#         webSocket?.close(1000, "Client disconnect")
#     }
# }

# ============================================================
# PROGUARD RULES (proguard-rules.pro)
# ============================================================
# -keep class io.qelera.core.** { *; }
# -keepattributes *Annotation*
# -keepattributes Signature
# -keepclassmembers class * implements android.os.Parcelable {
#     public static final ** CREATOR;
# }
# -keep class org.tensorflow.** { *; }
# -keep class org.tensorflow.lite.** { *; }
# -dontwarn okio.**
# -dontwarn kotlinx.coroutines.**
