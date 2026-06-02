# Competitor Analysis — Qelera

## 1. WITBE (Primary Competitor)

### Company Profile
- **Founded:** 2000 (25th anniversary 2025)
- **Public:** Euronext Growth (FR0013143872)
- **Scale:** 200K+ errors detected, 50K+ channels tested, 120+ countries, 20K+ devices
- **Clients:** Comcast, Verizon, Bouygues Telecom, A+E Global Media, TAG Video Systems
- **Awards:** NAB Product of the Year (2023, 2024), CSI Award, DTR Diamond Reviews

### Product Lines
- **WitboxOne:** Single device monitoring unit (~$2.5K-5K hardware)
- **Witbox+:** Up to 4 devices per unit (~$5K-10K hardware)
- **WitboxNet:** Web/browser testing gateway
- **Smartgate:** Cloud observability platform (KPI dashboards, alerts)
- **Agentic AI:** (launched July 2025) Designer/Runner/Analyst agents

### Pricing Model
- **No public pricing** — enterprise sales only
- Estimated: $50K-500K+ per year per deployment
- Hardware sold separately (Witbox units)
- Long contracts (1-3 years typical)

### Strengths
- 25 years of industry relationships
- Massive global deployment (120+ countries)
- Smart TV / Set-Top Box expertise
- NAB Show Product of the Year (2 consecutive years)
- Agentic AI platform (though new and unproven)
- Frame-level ad detection and fingerprinting

### Weaknesses (Our Attack Vectors)
- **Pricing is opaque and expensive** — no self-serve option
- **TV/STB centric** — mobile is a secondary market
- **Hardware-locked** — must buy Witbox units
- **No open-source component** — complete vendor lock-in
- **Agentic AI is brand new** (July 2025) — reliability unproven
- **No developer API** — built for QA teams, not developers
- **Slow deployment** — weeks to months for enterprise onboarding
- **Rigid platform** — hard to customize beyond their use cases
- **Limited mobile app testing** — focused on streaming apps on TV

### Our Counter-Strategy
1. **Open-source core** — build trust, attract developers, create community
2. **Mobile-first** — fill the gap Witbe ignores
3. **API-native** — integrate with existing DevOps toolchains
4. **Self-serve pricing** — $299/mo starting point, no sales call required
5. **Faster deployment** — plug and play in hours, not weeks
6. **Research-backed precision** — sub-10ms using PhD-level computer vision

---

## 2. STB-TESTER (Secondary Competitor)

### Company Profile
- **Open-source** core test execution engine (Python-based)
- **Hardware:** HDMI Nodes for capturing device output
- **Services:** Boutique consultancy for test automation

### Strengths
- Truly open-source core
- Strong in STB/Smart TV testing
- CI/CD integration
- Trusted by YouView, Sony, Telefónica, VodafoneZiggo

### Weaknesses
- STB/Smart TV focused only — no mobile
- No AI agents or ML-powered analysis
- No cloud platform (on-premises only)
- No real-time monitoring (batch testing only)
- Small team, limited resources for innovation

### Our Position
- **Better than stb-tester for mobile** — stb-tester has no mobile support
- **Better than stb-tester for AI** — we add ML-powered analysis
- **Better than stb-tester for cloud** — we offer a hosted platform
- **Worse than stb-tester for STB** — we're not targeting this market yet

---

## 3. MUX DATA (Complementary, Not Direct Competitor)

### Company Profile
- **Software-only** video analytics API
- **In-app SDK** — requires instrumentation in the app
- **Real-time QoE metrics** — startup time, rebuffering, errors

### Strengths
- Developer-friendly API
- Easy integration (few lines of code)
- Real-time analytics at scale
- Trusted by major streaming platforms

### Weaknesses
- **Requires app instrumentation** — not black-box testing
- **No device control** — can't run tests, only monitor
- **No physical testing** — purely software-based
- **Complementary to Qelera** — we can integrate with Mux, not replace it

### Our Position
- **We do what Mux CAN'T** — black-box testing on real devices
- **We complement Mux** — Mux gives in-app metrics, we give device-level metrics
- **Integration opportunity** — show both views in our dashboard

---

## 4. ROHDE & SCHWARZ (Enterprise Lab)

### Company Profile
- **Professional A/V quality testing** equipment
- **Lab-grade** accuracy, D/A interface analysis
- **VTC/VTE** video testers for STB/TV testing

### Strengths
- Lab-grade measurement accuracy
- D/A interface analysis (bit-perfect)
- Trusted by broadcast industry
- Decades of reputation

### Weaknesses
- Very expensive ($50K+ per unit)
- Lab-only, no remote monitoring
- No cloud platform
- No mobile support
- No AI or ML capabilities

### Our Position
- **Not competing** — we're for field/lab hybrid testing
- **More affordable** — $3K vs $50K
- **More versatile** — mobile + desktop + cloud

---

## 5. COMPETITIVE MATRIX

| Feature | Qelera | Witbe | stb-tester | Mux | R&S |
|---------|------------|-------|------------|-----|-----|
| **Mobile Testing** | ✅ Primary | ⚠️ Secondary | ❌ | ⚠️ SDK only | ❌ |
| **Smart TV Testing** | ❌ | ✅ Primary | ✅ | ❌ | ⚠️ |
| **Black-Box Testing** | ✅ | ✅ | ✅ | ❌ (SDK) | ✅ |
| **Real-Time Monitoring** | ✅ | ✅ | ⚠️ Batch | ✅ | ❌ |
| **AI Agents** | ✅ Planned | ✅ New | ❌ | ❌ | ❌ |
| **Open Source** | ✅ Core | ❌ | ✅ | ❌ | ❌ |
| **Cloud Platform** | ✅ | ✅ | ❌ | ✅ | ❌ |
| **Developer API** | ✅ | ❌ | ✅ | ✅ | ❌ |
| **Self-Serve Pricing** | ✅ | ❌ | ❌ | ✅ | ❌ |
| **Starting Price** | $299/mo | $50K+/yr | Custom | Usage | $50K+ |
| **Deployment** | Hours | Weeks | Days | Minutes | Weeks |
| **Precision** | <10ms | ~50-100ms | ~100ms | Network-level | Lab-grade |

---

## 6. MARKET OPPORTUNITY

### Total Addressable Market
- **Mobile App Development Companies:** 5M+ globally (Statista 2024)
- **QA/Testing Agencies:** 50K+ globally
- **Video Streaming Companies:** 500K+ (including OTT, live, VOD)
- **Research Institutions:** 20K+ universities with CS programs

### Serviceable Addressable Market
- **North America + Europe:** 1M+ companies
- **Mid-market (100-1000 employees):** 500K+ companies
- **Witbe customers who want alternatives:** 500+ companies

### Serviceable Obtainable Market (Year 3)
- 500 paying customers × $5K average annual = **$2.5M ARR**
