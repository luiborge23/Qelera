// ============================================================
// CameraCaptureManager.kt — CameraX Integration
// ============================================================
// Handles camera capture with CameraX library
// Supports multiple capture modes: CAMERA, SCREEN, HDMI
// ============================================================

package io.qelera.sdk.camera

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.ImageFormat
import android.graphics.SurfaceTexture
import android.hardware.display.DisplayManager
import android.view.Surface
import android.view.SurfaceView
import android.view.TextureView
import androidx.camera.core.*
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.video.*
import androidx.core.content.ContextCompat
import androidx.lifecycle.LifecycleOwner
import com.google.common.util.concurrent.ListenableFuture
import timber.log.Timber
import java.io.File
import java.util.concurrent.Executor
import java.util.concurrent.Executors

/**
 * Capture modes for different input sources
 */
enum class CaptureMode {
    CAMERA,      // Rear/front camera
    SCREEN,      // Screen recording (MediaProjection)
    HDMI         // HDMI capture (external device)
}

/**
 * Camera configuration
 */
data class CameraConfig(
    val captureMode: CaptureMode = CaptureMode.CAMERA,
    val cameraSelector: CameraSelector = CameraSelector.DEFAULT_BACK_CAMERA,
    val resolution: Resolution = Resolution(android.util.Size(1920, 1080)),
    val frameRate: IntRange = IntRange(30, 60),
    val targetFPS: Int = 30,
    val bitrate: Int = 5_000_000, // 5 Mbps
    val storagePath: String? = null
)

/**
 * Frame callback for real-time processing
 */
interface FrameCallback {
    fun onFrameCaptured(frame: android.graphics.ImageProxy, timestamp: Long)
}

/**
 * Manages CameraX capture operations
 */
class CameraCaptureManager(private val context: Context) {
    private var cameraProvider: ProcessCameraProvider? = null
    private var preview: Preview? = null
    private var imageAnalysis: ImageAnalysis? = null
    private var videoRecorder: VideoRecorder? = null
    private var recording: Recording? = null

    private val cameraExecutor = ContextCompat.getMainExecutor(context)
    private val analysisExecutor: Executor = Executors.newSingleThreadExecutor()

    private var frameCallback: FrameCallback? = null
    private var isCapturing = false
    private var currentConfig: CameraConfig? = null

    /**
     * Initialize the camera with configuration
     */
    @SuppressLint("UnsafeOptInUsageError")
    suspend fun initialize(config: CameraConfig): Result<Unit> {
        return try {
            currentConfig = config

            // Bind camera provider
            val cameraProviderFuture = ProcessCameraProvider.getInstance(context)
            cameraProviderFuture.addListener({
                cameraProvider = cameraProviderFuture.get()
                Timber.d("Camera provider initialized")
            }, cameraExecutor)

            cameraProviderFuture.get() // Wait for initialization

            Result.success(Unit)
        } catch (e: Exception) {
            Timber.e(e, "Failed to initialize camera")
            Result.failure(e)
        }
    }

    /**
     * Start real-time frame capture for ML processing
     */
    @SuppressLint("UnsafeOptInUsageError")
    fun startCapture(callback: FrameCallback) {
        if (isCapturing) {
            Timber.w("Capture already running")
            return
        }

        frameCallback = callback
        isCapturing = true

        val config = currentConfig ?: return

        try {
            // Create image analysis use case for ML processing
            imageAnalysis = ImageAnalysis.Builder()
                .setTargetResolution(config.resolution)
                .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                .setOutputImageFormat(ImageAnalysis.OUTPUT_IMAGE_FORMAT_YUV_420_888)
                .build()

            imageAnalysis?.setAnalyzer(analysisExecutor) { imageProxy ->
                if (isCapturing) {
                    callback.onFrameCaptured(imageProxy, System.currentTimeMillis())
                } else {
                    imageProxy.close()
                }
            }

            // Create preview for display
            preview = Preview.Builder()
                .setTargetResolution(config.resolution)
                .build()

            // Bind use cases to lifecycle
            cameraProvider?.bindToLifecycle(
                context as LifecycleOwner,
                config.cameraSelector,
                preview,
                imageAnalysis
            )

            Timber.i("Camera capture started (mode: ${config.captureMode})")
        } catch (e: Exception) {
            Timber.e(e, "Failed to start capture")
            isCapturing = false
            frameCallback = null
        }
    }

    /**
     * Stop frame capture
     */
    fun stopCapture() {
        isCapturing = false
        frameCallback = null
        Timber.d("Camera capture stopped")
    }

    /**
     * Start video recording
     */
    @SuppressLint("UnsafeOptInUsageError")
    fun startRecording(outputFile: File): Recording? {
        if (isCapturing) {
            Timber.w("Cannot record while frame capture is running")
            return null
        }

        val config = currentConfig ?: return null

        try {
            val videoFileOutput = FileOutputOptions.Builder(outputFile).build()

            val recorder = VideoRecorder.Builder()
                .setQualitySelector(QualitySelector.from(Quality.HIGHEST))
                .build()

            videoRecorder = recorder

            val recording = recorder.output
                .start(cameraExecutor, videoFileOutput)

            Timber.i("Video recording started: ${outputFile.absolutePath}")
            return recording
        } catch (e: Exception) {
            Timber.e(e, "Failed to start recording")
            return null
        }
    }

    /**
     * Stop video recording
     */
    fun stopRecording() {
        recording?.stop()
        recording = null
        Timber.d("Video recording stopped")
    }

    /**
     * Switch camera (front/back)
     */
    fun switchCamera() {
        val currentSelector = currentConfig?.cameraSelector ?: return
        val newSelector = if (currentSelector == CameraSelector.DEFAULT_BACK_CAMERA) {
            CameraSelector.DEFAULT_FRONT_CAMERA
        } else {
            CameraSelector.DEFAULT_BACK_CAMERA
        }

        currentConfig = currentConfig?.copy(cameraSelector = newSelector)

        // Rebind with new selector
        try {
            cameraProvider?.unbindAll()
            cameraProvider?.bindToLifecycle(
                context as LifecycleOwner,
                newSelector,
                preview,
                imageAnalysis
            )
            Timber.d("Camera switched to: ${if (newSelector == CameraSelector.DEFAULT_FRONT_CAMERA) "front" else "back"}")
        } catch (e: Exception) {
            Timber.e(e, "Failed to switch camera")
        }
    }

    /**
     * Toggle flash
     */
    fun toggleFlash(): Boolean {
        // Implementation depends on camera flash state
        // Returns new flash state
        return false
    }

    /**
     * Check if camera is available
     */
    fun isCameraAvailable(): Boolean {
        return try {
            val providerFuture = ProcessCameraProvider.getInstance(context)
            providerFuture.get() != null
        } catch (e: Exception) {
            false
        }
    }

    /**
     * Get list of available cameras
     */
    fun getAvailableCameras(): List<CameraInfo> {
        return try {
            val provider = ProcessCameraProvider.getInstance(context).get()
            provider.cameraSelectorMap.entries.flatMap { (_, cameras) -> cameras }
        } catch (e: Exception) {
            emptyList()
        }
    }

    /**
     * Release camera resources
     */
    fun release() {
        stopCapture()
        stopRecording()
        cameraProvider?.unbindAll()
        cameraProvider = null
        Timber.d("Camera resources released")
    }
}
