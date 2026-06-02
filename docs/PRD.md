# Product Requirements Document — Qelera

## 1. Product Vision

Build a **developer-first, API-native QoE testing platform** that combines high-speed video capture with AI-powered analysis to measure mobile and TV app performance at sub-10ms precision. Target mobile app developers, TV/OTT teams, QA organizations, and DevOps engineers who need fast, actionable QoE data without enterprise pricing or vendor lock-in.

**Updated Vision (Q2 2026):** Expand from mobile-only testing into **Smart TV, Set-Top Box, and OTT platform testing** — becoming the unified QoE platform for every screen where video is consumed. This captures the $30B+ OTT/streaming market alongside the mobile video ecosystem.

---

## 2. Target Customers

### Primary Targets (Enterprise — Year 1-2)

| Segment | Companies | Annual Budget | Use Case |
|---------|-----------|---------------|----------|
| **Streaming Platforms** | Netflix, Disney+, Max, Paramount+, Peacock | $500K-2M/yr | QoE monitoring across Smart TV apps (Roku, Samsung Tizen, LG webOS, Android TV, Fire TV) |
| **Social Video Platforms** | Meta (IGTV/Reels TV), Google (YouTube/YouTube TV), TikTok (TikTok TV) | $500K-2M/yr | Cross-device QoE consistency — mobile to TV |
| **Cable/DBO Providers** | Comcast Xfinity, Charter Spectrum, DirecTV, Dish | $200K-1M/yr | STB app performance, DAI insertion quality, channel switch latency |
| **Smart TV OEMs** | Samsung, LG, Sony, Roku, Amazon (Fire TV) | $300K-1.5M/yr | Pre-certification of streaming apps before store approval |

### Secondary Targets (Mid-Market — Year 1)

| Segment | Description | Monthly Budget |
|---------|-------------|---------------|
| **Mobile App Developers** | B2C apps with video content (TikTok, IG Reels, YouTube) | $299-999/mo |
| **QA/Testing Agencies** | Testing services for multiple clients | $999-2,999/mo |
| **DevOps/SRE Teams** | Production app monitoring | $999-4,999/mo |
| **Research Institutions** | Academic QoE research | $99-299/mo (educational) |

---

## 3. Core Features by Phase

### Phase 1: Core Detection Pipeline (Months 1-3) — MVP

**3.1 Frame-Level Detection (Mobile)**
- [ ] Cold start detection (ALD) — logo → nav bar → content area transitions
- [ ] Video load detection (FVILD/OVILD) — optical flow entropy analysis
- [ ] Spinner vs. video classification — MOG2 + YOLOv8 Nano hybrid
- [ ] Template-based UI state detection — logo, progress bars, error toasts
- [ ] Audio-independent detection — frame diff + optical flow only

**3.2 Mobile Metrics Supported (from Meta Doc)**

| Metric | Detection Method | Target Precision |
|--------|-----------------|-----------------|
| Cold Start | Frame diff → template match | 4-6ms |
| ALD | Template match (logo + nav) + state detection | 4-6ms |
| FVILD | Dense optical flow entropy + MOG2 | 2-6ms |
| OVILD | FVILD per-swipe loop | 4ms |
| DLD | Audio onset + frame diff + template match | 4-6ms |
| REBUF | YOLOv8 Nano spinner + uniform frame classifier | 4ms |
| Upload/Publish Latency | OCR or template match for progress bars | 4-8ms |

**3.3 Real-Time Dashboard**
- [ ] Live KPI view per device
- [ ] Historical metric trends
- [ ] Alert thresholds and notifications
- [ ] Export to CSV/JSON

---

### Phase 2: Multi-Platform Expansion (Months 4-9)

**3.4 Smart TV & OTT App Testing**

| Platform | Target OS | Capture Method | Key Metrics |
|----------|-----------|----------------|-------------|
| **Samsung Tizen** | Tizen OS | HDMI capture (HDMI out from Tizen dev unit) | ALD, FVILD, REBUF, channel switch |
| **LG webOS** | webOS | HDMI capture (LG webOS dev unit) | ALD, FVILD, REBUF, app launch |
| **Roku** | Roku OS | HDMI capture (Roku dev device) | ALD, FVILD, REBUF, channel switch |
| **Android TV / Google TV** | Android TV | HDMI capture (AVerMedia GC573) | ALD, FVILD, REBUF, DAI insertion |
| **Amazon Fire TV** | Fire OS | HDMI capture | ALD, FVILD, REBUF, Alexa voice nav |

**3.5 TV-Specific Metrics**

| Metric | Method | Target | Source |
|--------|--------|--------|--------|
| **Channel Switch Time** | Frame diff + EPG overlay detection | <100ms | DVB-CI spec |
| **DAI Insertion Latency** | Template match (ad break indicator) + optical flow | <50ms | CTA-WAVE |
| **Stream Start Time** | Frame diff → black → content detection | <200ms | ATSC A/302 |
| **Rebuffering Events** | YOLOv8 spinner + uniform frame classifier | 4ms | same as mobile |
| **Video Quality Drop** | PSNR/SSIM frame comparison | <5ms | ITU-T P.1203 |
| **Ad Skippability Detection** | Template match (skip button appearance) | <100ms | CTA-2063 |
| **Picture-in-Picture** | Template match (PiP window detection) | <50ms | CTA-861 |
| **CEC Command Latency** | HDMI CEC monitor + frame diff | <20ms | HDMI CEC spec |
| **HLS/DASH Manifest Parse Time** | Network capture + frame correlation | <100ms | CTA-WAVE |
| **DRM Handshake Time** | Frame diff (black → content after DRM) | <200ms | Widevine/PlayReady |

**3.6 IGTV / YouTube TV / TikTok TV Specific Testing**

| Platform | Test Scenarios | Key QoE Concerns |
|----------|---------------|------------------|
| **Netflix (Smart TV apps)** | App launch, content load, playback, 4K switching, profile switch, subtitle toggle, audio track change, cast to TV | Cold start on Tizen/webOS, 4K rebuffer, subtitle sync, profile switching latency |
| **YouTube / YouTube TV (Android TV)** | App launch, video load, live TV channel switch, DVR controls, quality auto-switch, multi-audio, YouTube Premium ad flow | Channel switch on live TV, DVR seek latency, quality adaptation, ad insertion |
| **Meta (IGTV/Reels on TV)** | App launch, Reels feed load, swipe-to-next, full-screen transition, comment overlay, share flow | Feed load on TV, swipe latency, overlay rendering, cross-device sync |
| **TikTok (TikTok TV)** | App launch, feed load, scroll-to-next, full-screen transition, live stream join | Feed load latency, scroll responsiveness, live stream start, aspect ratio handling |

**3.7 Video Protocol Testing**

| Protocol | Test | Tool |
|----------|------|------|
| **HLS (m3u8)** | Playlist reload, segment download, quality switch, discontinuity | FFprobe + frame capture |
| **DASH (MPD)** | Manifest parse, adaptation set switch, DRM license | FFprobe + frame capture |
| **CMAF** | Chunked delivery, low-latency HLS/DASH | Frame capture + network |
| **LL-HLS** | Low-latency HLS (<10s) | Frame capture + timing |
| **AV1 codec** | Decoding performance, rebuffer on AV1 | Frame capture + decoder metrics |

---

### Phase 3: AI Agents (Months 7-12)

- [ ] **Designer Agent:** Auto-generate test plans from natural language
- [ ] **Runner Agent:** Self-discovering tests with intelligent UI navigation
- [ ] **Analyst Agent:** Autonomous error classification (crashes, playback, network)
- [ ] **TV-Specific Agent:** Detect platform-specific issues (Tizen crashes, webOS hangs, Roku OSCR)
- [ ] Trend analysis and anomaly detection
- [ ] Comparative benchmarking across app versions and platforms

---

### Phase 4: Cloud Platform & Certification (Months 10-18)

- [ ] Multi-region deployment
- [ ] Team collaboration (roles, permissions, shared dashboards)
- [ ] API integrations (Jira, GitHub, Slack, PagerDuty)
- [ ] Custom reporting and benchmarking
- [ ] SOC 2 compliance
- [ ] White-label options for agencies
- [ ] **OEM Pre-Certification Program:** Partner with Samsung, LG, Roku to offer Qelera as the recommended QoE testing tool for app submission
- [ ] **CTA-WAVE Compliance Testing:** Automated compliance checks for CTA-WAVE standards

---

## 4. Non-Functional Requirements

| Requirement | Target |
|------------|--------|
| **Precision** | <10ms latency measurement accuracy (mobile), <100ms for TV events |
| **Scalability** | 100+ concurrent devices per cluster (mobile + TV combined) |
| **Reliability** | 99.9% uptime for monitoring mode |
| **Security** | End-to-end encryption, SOC 2 Type II, GDPR compliance |
| **Openness** | Open-source core detection pipeline (Apache 2.0) |
| **Latency** | Sub-50ms end-to-end event detection |
| **Storage** | 30-day rolling metrics (cloud), configurable retention |
| **Platform Coverage** | iOS, Android, Tizen, webOS, Roku, Android TV, Fire TV |
| **Protocol Support** | HLS, DASH, CMAF, LL-HLS, AV1 |

---

## 5. Success Metrics

| Metric | Target (Month 12) | Target (Month 24) |
|--------|------------------|------------------|
| Active paying customers | 50 | 200 |
| Devices under management | 500+ | 2,000+ |
| Tests executed per month | 1M+ | 10M+ |
| NPS score | 60+ | 70+ |
| Churn rate | <5% monthly | <3% monthly |
| Revenue run rate | $500K ARR | $10M ARR |
| Enterprise customers (>$100K ACV) | 3 | 15 |
| TV/OTT revenue share | 20% | 60% |

---

## 6. Open Source vs. Commercial Split

See Section 8 of the GTM document for the full IP strategy. Summary:

| Component | Licensing | Rationale |
|-----------|-----------|-----------|
| Core detection algorithms | Apache 2.0 OSS | Community trust, developer adoption |
| Frame processing library | Apache 2.0 OSS | Foundation everyone can use |
| Dashboard UI | Apache 2.0 OSS | Lowers barrier to self-host |
| TV/OTT detection engine | **Commercial** | Key differentiator, high value |
| Multi-device orchestration | **Commercial** | Complex, high value |
| Cloud platform | **Commercial** | SaaS delivery layer |
| AI agents | **Commercial** | Proprietary models, high value |
| OEM certification program | **Commercial** | Exclusive partnerships |
| Enterprise features (SSO, audit, SLA) | **Commercial** | Enterprise requirements |

---

## 7. Out of Scope (For Now)

- Audio-only content testing (podcasts, radio apps)
- VR/AR video testing (Apple Vision Pro, Meta Quest)
- In-game video stream testing (NBA League Pass in-game)
- Hardware manufacturing (use commodity capture cards)
- Mobile SDK for in-app telemetry (complementary, not competitive)
