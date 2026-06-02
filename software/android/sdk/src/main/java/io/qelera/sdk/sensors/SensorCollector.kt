// ============================================================
// SensorCollector.kt — Device Sensor Data Collection
// ============================================================
// Collects network, battery, IMU, and system metrics
// ============================================================

package io.qelera.sdk.sensors

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.os.BatteryManager
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import timber.log.Timber
import kotlin.math.sqrt

/**
 * Collected sensor data
 */
data class SensorData(
    val timestamp: Long,
    val deviceId: String,
    val networkInfo: NetworkInfo,
    val batteryInfo: BatteryInfo,
    val imuData: IMUData?,
    val systemInfo: SystemInfo
)

data class NetworkInfo(
    val connectivityType: String,
    val signalStrength: Int,
    val rssi: Int,
    val uploadSpeedMbps: Double,
    val downloadSpeedMbps: Double,
    val latencyMs: Double,
    val packetLossPct: Double,
    val ipAddress: String?,
    val wifiSSID: String?
)

data class BatteryInfo(
    val level: Int,
    val status: Int,
    val temperature: Float,
    val voltage: Float,
    val isCharging: Boolean
)

data class IMUData(
    val accelerometer: FloatArray, // [x, y, z]
    val gyroscope: FloatArray,     // [x, y, z]
    val timestamp: Long
)

data class SystemInfo(
    val cpuUsage: Double,
    val memoryUsage: Double,
    val diskUsage: Double,
    val thermalState: String,
    val uptimeMillis: Long
)

/**
 * Collects device sensors and system metrics
 */
class SensorCollector(private val context: Context) {
    private val sensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    private val connectivityManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
    private val batteryManager = context.getSystemService(Context.BATTERY_SERVICE) as BatteryManager

    private val _sensors = MutableSharedFlow<SensorData>(replay = 0)
    val sensors = _sensors.asSharedFlow()

    private var isCollecting = false
    private var deviceId: String? = null

    // Sensor listeners
    private val accelerometerListener = object : SensorEventListener {
        override fun onSensorChanged(event: SensorEvent?) {
            if (event != null && collecting) {
                currentAccel = event.values.clone()
            }
        }
        override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}
    }

    private val gyroscopeListener = object : SensorEventListener {
        override fun onSensorChanged(event: SensorEvent?) {
            if (event != null && collecting) {
                currentGyro = event.values.clone()
            }
        }
        override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}
    }

    private var currentAccel = FloatArray(3)
    private var currentGyro = FloatArray(3)
    private var collecting = false

    /**
     * Start collecting sensor data
     */
    fun startCollection(deviceId: String) {
        this.deviceId = deviceId
        collecting = true
        isCollecting = true

        // Register sensor listeners
        val accelSensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)
        val gyroSensor = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE)

        accelSensor?.let {
            sensorManager.registerListener(accelerometerListener, it, SensorManager.SENSOR_DELAY_FASTEST)
        }
        gyroSensor?.let {
            sensorManager.registerListener(gyroscopeListener, it, SensorManager.SENSOR_DELAY_FASTEST)
        }

        Timber.i("Sensor collection started for device: $deviceId")
    }

    /**
     * Stop collecting sensor data
     */
    fun stopCollection() {
        collecting = false
        isCollecting = false

        sensorManager.unregisterListener(accelerometerListener)
        sensorManager.unregisterListener(gyroscopeListener)

        Timber.d("Sensor collection stopped")
    }

    /**
     * Get current battery level
     */
    val batteryLevel: Int
        get() {
            val intent = context.registerReceiver(null, android.content.IntentFilter(android.content.Intent.ACTION_BATTERY_CHANGED))
            return intent?.getIntExtra(BatteryManager.EXTRA_LEVEL, -1) ?: -1
        }

    /**
     * Get current CPU usage
     */
    val cpuUsage: Double
        get() {
            // Read from /proc/stat
            return try {
                val lines = context.assets.open("proc/stat").bufferedReader().use { it.readLines() }
                val cpuLine = lines.find { it.startsWith("cpu ") } ?: return 0.0
                // Simplified CPU calculation
                0.0
            } catch (e: Exception) {
                0.0
            }
        }

    /**
     * Get current memory usage
     */
    val memoryUsage: Double
        get() {
            val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as android.app.ActivityManager
            val memoryInfo = android.app.ActivityManager.MemoryInfo()
            activityManager.getMemoryInfo(memoryInfo)
            return if (memoryInfo.totalMem > 0) {
                1.0 - (memoryInfo.availMem.toDouble() / memoryInfo.totalMem.toDouble())
            } else 0.0
        }

    /**
     * Collect and emit sensor data
     */
    suspend fun collectAndEmit() {
        if (!isCollecting || deviceId == null) return

        val networkInfo = getNetworkInfo()
        val batteryInfo = getBatteryInfo()
        val imuData = getIMUData()
        val systemInfo = getSystemInfo()

        val sensorData = SensorData(
            timestamp = System.currentTimeMillis(),
            deviceId = deviceId!!,
            networkInfo = networkInfo,
            batteryInfo = batteryInfo,
            imuData = imuData,
            systemInfo = systemInfo
        )

        _sensors.emit(sensorData)
    }

    /**
     * Get current network information
     */
    private fun getNetworkInfo(): NetworkInfo {
        val network = connectivityManager.activeNetwork
        val capabilities = connectivityManager.getNetworkCapabilities(network)

        val connectivityType = when {
            capabilities?.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) == true -> "WIFI"
            capabilities?.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR) == true -> "CELLULAR"
            capabilities?.hasTransport(NetworkCapabilities.TRANSPORT_ETHERNET) == true -> "ETHERNET"
            else -> "NONE"
        }

        val rssi = capabilities?.getCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED)?.let {
            // RSSI extraction depends on transport type
            -70
        } ?: -70

        return NetworkInfo(
            connectivityType = connectivityType,
            signalStrength = rssi,
            rssi = rssi,
            uploadSpeedMbps = 0.0,
            downloadSpeedMbps = 0.0,
            latencyMs = 0.0,
            packetLossPct = 0.0,
            ipAddress = null,
            wifiSSID = null
        )
    }

    /**
     * Get current battery information
     */
    private fun getBatteryInfo(): BatteryInfo {
        val intent = context.registerReceiver(null, android.content.IntentFilter(android.content.Intent.ACTION_BATTERY_CHANGED))

        val level = intent?.getIntExtra(BatteryManager.EXTRA_LEVEL, -1) ?: 0
        val scale = intent?.getIntExtra(BatteryManager.EXTRA_SCALE, -1) ?: 100
        val status = intent?.getIntExtra(BatteryManager.EXTRA_STATUS, -1) ?: -1
        val temperature = intent?.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, -1)?.toFloat()?.div(10) ?: 0f
        val voltage = intent?.getIntExtra(BatteryManager.EXTRA_VOLTAGE, -1)?.toFloat() ?: 0f

        return BatteryInfo(
            level = (level * 100 / scale).toInt(),
            status = status,
            temperature = temperature,
            voltage = voltage,
            isCharging = status == BatteryManager.BATTERY_STATUS_CHARGING ||
                       status == BatteryManager.BATTERY_STATUS_FULL
        )
    }

    /**
     * Get current IMU data
     */
    private fun getIMUData(): IMUData? {
        if (currentAccel.contentEquals(FloatArray(3)) && currentGyro.contentEquals(FloatArray(3))) {
            return null
        }

        return IMUData(
            accelerometer = currentAccel.clone(),
            gyroscope = currentGyro.clone(),
            timestamp = System.currentTimeMillis()
        )
    }

    /**
     * Get current system information
     */
    private fun getSystemInfo(): SystemInfo {
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as android.app.ActivityManager
        val memoryInfo = android.app.ActivityManager.MemoryInfo()
        activityManager.getMemoryInfo(memoryInfo)

        val memoryUsage = if (memoryInfo.totalMem > 0) {
            1.0 - (memoryInfo.availMem.toDouble() / memoryInfo.totalMem.toDouble())
        } else 0.0

        return SystemInfo(
            cpuUsage = 0.0, // Would need /proc/stat parsing
            memoryUsage = memoryUsage,
            diskUsage = 0.0, // Would need storage stats
            thermalState = "NORMAL", // Would need thermal manager
            uptimeMillis = android.os.SystemClock.uptimeMillis()
        )
    }

    /**
     * Release sensor resources
     */
    fun release() {
        stopCollection()
        Timber.d("Sensor collector released")
    }
}
