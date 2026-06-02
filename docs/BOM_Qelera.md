# VISIONTRACK — BILL OF MATERIALS (BOM)

**Version:** 1.0  
**Date:** May 28, 2026  
**Author:** Qelera Product Team  
**Status:** Draft for Review  

---

## 1. HARDWARE COMPONENTS

### 1.1 Edge Capture Hardware

| Item | Component | Specifications | Quantity | Unit Cost | Total Cost |
|------|-----------|----------------|----------|-----------|------------|
| HW-001 | AVerMedia GC573 | USB 3.0 Capture Card, 4K@60fps, HDR10 | 100 | $149.00 | $14,900.00 |
| HW-002 | Basler ace2 | 2MP USB3 Camera, 60fps, Global Shutter | 100 | $299.00 | $29,900.00 |
| HW-003 | Arduino Nano 33 BLE | Bluetooth 5.0, 6-axis IMU, 3.3V | 200 | $25.00 | $5,000.00 |
| HW-004 | Raspberry Pi 5 | 8GB RAM, WiFi 6, Bluetooth 5.0, USB 3.0 | 50 | $80.00 | $4,000.00 |
| HW-005 | Intel NUC 13 Pro | i7-1360P, 32GB RAM, 1TB NVMe, WiFi 6E | 20 | $1,200.00 | $24,000.00 |
| HW-006 | USB-C Hub | 7-in-1, HDMI 4K, USB 3.0, Ethernet, SD | 100 | $39.00 | $3,900.00 |
| HW-007 | HDMI Cable | 2.1, 8K@60fps, 48Gbps, 2m | 100 | $19.00 | $1,900.00 |
| HW-008 | USB-C Cable | 3.2 Gen 2, 10Gbps, 2m | 100 | $15.00 | $1,500.00 |
| HW-009 | Power Supply | 12V/5A, Universal, Multiple Tips | 100 | $25.00 | $2,500.00 |
| HW-010 | Mounting Kit | Adjustable Arm, VESA Mount, Cable Management | 100 | $45.00 | $4,500.00 |

**Subtotal Hardware:** $92,100.00

### 1.2 Test Devices

| Item | Component | Specifications | Quantity | Unit Cost | Total Cost |
|------|-----------|----------------|----------|-----------|------------|
| TD-001 | Samsung Galaxy S24 | Android 14, Exynos 2400, 12GB RAM | 10 | $899.00 | $8,990.00 |
| TD-002 | Google Pixel 8 | Android 14, Tensor G3, 8GB RAM | 10 | $699.00 | $6,990.00 |
| TD-003 | iPhone 15 Pro | iOS 17, A17 Pro, 8GB RAM | 10 | $999.00 | $9,990.00 |
| TD-004 | iPad Pro 12.9" | iOS 17, M2, 8GB RAM | 5 | $1,099.00 | $5,495.00 |
| TD-005 | Samsung Galaxy Tab S9 | Android 14, Snapdragon 8 Gen 2, 8GB RAM | 5 | $799.00 | $3,995.00 |
| TD-006 | Amazon Fire HD 10 | Android 12, MediaTek Helio G80, 4GB RAM | 10 | $149.00 | $1,490.00 |

**Subtotal Test Devices:** $36,950.00

### 1.3 Network Equipment

| Item | Component | Specifications | Quantity | Unit Cost | Total Cost |
|------|-----------|----------------|----------|-----------|------------|
| NE-001 | Ubiquiti UniFi 6E | WiFi 6E, 2.5Gbps, PoE+, Enterprise | 5 | $399.00 | $1,995.00 |
| NE-002 | Cisco Catalyst 9200 | 48-Port PoE+, 1Gbps, Managed Switch | 2 | $2,500.00 | $5,000.00 |
| NE-003 | 5G Router | Verizon 5G Home Internet, 1Gbps | 2 | $400.00 | $800.00 |
| NE-004 | Ethernet Cable | Cat6, 10Gbps, 10m, Shielded | 50 | $15.00 | $750.00 |
| NE-005 | Network Monitor | Wireshark License, PRTG Monitor | 1 | $500.00 | $500.00 |

**Subtotal Network:** $9,045.00

### 1.4 Hardware BOM Summary

| Category | Quantity | Total Cost |
|----------|----------|------------|
| Edge Capture Hardware | 1,070 items | $92,100.00 |
| Test Devices | 50 devices | $36,950.00 |
| Network Equipment | 60 items | $9,045.00 |
| **Total Hardware** | **1,180 items** | **$138,095.00** |

---

## 2. SOFTWARE COMPONENTS

### 2.1 Open Source Software (Free)

| Component | Version | License | Purpose |
|-----------|---------|---------|---------|
| TensorFlow Lite | 2.15+ | Apache 2.0 | Edge AI inference |
| YOLOv8 | 8.0+ | AGPL 3.0 | Real-time object detection |
| Apache Kafka | 3.6+ | Apache 2.0 | Event streaming |
| Apache Flink | 1.18+ | Apache 2.0 | Stream processing |
| Redis | 7.2+ | BSD 3-Clause | Cache, streams, pub/sub |
| PostgreSQL | 16+ | PostgreSQL | Primary database |
| TimescaleDB | 2.14+ | Apache 2.0 | Time-series extension |
| Elasticsearch | 8.11+ | SSPL | Full-text search |
| Kong | 3.5+ | Apache 2.0 | API Gateway |
| Celery | 5.3+ | BSD 3-Clause | Task scheduling |
| FastAPI | 0.104+ | MIT | Backend framework |
| React | 18+ | MIT | Frontend framework |
| WebSocket | RFC 6455 | IETF | Real-time communication |
| OpenSSL | 3.1+ | Apache 2.0 | TLS/SSL encryption |
| JWT | RFC 7519 | IETF | Authentication tokens |
| OAuth 2.0 | RFC 6749 | IETF | Authorization framework |
| QUIC | RFC 9000 | IETF | Transport protocol |
| gRPC | 1.59+ | Apache 2.0 | Remote procedure calls |
| Protobuf | 24+ | BSD 3-Clause | Data serialization |
| Avro | 1.11+ | Apache 2.0 | Schema registry |
| MLflow | 2.9+ | Apache 2.0 | Model versioning |
| ELK Stack | 8.11+ | SSPL | Logging and monitoring |
| Grafana | 10+ | Apache 2.0 | Dashboards and visualization |
| Prometheus | 2.47+ | Apache 2.0 | Metrics collection |
| Terraform | 1.6+ | BUSL 1.1 | Infrastructure as code |
| Docker | 24+ | Apache 2.0 | Containerization |
| Kubernetes | 1.28+ | Apache 2.0 | Orchestration |
| Helm | 3.13+ | Apache 2.0 | Package management |
| Istio | 1.20+ | Apache 2.0 | Service mesh |
| Envoy | 1.28+ | Apache 2.0 | Proxy and load balancer |
| Vault | 1.15+ | BUSL 1.1 | Secret management |
| Consul | 1.17+ | BUSL 1.1 | Service discovery |
| Nomad | 1.7+ | BUSL 1.1 | Job scheduler |
| Packer | 1.9+ | BUSL 1.1 | Image building |
| Vagrant | 2.4+ | BUSL 1.1 | Development environments |
| Ansible | 2.16+ | GPL 3.0 | Configuration management |
| Puppet | 8+ | Apache 2.0 | Configuration management |
| Chef | 18+ | Apache 2.0 | Configuration management |
| Jenkins | 2.426+ | MIT | CI/CD pipeline |
| GitHub Actions | - | Proprietary | CI/CD pipeline |
| GitLab CI/CD | - | Proprietary | CI/CD pipeline |
| SonarQube | 10+ | LGPL 3.0 | Code quality |
| OWASP ZAP | 2.14+ | Apache 2.0 | Security scanning |
| Burp Suite | Community | Proprietary | Security testing |
| Nmap | 7.94+ | GPL 3.0 | Network scanning |
| Metasploit | 6.3+ | BSD 3-Clause | Penetration testing |
| Wireshark | 4.2+ | GPL 2.0 | Network analysis |
| tcpdump | 4.99+ | BSD 3-Clause | Packet capture |
| iptables | 1.8+ | GPL 2.0 | Firewall management |
| ufw | 0.36+ | GPL 3.0 | Firewall management |
| fail2ban | 0.11+ | GPL 3.0 | Intrusion prevention |
| rsyslog | 8.23+ | GPL 3.0 | Log management |
| logstash | 8.11+ | Apache 2.0 | Log processing |
| filebeat | 8.11+ | Apache 2.0 | Log shipping |
| metricbeat | 8.11+ | Apache 2.0 | Metrics shipping |
| packetbeat | 8.11+ | Apache 2.0 | Network metrics |
| heartbeat | 8.11+ | Apache 2.0 | Uptime monitoring |
| elastic-agent | 8.11+ | Apache 2.0 | Centralized agent |
| telegraf | 1.29+ | MIT | Metrics collection |
| influxdb | 2.7+ | MIT | Time-series database |
| grafana-alloy | 1.0+ | Apache 2.0 | Metrics pipeline |
| promtail | 2.9+ | Apache 2.0 | Log shipping |
| loki | 2.9+ | AGPL 3.0 | Log aggregation |
| cortex | 1.13+ | Apache 2.0 | Metrics federation |
| thanos | 0.33+ | Apache 2.0 | Metrics long-term storage |
| VictoriaMetrics | 1.97+ | Apache 2.0 | Time-series database |
| graphite | 1.1+ | Apache 2.0 | Metrics storage |
| carbon | 1.1+ | Apache 2.0 | Metrics ingestion |
| whisper | 1.1+ | Apache 2.0 | Metrics storage |
| statsd | 0.5+ | MIT | Metrics aggregation |
| dogstatsd | 5.0+ | Proprietary | Metrics aggregation |
| newrelic | 8.0+ | Proprietary | APM |
| datadog | 7.0+ | Proprietary | APM |
| splunk | 9.0+ | Proprietary | SIEM |
| elasticsearch | 8.11+ | SSPL | Search engine |
| kibana | 8.11+ | Apache 2.0 | Visualization |
| grafana | 10+ | Apache 2.0 | Dashboards |
| prometheus | 2.47+ | Apache 2.0 | Metrics |
| alertmanager | 0.26+ | Apache 2.0 | Alerting |
| node-exporter | 1.7+ | Apache 2.0 | Host metrics |
| cadvisor | 0.47+ | Apache 2.0 | Container metrics |
| kube-state-metrics | 2.11+ | Apache 2.0 | K8s metrics |
| metrics-server | 0.6+ | Apache 2.0 | K8s metrics |
| vertical-pod-autoscaler | 1.0+ | Apache 2.0 | K8s autoscaling |
| horizontal-pod-autoscaler | 1.0+ | Apache 2.0 | K8s autoscaling |
| cluster-autoscaler | 1.28+ | Apache 2.0 | K8s autoscaling |
| keda | 2.13+ | Apache 2.0 | Event-driven autoscaling |
| karpenter | 0.32+ | Apache 2.0 | K8s node provisioning |
| argocd | 2.9+ | Apache 2.0 | GitOps |
| flux | 2.2+ | Apache 2.0 | GitOps |
| helm | 3.13+ | Apache 2.0 | Package management |
| kustomize | 5.0+ | Apache 2.0 | K8s configuration |
| istio | 1.20+ | Apache 2.0 | Service mesh |
| linkerd | 1.14+ | Apache 2.0 | Service mesh |
| consul | 1.17+ | BUSL 1.1 | Service discovery |
| etcd | 3.5+ | Apache 2.0 | Key-value store |
| consul-template | 0.55+ | BUSL 1.1 | Configuration templating |
| nomad | 1.7+ | BUSL 1.1 | Job scheduler |
| packer | 1.9+ | BUSL 1.1 | Image building |
| vagrant | 2.4+ | BUSL 1.1 | Development environments |
| ansible | 2.16+ | GPL 3.0 | Configuration management |
| puppet | 8+ | Apache 2.0 | Configuration management |
| chef | 18+ | Apache 2.0 | Configuration management |
| saltstack | 3005+ | Apache 2.0 | Configuration management |
| terraform | 1.6+ | BUSL 1.1 | Infrastructure as code |
| pulumi | 3.0+ | Apache 2.0 | Infrastructure as code |
| cdk | 2.0+ | Apache 2.0 | Infrastructure as code |
| cloudformation | - | Proprietary | Infrastructure as code |
| serverless | 3.0+ | MIT | Serverless framework |
| aws-sam | 1.0+ | Apache 2.0 | AWS serverless |
| gcp-cloud-run | - | Proprietary | GCP serverless |
| azure-functions | - | Proprietary | Azure serverless |
| openfaas | 1.0+ | Apache 2.0 | Serverless framework |
| knative | 1.11+ | Apache 2.0 | Serverless on K8s |
| istio | 1.20+ | Apache 2.0 | Service mesh |
| linkerd | 1.14+ | Apache 2.0 | Service mesh |
| consul | 1.17+ | BUSL 1.1 | Service discovery |
| vault | 1.15+ | BUSL 1.1 | Secret management |
| consul-template | 0.55+ | BUSL 1.1 | Configuration templating |
| nomad | 1.7+ | BUSL 1.1 | Job scheduler |
| packer | 1.9+ | BUSL 1.1 | Image building |
| vagrant | 2.4+ | BUSL 1.1 | Development environments |

### 2.2 Commercial Software

| Component | License Type | Annual Cost | Purpose |
|-----------|--------------|-------------|---------|
| Datadog APM | Enterprise | $2,500 | Application performance monitoring |
| New Relic APM | Enterprise | $2,000 | Application performance monitoring |
| Splunk Enterprise | Enterprise | $5,000 | SIEM and log management |
| Burp Suite Professional | Professional | $449 | Security testing |
| OWASP ZAP | Enterprise | $1,000 | Security scanning |
| SonarQube Enterprise | Enterprise | $2,000 | Code quality |
| PagerDuty | Enterprise | $1,500 | Incident management |
| Slack Business | Business | $1,200 | Team communication |
| Notion Enterprise | Enterprise | $1,000 | Documentation and knowledge base |
| Figma Organization | Organization | $1,500 | Design and collaboration |

**Subtotal Commercial Software:** $17,149.00/year

### 2.3 Software BOM Summary

| Category | Annual Cost |
|----------|-------------|
| Open Source Software | $0.00 |
| Commercial Software | $17,149.00 |
| **Total Software** | **$17,149.00/year** |

---

## 3. CLOUD INFRASTRUCTURE

### 3.1 AWS Infrastructure (Primary)

| Service | Configuration | Monthly Cost | Annual Cost |
|---------|---------------|--------------|-------------|
| EC2 | m5.2xlarge (4 vCPU, 16GB RAM) x 10 | $1,200.00 | $14,400.00 |
| ECS | Fargate 4 vCPU, 8GB RAM x 20 | $800.00 | $9,600.00 |
| RDS | db.r6g.xlarge (4 vCPU, 32GB RAM) x 2 | $1,600.00 | $19,200.00 |
| ElastiCache | r6g.large (2 vCPU, 16GB RAM) x 3 | $600.00 | $7,200.00 |
| S3 | 100TB storage, 1M requests/month | $2,300.00 | $27,600.00 |
| CloudFront | 1TB transfer/month | $90.00 | $1,080.00 |
| Route 53 | 10 hosted zones | $0.50 | $6.00 |
| API Gateway | 1M requests/month | $350.00 | $4,200.00 |
| Lambda | 1M requests/month | $20.00 | $240.00 |
| Kinesis | 100MB/second ingestion | $500.00 | $6,000.00 |
| SQS | 1M messages/month | $40.00 | $480.00 |
| SNS | 1M notifications/month | $5.00 | $60.00 |
| CloudWatch | 100K log events/day | $50.00 | $600.00 |
| WAF | 1 web ACL | $10.00 | $120.00 |
| Shield Advanced | 10 resources | $3,000.00 | $36,000.00 |
| GuardDuty | 100 instances | $500.00 | $6,000.00 |
| Macie | 100GB scanned/month | $100.00 | $1,200.00 |
| Secrets Manager | 100 secrets | $5.00 | $60.00 |
| Certificate Manager | 10 certificates | $0.00 | $0.00 |
| **Subtotal AWS** | | **$8,230.50** | **$98,766.00/year** |

### 3.2 GCP Infrastructure (Secondary/DR)

| Service | Configuration | Monthly Cost | Annual Cost |
|---------|---------------|--------------|-------------|
| Compute Engine | n2-standard-4 (4 vCPU, 16GB RAM) x 10 | $1,000.00 | $12,000.00 |
| Cloud Run | 4 vCPU, 8GB RAM x 20 | $600.00 | $7,200.00 |
| Cloud SQL | db-custom-4-16384 (4 vCPU, 32GB RAM) x 2 | $1,400.00 | $16,800.00 |
| Memorystore | redis-standard-2 (2 vCPU, 16GB RAM) x 3 | $500.00 | $6,000.00 |
| Cloud Storage | 100TB storage, 1M requests/month | $2,000.00 | $24,000.00 |
| Cloud CDN | 1TB transfer/month | $80.00 | $960.00 |
| Cloud DNS | 10 hosted zones | $0.50 | $6.00 |
| Cloud Load Balancing | 1 global LB | $250.00 | $3,000.00 |
| Cloud Functions | 1M invocations/month | $10.00 | $120.00 |
| Pub/Sub | 100MB/second ingestion | $400.00 | $4,800.00 |
| Cloud Monitoring | 100K log events/day | $40.00 | $480.00 |
| Security Command Center | 100 resources | $400.00 | $4,800.00 |
| Data Loss Prevention | 100GB scanned/month | $80.00 | $960.00 |
| Secret Manager | 100 secrets | $5.00 | $60.00 |
| Certificate Manager | 10 certificates | $0.00 | $0.00 |
| **Subtotal GCP** | | **$6,765.50** | **$81,186.00/year** |

### 3.3 Cloud Infrastructure Summary

| Provider | Monthly Cost | Annual Cost |
|----------|--------------|-------------|
| AWS (Primary) | $8,230.50 | $98,766.00 |
| GCP (Secondary/DR) | $6,765.50 | $81,186.00 |
| **Total Cloud** | **$15,000.00** | **$179,952.00/year** |

---

## 4. TOTAL BOM SUMMARY

### 4.1 One-Time Costs

| Category | Cost |
|----------|------|
| Hardware | $138,095.00 |
| Software (Commercial) | $17,149.00 |
| **Total One-Time** | **$155,244.00** |

### 4.2 Recurring Costs (Annual)

| Category | Annual Cost |
|----------|-------------|
| Cloud Infrastructure | $179,952.00 |
| Commercial Software | $17,149.00 |
| **Total Recurring** | **$197,101.00/year** |

### 4.3 Total Cost of Ownership (TCO) — Year 1

| Category | Cost |
|----------|------|
| One-Time Costs | $155,244.00 |
| Recurring Costs (Year 1) | $197,101.00 |
| **Total Year 1 TCO** | **$352,345.00** |

### 4.4 Total Cost of Ownership (TCO) — 3 Years

| Category | Cost |
|----------|------|
| One-Time Costs | $155,244.00 |
| Recurring Costs (3 Years) | $591,303.00 |
| **Total 3-Year TCO** | **$746,547.00** |

---

## 5. COST OPTIMIZATION STRATEGIES

### 5.1 Cloud Cost Optimization

| Strategy | Potential Savings | Implementation |
|----------|-------------------|----------------|
| Reserved Instances (1-year) | 30-40% | Commit to 1-year RIs for stable workloads |
| Spot Instances | 60-90% | Use for batch processing and non-critical workloads |
| Auto-scaling | 20-30% | Scale down during off-peak hours |
| Data lifecycle policies | 10-20% | Archive old data to cheaper storage |
| Right-sizing | 15-25% | Regularly review and adjust instance sizes |
| Committed use discounts | 20-30% | Commit to 1-year CUDs for stable workloads |
| Multi-cloud strategy | 10-15% | Use cheapest provider for each workload |
| Serverless migration | 20-40% | Migrate stateless workloads to serverless |

### 5.2 Hardware Cost Optimization

| Strategy | Potential Savings | Implementation |
|----------|-------------------|----------------|
| Bulk purchasing | 10-20% | Buy in bulk for better pricing |
| Refurbished equipment | 20-30% | Use certified refurbished hardware |
| Open-source alternatives | 0% | Already using open-source software |
| Cloud hardware rental | 10-15% | Rent hardware instead of buying |
| Shared infrastructure | 15-25% | Share infrastructure across teams |

### 5.3 Software Cost Optimization

| Strategy | Potential Savings | Implementation |
|----------|-------------------|----------------|
| Open-source alternatives | 50-100% | Replace commercial software with open-source |
| Community editions | 30-50% | Use community editions where possible |
| Volume discounts | 10-20% | Negotiate volume discounts with vendors |
| Annual vs monthly | 10-15% | Pay annually for discounts |
| Free tiers | 100% | Use free tiers for development and testing |

---

## 6. REVENUE PROJECTIONS

### 6.1 Pricing Tiers

| Tier | Price/Month | Features | Target Customers |
|------|-------------|----------|------------------|
| Starter | $99 | 10 devices, Basic QoE, Email support | Small startups |
| Professional | $499 | 100 devices, Advanced QoE, Priority support | Medium businesses |
| Enterprise | $1,999 | 1,000 devices, Full QoE, Dedicated support | Large enterprises |
| Custom | Custom | Unlimited devices, Custom features, SLA | Fortune 500 |

### 6.2 Revenue Projections

| Year | Customers | ARR | Gross Margin | Net Profit |
|------|-----------|-----|--------------|------------|
| Year 1 | 20 | $240,000 | 60% | -$112,000 |
| Year 2 | 50 | $600,000 | 70% | $120,000 |
| Year 3 | 100 | $1,200,000 | 75% | $420,000 |
| Year 4 | 200 | $2,400,000 | 80% | $1,080,000 |
| Year 5 | 500 | $6,000,000 | 85% | $3,600,000 |

### 6.3 Break-Even Analysis

| Metric | Value |
|--------|-------|
| Fixed Costs/Year | $197,101 |
| Variable Costs/Customer | $100/year |
| Average Revenue/Customer | $12,000/year |
| Contribution Margin/Customer | $11,900/year |
| Break-Even Customers | 17 customers |

---

## 7. RISK-ADJUSTED BOM

### 7.1 Contingency Budget

| Risk | Contingency % | Contingency Amount |
|------|---------------|-------------------|
| Hardware delays | 10% | $13,809.50 |
| Cloud cost overruns | 20% | $35,990.40 |
| Software license increases | 15% | $2,572.35 |
| Regulatory compliance | 10% | $15,524.40 |
| **Total Contingency** | **14.5%** | **$67,896.65** |

### 7.2 Total Risk-Adjusted Budget

| Category | Amount |
|----------|--------|
| Base TCO (Year 1) | $352,345.00 |
| Contingency | $67,896.65 |
| **Total Risk-Adjusted Budget** | **$420,241.65** |

---

## 8. APPENDIX

### 8.1 Hardware Specifications

**AVerMedia GC573:**
- USB 3.0 capture card
- 4K@60fps, 1080p@120fps
- HDR10 support
- Low latency mode (<10ms)
- Compatible with Windows, macOS, Linux

**Basler ace2:**
- 2MP Sony IMX250 sensor
- USB 3.0 interface
- 60fps at 1920x1080
- Global shutter
- GigE Vision, USB3 Vision compliant

**Arduino Nano 33 BLE:**
- ARM Cortex-M4F processor
- Bluetooth 5.0
- 6-axis IMU (accelerometer, gyroscope)
- 3.3V I/O
- USB-C interface

**Raspberry Pi 5:**
- Broadcom BCM2712 (Quad-core Cortex-A76)
- 8GB LPDDR4X RAM
- WiFi 6 (802.11ax)
- Bluetooth 5.0
- USB 3.0 ports
- PCIe 2.0 x1 interface

**Intel NUC 13 Pro:**
- Intel Core i7-1360P (12 cores, 16 threads)
- 32GB LPDDR5 RAM
- 1TB NVMe SSD
- WiFi 6E (802.11ax)
- Bluetooth 5.3
- Thunderbolt 4

### 8.2 Software Licenses

**Apache 2.0:** Allows commercial use, modification, distribution, and patent use. Requires preservation of copyright and license notices.

**MIT:** Allows commercial use, modification, distribution, and private use. Requires preservation of copyright and license notices.

**BSD 3-Clause:** Allows commercial use, modification, distribution, and private use. Requires preservation of copyright and license notices. Does not use contributors' names for endorsement.

**GPL 3.0:** Requires derivative works to be licensed under GPL 3.0. Allows commercial use, modification, distribution, and private use.

**AGPL 3.0:** Same as GPL 3.0, but also requires source code disclosure if the software is used over a network.

**SSPL:** Server Side Public License. Requires source code disclosure if the software is offered as a service.

**BUSL 1.1:** Business Source License. Allows use, modification, and distribution for non-production purposes. Requires commercial license for production use.

### 8.3 References

1. AWS Pricing Calculator — https://calculator.aws
2. GCP Pricing Calculator — https://cloud.google.com/products/calculator
3. Hardware specifications from manufacturer websites
4. Software license information from official documentation
5. Market research on QoE testing tools and pricing

---

**END OF BOM**
