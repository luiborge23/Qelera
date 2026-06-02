# PhD Research Papers Summary — Qelera

## 1. Optical Flow for Motion Detection

### Paper: "Performance Study of the Farneback Algorithm for Optical Flow Tracking in Video and Images"
- **Focus:** Dense optical flow estimation for event detection
- **Key Finding:** Farneback method remains the gold standard for dense motion estimation at low computational cost
- **Relevance to Qelera:** Core algorithm for FVILD (First Video Interaction Load Duration) measurement
- **Implementation:** `cv2.calcOpticalFlowFarneback()` with pyr_scale=0.5, levels=3, winsize=15, iterations=3, poly_n=5, poly_sigma=1.1
- **Performance:** ~30-60fps on CPU for 720p, ~120fps on GPU
- **Flow Entropy Metric:** Variance of flow vectors across frame → distinguishes loading (uniform) from video (incoherent) motion

### Paper: "FastRIFE: Optimization of Real-Time Intermediate Flow Estimation" (arXiv 2105.13482)
- **Focus:** Real-time frame interpolation using learned optical flow
- **Key Finding:** RIFE achieves real-time intermediate flow estimation, useful for up-sampling low-FPS captures
- **Relevance:** Could be used to interpolate between captured frames for higher effective FPS
- **Trade-off:** GPU-dependent, may not fit edge deployment

---

## 2. Background Subtraction for State Classification

### Paper: "Evaluating a Motion-Based Region Proposal Approach with MOG2 and ViBe" (2024)
- **Focus:** Comparing MOG2 (Mixture of Gaussians) vs. ViBe for motion detection
- **Key Finding:** MOG2 superior for detecting uniform/loading screens (low motion entropy) vs. dynamic video content
- **Relevance to Qelera:** Core algorithm for spinner vs. video classification (REBUF metric)
- **Hybrid Approach:** MOG2 → if confidence < 0.7 → YOLOv8 Nano for classification
- **Implementation:** `cv2.createBackgroundSubtractorMOG2(history=500, varThreshold=50, detectShadows=True)`

### Paper: "Enhanced Surveillance: Triple Background Subtraction with YOLO V8" (IJISAE)
- **Focus:** Combining background subtraction with YOLO for improved detection
- **Key Finding:** Triple subtraction (MOG2 + ViBe + KNN) + YOLO achieves higher accuracy than any single method
- **Relevance:** Could be adapted for complex UI states (multiple overlapping elements)

---

## 3. Frame Differencing for Event Detection

### Paper: "Integrating Visual and Network Data with Deep Learning for QoE Prediction" (PMC 2023)
- **Focus:** Combining visual frame analysis with network telemetry for QoE prediction
- **Key Finding:** Histogram Bhattacharyya distance is highly effective for detecting state transitions
- **Relevance to Qelera:** Cold start detection (ALD metric)
- **Implementation:** `cv2.bhattacharyya()` between consecutive frame histograms
- **Threshold:** Bhattacharyya distance > 0.3 indicates state transition

---

## 4. Template Matching for UI State Detection

### Research Summary (Multiple papers, 2023-2025)
- **Focus:** Template matching for detecting specific UI elements (logos, navigation bars, progress indicators)
- **Key Finding:** Template matching remains highly effective for known UI patterns, especially when combined with frame differencing
- **Relevance to Qelera:** ALD detection (logo → nav bar → content area)
- **Implementation:** `cv2.matchTemplate()` with TM_CCOEFF_NORMED, threshold 0.85
- **Optimization:** Use multi-scale template matching for responsive UIs

---

## 5. Multimodal Indoor Positioning (BLE + Video)

### Paper: "Machine Learning Models for Indoor Positioning Using Bluetooth RSSI and Video Imagery" (PMC 2025, Mamede et al.)
- **Focus:** Multimodal IPS integrating BLE RSSI with video imagery
- **Key Finding:** CNN-based approaches achieve 85-92% accuracy vs. 70-75% for KNN
- **Relevance to Qelera:** Could be adapted for device proximity detection in multi-device testing
- **Algorithms:** KNN, WKNN, NB, RSS-NN, CNN-based approaches
- **Future Use:** Multi-device coordination, device clustering

---

## 6. Edge AI / TinyML for On-Device Processing

### Paper: "Deploying TinyML for Energy-Efficient Object Detection" (PMC 2025)
- **Focus:** Running ML models on ultra-low-power edge devices
- **Key Finding:** ARM Cortex-M4 (64 MHz) can run optimized models at 15-30fps for simple classification
- **Relevance to Qelera:** On-device preprocessing reduces bandwidth and latency
- **Implementation:** TensorFlow Lite Micro on Arduino Nano 33 BLE Sense
- **Models:** MobileNetV3 Small, EfficientNet-Lite0, YOLOv8 Nano

---

## 7. Deep Learning for Wireless Localization

### Paper: "Wireless Localization Using Deep Learning: A Survey" (SCILTP 2024)
- **Focus:** Comprehensive review of DL-based localization for BLE, Wi-Fi, UWB
- **Key Finding:** Graph Neural Networks (GNNs) outperform traditional ML for multi-beacon localization
- **Relevance to Qelera:** Multi-device coordination in testing clusters
- **Future Use:** Beacon-based device tracking, spatial awareness

---

## 8. YOLO for Object Detection

### Paper: "Object Detection Using YOLO: Challenges, Architectural Successors" (PMC 2024)
- **Focus:** Comprehensive review of YOLO architectures
- **Key Finding:** YOLOv8 Nano achieves best trade-off between speed and accuracy for edge deployment
- **Relevance to Qelera:** AI fallback for complex classification tasks
- **Implementation:** `ultralytics.YOLO('yolov8n.pt')` for spinner, loading, error state detection

---

## 9. Research Implementation Priority

| Priority | Paper | Implementation | Effort |
|----------|-------|---------------|--------|
| **P0** | Farneback Optical Flow Study | Core FVILD detection | 1 week |
| **P0** | MOG2 + YOLO Hybrid | Spinner vs. video classification | 1 week |
| **P1** | Frame Differencing + Bhattacharyya | Cold start detection | 3 days |
| **P1** | Template Matching Research | UI state detection | 3 days |
| **P2** | YOLO Object Detection Review | AI fallback classifier | 1 week |
| **P3** | TinyML Edge Deployment | On-device preprocessing | 2 weeks |
| **P3** | Multimodal BLE + Video | Future multi-device coordination | 3 weeks |
