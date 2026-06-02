# Detection Pipeline Design — Qelera

## 1. Pipeline Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    DETECTION PIPELINE                           │
│                                                                 │
│  INPUT: Raw frames + T0 timestamp                               │
│                                                                 │
│  STEP 1: Frame Differencing (every frame)                       │
│    prev_gray = cvtColor(prev, GRAY)                             │
│    diff = absdiff(prev_gray, curr_gray)                         │
│    IF change > baseline + k*std → EVENT CANDIDATE               │
│                                                                 │
│  STEP 2: Classify Event Type                                    │
│    A) Histogram Bhattacharyya → Cold Start transitions          │
│    B) Template Matching → ALD (logo + nav + content)            │
│    C) Dense Optical Flow (Farneback) → FVILD (flow entropy)     │
│    D) Background Subtraction (MOG2) → spinner vs video          │
│                                                                 │
│  STEP 3: AI Classification (if confidence < 0.8)                │
│    NIS Lens API with Focus Directives:                          │
│    [CRITICAL] "Detect app state transitions"                    │
│    → Structured JSON: timestamps, states, confidence            │
│                                                                 │
│  OUTPUT: T1 timestamp per metric | Latency = T1 - T0            │
└─────────────────────────────────────────────────────────────────┘
```

## 2. Core Algorithms (Reference Implementations)

### 2.1 Frame Differencing + Event Candidate Detection

```python
import cv2
import numpy as np

class FrameDetector:
    def __init__(self, k_factor=2.5, window_size=30):
        self.k = k_factor
        self.window = window_size
        self.diff_history = []
        self.baseline = None
        self.std_dev = None

    def update_baseline(self, diff_frame):
        self.diff_history.append(diff_frame.flatten())
        if len(self.diff_history) > self.window:
            self.diff_history.pop(0)
        arr = np.array(self.diff_history)
        self.baseline = np.mean(arr, axis=0)
        self.std_dev = np.std(arr, axis=0)

    def is_event_candidate(self, current_diff):
        if self.baseline is None:
            self.update_baseline(current_diff)
            return False
        change = np.abs(current_diff - self.baseline)
        threshold = self.baseline + self.k * self.std_dev
        return np.sum(change > threshold) > (current_diff.size * 0.1)
```

### 2.2 Cold Start Detection (Bhattacharyya Distance)

```python
def detect_cold_start(prev_frame, curr_frame, threshold=0.3):
    """
    Uses histogram Bhattacharyya distance to detect cold start transitions.
    High distance (>0.3) indicates major UI change (app launch).
    """
    prev_hist = cv2.calcHist([prev_frame], [0, 1, 2], None,
                             [32, 32, 32], [0, 256, 0, 256, 0, 256])
    curr_hist = cv2.calcHist([curr_frame], [0, 1, 2], None,
                             [32, 32, 32], [0, 256, 0, 256, 0, 256])
    cv2.normalize(prev_hist, prev_hist, 0, 1, cv2.NORM_MINMAX)
    cv2.normalize(curr_hist, curr_hist, 0, 1, cv2.NORM_MINMAX)
    distance = cv2.bhattacharyya(prev_hist, curr_hist)
    return distance > threshold, distance
```

### 2.3 Optical Flow for FVILD (Farneback)

```python
def calculate_flow_entropy(prev_gray, curr_gray):
    """
    Computes flow entropy to distinguish loading (uniform rotation)
    from actual video content (spatially incoherent motion).
    Reference: Farneback algorithm performance studies.
    """
    flow = cv2.calcOpticalFlowFarneback(
        prev_gray, curr_gray,
        pyr_scale=0.5, levels=3, winsize=15,
        iterations=3, poly_n=5, poly_sigma=1.1,
        flags=0
    )
    # Magnitude and angle
    magnitude, angle = cv2.cartToPolar(flow[..., 0], flow[..., 1])
    # Entropy of magnitude distribution
    hist, _ = np.histogram(magnitude.flatten(), bins=50, range=(0, magnitude.max()))
    hist = hist / hist.sum()
    entropy = -np.sum(hist * np.log2(hist + 1e-10))
    return entropy, flow

def is_spinner(entropy, threshold=2.5):
    """Spinners show uniform rotational motion → lower entropy."""
    return entropy < threshold
```

### 2.4 MOG2 + YOLO Hybrid for Spinner/Video Classification

```python
class SpinnerClassifier:
    def __init__(self, mog_history=500, yolo_conf=0.5):
        self.mog = cv2.createBackgroundSubtractorMOG2(
            history=mog_history, varThreshold=50, detectShadows=True
        )
        self.yolo = None  # Load YOLOv8 Nano
        self.confidence = 0.0

    def classify(self, frame, prev_frame):
        # Path A: MOG2 background subtraction
        mask = self.mog.apply(frame)
        motion_ratio = np.count_nonzero(mask) / mask.size
        if motion_ratio < 0.05:
            return "loading", 0.9  # Static/loading screen
        elif motion_ratio < 0.15:
            # Low motion → check if spinner
            flow_entropy, _ = calculate_flow_entropy(
                cv2.cvtColor(prev_frame, cv2.COLOR_BGR2GRAY),
                cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            )
            if is_spinner(flow_entropy):
                return "spinner", 0.85

        # Path B: YOLO fallback for complex cases
        if self.yolo:
            results = self.yolo(frame, conf=0.5)
            if results[0].boxes.conf.mean() > 0.7:
                return "video", 0.9

        return "unknown", 0.5
```

### 2.5 Template Matching for ALD

```python
def detect_ui_elements(frame, templates, threshold=0.85):
    """
    Detects logo, navigation bar, content area using template matching.
    Returns dict of {element: {"found": bool, "confidence": float, "bbox": tuple}}
    """
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    results = {}
    for name, template_path in templates.items():
        template = cv2.imread(template_path, cv2.IMREAD_GRAYSCALE)
        w, h = template.shape[1], template.shape[0]
        res = cv2.matchTemplate(gray, template, cv2.TM_CCOEFF_NORMED)
        min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)
        results[name] = {
            "found": max_val >= threshold,
            "confidence": float(max_val),
            "bbox": (max_loc[0], max_loc[1], w, h) if max_val >= threshold else None
        }
    return results

def detect_cold_start_complete(frame, templates):
    """
    Cold start is complete when: logo + nav bar + content area all detected.
    """
    elements = detect_ui_elements(frame, templates)
    cold_start_complete = all(
        elements[k]["found"] for k in ["logo", "nav", "content"]
    )
    return cold_start_complete, elements
```

## 3. Metric Calculation Engine

```python
class QoEMetrics:
    def __init__(self, fps=120):
        self.fps = fps
        self.t0 = None  # App launch timestamp
        self.t_ald = None  # App Launch Duration
        self.t_fvild = None  # First Video Interaction Load Duration
        self.rebuffer_count = 0
        self.spinner_count = 0

    def update(self, frame, prev_frame, timestamp_ms):
        if self.t0 is None:
            self.t0 = timestamp_ms
            return

        # Cold start / ALD detection
        is_cold_start, _ = detect_cold_start(prev_frame, frame)
        if is_cold_start and self.t_ald is None:
            self.t_ald = timestamp_ms

        # FVILD detection via optical flow
        entropy, flow = calculate_flow_entropy(
            cv2.cvtColor(prev_frame, cv2.COLOR_BGR2GRAY),
            cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        )
        if entropy > 3.0 and self.t_fvild is None:  # Video content detected
            self.t_fvild = timestamp_ms

        # Spinner detection
        classifier = SpinnerClassifier()
        state = classifier.classify(frame, prev_frame)
        if state[0] == "spinner":
            self.spinner_count += 1

    def get_metrics(self):
        return {
            "cold_start_ms": (self.t_ald - self.t0) if self.t_ald else None,
            "ald_ms": (self.t_ald - self.t0) if self.t_ald else None,
            "fvild_ms": (self.t_fvild - self.t0) if self.t_fvild else None,
            "rebuffer_count": self.rebuffer_count,
            "spinner_count": self.spinner_count,
            "precision_ms": 1000 / self.fps
        }
```

## 4. Performance Benchmarks (Expected)

| Algorithm | Input Size | CPU (Ryzen 5 7600X) | GPU (RTX 4060) |
|-----------|-----------|---------------------|----------------|
| Frame Differencing | 1920×1080 | 0.5ms | 0.2ms |
| Bhattacharyya Histogram | 1920×1080 | 2ms | 0.8ms |
| Farneback Optical Flow | 1280×720 | 8ms | 3ms |
| MOG2 Background Sub | 1920×1080 | 4ms | 1.5ms |
| YOLOv8 Nano | 640×640 | 12ms | 4ms |
| Template Match (3 templates) | 1920×1080 | 5ms | 2ms |
| **Total per frame** | | **~31ms** | **~11ms** |

At 120fps on GPU: 11ms × 120 = 1.32s processing vs 1s frame budget → **feasible**
At 240fps on GPU: 11ms × 240 = 2.64s vs 0.42s frame budget → **needs optimization**

**Optimization Strategy:**
- Run full pipeline at 60fps, interpolate timestamps for 120/240fps capture
- Use GPU acceleration for all OpenCV operations (`cv2.cuda`)
- Cache template matches, only re-extract on UI changes
- Early exit: if frame differencing shows <5% change, skip heavy classifiers

## 5. Critical Guardrails (From Meta Doc)

> **CRITICAL GUARDRAIL: ALL detection is visual-only at core. Audio is supplementary, never required for any metric calculation.**

Test every detector against:
1. Muted video
2. Static image
3. Slideshow
4. Black screen with no audio

This ensures our metrics are robust and not dependent on audio tracks that may vary.

## 6. Special Challenges & Solutions

| Challenge | Solution | Source |
|-----------|----------|--------|
| Spinner vs. Video | Optical flow entropy (spinner = uniform rotational; video = spatially incoherent) | Meta Doc |
| Uniform Screens | Histogram variance (near-zero for loading, textured for dark video) | Meta Doc |
| App Quirks | Programmable state machine handles per-app flow variations | Meta Doc |
| Two-Phone Sync | Shared T0 via GPIO or NTP sync; both capture cards on same host PC | Meta Doc |
| Low Light | Basler ace2 global shutter + IR illumination for iOS capture | Camera spec |
| High Motion Blur | Global shutter + high FPS (120+) eliminates motion artifacts | Camera spec |
