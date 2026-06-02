# Go-to-Market Strategy — Qelera

## 1. Market Positioning

### Value Proposition
> "Measure mobile app performance at sub-10ms precision — without enterprise pricing or vendor lock-in. Qelera gives you real-device, black-box QoE testing with an API-first approach that developers love."

### Positioning Statement
For mobile app developers and QA teams who need fast, actionable QoE data: Qelera is the developer-first, API-native QoE testing platform that delivers sub-10ms precision on real devices — at a fraction of Witbe's cost, with open-source flexibility.

### Key Differentiators
1. **Mobile-first focus** (Witbe is TV/STB centric)
2. **Transparent pricing** ($299/mo vs. $50K+/yr enterprise contracts)
3. **API-native** (developer-friendly REST API, not a GUI-only tool)
4. **Open-source core** (build trust, attract community)
5. **Hardware-agnostic** (works with capture cards OR cameras)
6. **Research-backed precision** (sub-10ms using PhD-level CV)

---

## 2. Pricing Strategy

| Tier | Price | Devices | Features | Target |
|------|-------|---------|----------|--------|
| **Hobby** | Free | 1 | Core detection, 100 tests/mo, community support | Developers, students |
| **Startup** | $299/mo | 2 | All metrics, 1K tests/mo, email support | Early-stage startups |
| **Pro** | $999/mo | 4 | All metrics, 10K tests/mo, priority support | Mid-market teams |
| **Team** | $2,999/mo | 10 | All metrics, unlimited tests, API access, SLA | Growing companies |
| **Enterprise** | Custom | 50+ | All features, custom integrations, dedicated support, SOC 2 | Large organizations |

### Hardware Pricing (Optional)
- **Qelera Kit (4 devices):** $3,169 (one-time)
- **Qelera Cloud (compute hosted):** $499/mo per cluster

### Why This Works
- **Freemium model** drives adoption and community growth
- **Self-serve pricing** eliminates sales friction
- **Hardware-optional** reduces barrier to entry
- **Scales with customer** — they pay as they grow

---

## 3. Customer Acquisition Channels

### Channel 1: Open-Source Community (Months 1-6)
- Release core detection pipeline on GitHub
- Publish technical blog posts with research-backed implementations
- Contribute to OpenCV, YOLO, and computer vision communities
- Build reputation as the "go-to" for QoE testing research
- **Goal:** 1,000 GitHub stars, 100 active users by Month 6

### Channel 2: Content Marketing (Months 1-12)
- Publish QoE benchmark reports (e.g., "Top 10 Mobile Apps: QoE Analysis")
- Create YouTube tutorials on video processing for QoE
- Speak at conferences (NAB, IBC, QCon, DevOpsDays)
- Write guest posts for Medium, Dev.to, Towards Data Science
- **Goal:** 10K monthly website visitors by Month 12

### Channel 3: Product-Led Growth (Months 6-12)
- Free tier with generous limits (1 device, 100 tests/mo)
- In-app upgrade prompts when limits are hit
- Referral program (give 1 month free for each referral)
- Integration marketplace (Slack, GitHub, Jira, PagerDuty)
- **Goal:** 500 free users → 50 paying customers by Month 12

### Channel 4: Sales-Led (Months 9-18)
- Target companies currently using Witbe
- Offer migration program (30-day free trial, free setup)
- Competitive battle cards (Witbe comparison sheets)
- Partner with QA agencies and consulting firms
- **Goal:** 10 enterprise customers by Month 18

---

## 4. Competitive Attack Plan

### Against Witbe
1. **Price:** "Witbe costs $50K+/yr. Qelera starts at $299/mo. Same precision, 10x cheaper."
2. **Mobile:** "Witbe is built for TVs. We're built for mobile apps — your actual product."
3. **Speed:** "Deploy in hours, not weeks. Self-serve setup, no sales call required."
4. **Openness:** "Our core is open-source. No vendor lock-in. Your data, your code."
5. **API:** "Integrate with your CI/CD pipeline. Not a GUI-only tool."

### Against stb-tester
1. **Mobile:** "stb-tester doesn't support mobile. We do."
2. **AI:** "We add ML-powered analysis. They're purely rule-based."
3. **Cloud:** "We offer a hosted platform. They're on-premises only."

### Against Mux
1. **Black-Box:** "Mux requires in-app instrumentation. We test on real devices — no code changes needed."
2. **Device Control:** "Mux can only monitor. We can test, automate, and control."
3. **Complementary:** "Use Qelera for device-level testing + Mux for in-app metrics."

---

## 5. Revenue Projections

| Year | Customers | ARR | Hardware Sales | Total Revenue |
|------|-----------|-----|----------------|---------------|
| Year 1 | 25 | $150K | $50K | $200K |
| Year 2 | 100 | $750K | $200K | $950K |
| Year 3 | 300 | $2.5M | $500K | $3.0M |
| Year 4 | 600 | $6.0M | $800K | $6.8M |
| Year 5 | 1,000 | $12.5M | $1.2M | $13.7M |

### Unit Economics (Pro Tier)
- **ACV:** $11,988 ($999/mo × 12)
- **CAC:** $2,000 (content + sales)
- **LTV:** $35,000 (3-year average lifespan, 10% churn)
- **LTV:CAC Ratio:** 17.5:1 (excellent)

---

## 6. Risks & Mitigations

| Risk | Probability | Impact | Mitigation |
|------|------------|--------|------------|
| Witbe lowers pricing | Medium | High | Differentiate on mobile focus and API |
| Hardware supply chain | Low | Medium | Qualify 2+ suppliers per component |
| AI agent reliability | Medium | High | Start rule-based, add AI as fallback |
| Customer acquisition cost high | High | Medium | Focus on self-serve, virality through OSS |
| Technical debt from OSS | Medium | Medium | Clear OSS/commercial separation |
| Market education needed | High | Medium | Invest in content, benchmarks, tutorials |
