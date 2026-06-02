// ============================================================
// MLModelManager.kt — TensorFlow Lite Inference Engine
// ============================================================
// Manages ML model loading, inference, and performance monitoring
// Supports GPU delegate, NNAPI, and CPU execution
// ============================================================

package io.qelera.sdk.ml

import android.content.Context
import org.tensorflow.lite.Interpreter
import org.tensorflow.lite.gpu.GPUDelegate
import org.tensorflow.lite.nnapi.NnApiDelegate
import org.tensorflow.lite.support.common.FileUtil
import org.tensorflow.lite.support.common.ops.NormalizeOp
import org.tensorflow.lite.support.image.ImageProcessor
import org.tensorflow.lite.support.image.TensorImage
import org.tensorflow.lite.support.image.ops.ResizeOp
import org.tensorflow.lite.support.tensorbuffer.TensorBuffer
import timber.log.Timber
import java.io.FileInputStream
import java.nio.MappedByteBuffer
import java.nio.channels.FileChannel

/**
 * ML model inference results
 */
data class InferenceResult(
    val qoeScore: Double,
    val psnr: Double,
    val ssim: Double,
    val vmaf: Double,
    val anomalyDetected: Boolean,
    val anomalyType: String?,
    val anomalyConfidence: Double,
    val inferenceTimeMs: Long
)

/**
 * Model configuration
 */
data class MLConfig(
    val modelPath: String,
    val useGPUDelegate: Boolean = true,
    val useNNAPI: Boolean = true,
    val inputSize: Int = 224,
    val numThreads: Int = 4,
    val confidenceThreshold: Float = 0.5f
)

/**
 * Manages TensorFlow Lite model inference
 */
class MLModelManager(private val context: Context) {
    private var interpreter: Interpreter? = null
    private var gpuDelegate: GPUDelegate? = null
    private var nnapiDelegate: NnApiDelegate? = null

    private var mlConfig: MLConfig? = null
    private var isInitialized = false

    // Performance monitoring
    private val inferenceTimes = mutableListOf<Long>()
    private val maxInferenceTimes = 100

    // Image processor for preprocessing
    private lateinit var imageProcessor: ImageProcessor

    /**
     * Initialize ML model
     */
    fun initialize(config: MLConfig): Result<Unit> {
        return try {
            mlConfig = config

            // Load model buffer
            val modelBuffer = loadModelBuffer(config.modelPath)

            // Configure interpreter options
            val options = Interpreter.Options().apply {
                setNumThreads(config.numThreads)
                setAllowFp16(config.useGPUDelegate)
            }

            // Create interpreter
            interpreter = Interpreter(modelBuffer, options)

            // Load GPU delegate if available
            if (config.useGPUDelegate) {
                try {
                    gpuDelegate = GPUDelegate.create()
                    interpreter?.addDelegate(gpuDelegate!!)
                    Timber.d("GPU delegate loaded")
                } catch (e: Exception) {
                    Timber.w("GPU delegate not available, falling back to CPU")
                }
            }

            // Load NNAPI delegate
            if (config.useNNAPI) {
                try {
                    nnapiDelegate = NnApiDelegate()
                    interpreter?.addDelegate(nnapiDelegate!!)
                    Timber.d("NNAPI delegate loaded")
                } catch (e: Exception) {
                    Timber.w("NNAPI delegate not available")
                }
            }

            // Initialize image processor
            imageProcessor = ImageProcessor.Builder()
                .add(ResizeOp(config.inputSize, config.inputSize, ResizeOp.ResizeMethod.BILINEAR))
                .add(NormalizeOp(0f, 255f))
                .build()

            isInitialized = true
            Timber.i("ML model initialized: ${config.modelPath}")

            Result.success(Unit)
        } catch (e: Exception) {
            Timber.e(e, "Failed to initialize ML model")
            Result.failure(e)
        }
    }

    /**
     * Load model from assets or file
     */
    private fun loadModelBuffer(modelPath: String): MappedByteBuffer {
        return try {
            // Try to load from assets
            val assetFileDescriptor = context.assets.openFd(modelPath)
            val inputStream = FileInputStream(assetFileDescriptor.fileDescriptor)
            val fileChannel = inputStream.channel
            val startOffset = assetFileDescriptor.startOffset
            val declaredLength = assetFileDescriptor.declaredLength
            fileChannel.map(FileChannel.MapMode.READ_ONLY, startOffset, declaredLength)
        } catch (e: Exception) {
            // Try to load from file
            FileInputStream(modelPath).channel.map(FileChannel.MapMode.READ_ONLY, 0, File(modelPath).length())
        }
    }

    /**
     * Process frame and run inference
     */
    fun processFrame(frame: android.graphics.ImageProxy): InferenceResult {
        if (!isInitialized) {
            return InferenceResult(
                qoeScore = 0.0, psnr = 0.0, ssim = 0.0, vmaf = 0.0,
                anomalyDetected = false, anomalyType = null,
                anomalyConfidence = 0.0, inferenceTimeMs = 0
            )
        }

        val startTime = System.currentTimeMillis()

        try {
            // Convert ImageProxy to TensorImage
            val tensorImage = convertToTensorImage(frame)

            // Preprocess image
            val processedImage = imageProcessor.process(tensorImage)

            // Prepare input buffer
            val inputBuffer = processedImage.buffer

            // Prepare output buffers
            val numOutputs = interpreter?.outputTensorCount ?: 0
            val outputBuffers = Array(numOutputs) {
                TensorBuffer.createFixedSize(
                    interpreter!!.outputTensor[it].shape(),
                    interpreter!!.outputTensor[it].dataType()
                ).buffer
            }

            // Run inference
            interpreter?.run(inputBuffer, outputBuffers)

            // Parse outputs
            val qoeScore = parseQoEScore(outputBuffers[0])
            val anomalyResult = parseAnomalyResult(outputBuffers[1])

            val inferenceTime = System.currentTimeMillis() - startTime
            inferenceTimes.add(inferenceTime)
            if (inferenceTimes.size > maxInferenceTimes) {
                inferenceTimes.removeAt(0)
            }

            return InferenceResult(
                qoeScore = qoeScore,
                psnr = 35.0, // Would be computed from frame analysis
                ssim = 0.92, // Would be computed from frame analysis
                vmaf = 88.0, // Would be computed from frame analysis
                anomalyDetected = anomalyResult.detected,
                anomalyType = anomalyResult.type,
                anomalyConfidence = anomalyResult.confidence,
                inferenceTimeMs = inferenceTime
            )
        } catch (e: Exception) {
            Timber.e(e, "Frame processing failed")
            return InferenceResult(
                qoeScore = 0.0, psnr = 0.0, ssim = 0.0, vmaf = 0.0,
                anomalyDetected = false, anomalyType = null,
                anomalyConfidence = 0.0, inferenceTimeMs = System.currentTimeMillis() - startTime
            )
        }
    }

    /**
     * Convert ImageProxy to TensorImage
     */
    private fun convertToTensorImage(image: android.graphics.ImageProxy): TensorImage {
        val tensorImage = TensorImage(ImageFormat.YUV_420_888)
        tensorImage.load(image)
        return tensorImage
    }

    /**
     * Parse QoE score from model output
     */
    private fun parseQoEScore(outputBuffer: java.nio.ByteBuffer): Double {
        // Assuming first output is QoE score (0-100)
        outputBuffer.rewind()
        val score = outputBuffer.float
        return score.toDouble()
    }

    /**
     * Parse anomaly detection result
     */
    private fun parseAnomalyResult(outputBuffer: java.nio.ByteBuffer): AnomalyResult {
        outputBuffer.rewind()

        val numClasses = 5 // motion_blur, freeze, blur, color_distortion, noise
        var maxScore = 0.0f
        var maxClass = -1

        for (i in 0 until numClasses) {
            val score = outputBuffer.float
            if (score > maxScore) {
                maxScore = score
                maxClass = i
            }
        }

        val config = mlConfig ?: return AnomalyResult(false, null, 0.0)

        return if (maxScore >= config.confidenceThreshold) {
            val types = arrayOf("motion_blur", "freeze", "blur", "color_distortion", "noise")
            AnomalyResult(true, types[maxClass], maxScore.toDouble())
        } else {
            AnomalyResult(false, null, maxScore.toDouble())
        }
    }

    /**
     * Get average inference time
     */
    val averageInferenceTimeMs: Long
        get() {
            if (inferenceTimes.isEmpty()) return 0
            return inferenceTimes.average().toLong()
        }

    /**
     * Release model resources
     */
    fun release() {
        gpuDelegate?.close()
        nnapiDelegate?.close()
        interpreter?.close()
        interpreter = null
        gpuDelegate = null
        nnapiDelegate = null
        isInitialized = false
        inferenceTimes.clear()
        Timber.d("ML model released")
    }
}

/**
 * Anomaly detection result
 */
private data class AnomalyResult(
    val detected: Boolean,
    val type: String?,
    val confidence: Double
)
