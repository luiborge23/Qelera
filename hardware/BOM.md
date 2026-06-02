# Bill of Materials — Qelera Hardware

## Single Device Unit (Android + iOS Pair)

Each unit tests one Android device + one iOS device simultaneously.

| # | Component | Specification | Qty | Unit Cost | Total |
|---|-----------|--------------|-----|-----------|-------|
| 1 | HDMI Capture Card | AVerMedia GC573 (240fps, HDMI 2.0, USB 3.0) | 1 | $199 | $199 |
| 2 | Machine Vision Camera | Basler ace2 acA1920-155um (155fps, global shutter, USB 3.0) | 1 | $599 | $599 |
| 3 | USB Hub | Powered USB 3.0, 8-port (Anker or similar) | 1 | $45 | $45 |
| 4 | IR Blaster | Universal IR sender with USB interface (BroadLink RM4 or similar) | 1 | $25 | $25 |
| 5 | Bluetooth Module | BLE 5.0 USB dongle (TP-Link UB500) | 1 | $15 | $15 |
| 6 | GPIO Controller | Arduino Nano 33 BLE Sense (for hardware trigger sync) | 1 | $35 | $35 |
| 7 | Phone Mounts | Adjustable clamp mounts for phone testing | 2 | $20 | $40 |
| 8 | Cables & Accessories | HDMI cables, USB cables, power adapters | 1 set | $30 | $30 |
| | | | | **SUBTOTAL** | **$988** |

### Per-Device Breakdown

| Device | Capture Method | Cost |
|--------|---------------|------|
| Android (phone/tablet) | HDMI out → AVerMedia GC573 → USB 3.0 | ~$224 |
| iOS (iPhone) | Camera capture → Basler ace2 → USB 3.0 | ~$764 |

Note: The iOS camera capture is more expensive because it requires a dedicated machine vision
camera with global shutter (rolling shutter causes motion artifacts at high FPS).

## Computing Node (Per 4-Device Cluster)

One compute node handles processing for up to 4 device units (8 devices total).

| # | Component | Specification | Qty | Unit Cost | Total |
|---|-----------|--------------|-----|-----------|-------|
| 1 | GPU | NVIDIA RTX 4060 (8GB VRAM, PCIe 4.0 x8) | 1 | $299 | $299 |
| 2 | CPU | AMD Ryzen 5 7600X (6C/12T, 5.3GHz boost) | 1 | $229 | $229 |
| 3 | RAM | 32GB DDR5-5600 (2x16GB) | 1 | $89 | $89 |
| 4 | Storage | 1TB NVMe Gen4 SSD (Samsung 980 Pro or similar) | 1 | $79 | $79 |
| 5 | Motherboard | B650 chipset, ATX, USB 3.2, PCIe 5.0 | 1 | $149 | $149 |
| 6 | PSU | 650W 80+ Gold (Seasonic or Corsair) | 1 | $79 | $79 |
| 7 | Case | Mid-tower with airflow (Fractal Design Define 7) | 1 | $69 | $69 |
| | | | | **SUBTOTAL** | **$993** |

## Full Starter Kit

| Component | Qty | Unit Cost | Total |
|-----------|-----|-----------|-------|
| Device Units (Android + iOS pair) | 2 | $988 | $1,976 |
| Compute Node | 1 | $993 | $993 |
| Network Switch | 8-port Gigabit (TP-Link TL-SG108) | 1 | $50 | $50 |
| Rack Mount Kit (optional) | 1 | $150 | $150 |
| **TOTAL HARDWARE** | | | **$3,169** |

### Cost Comparison with Witbe

| | Qelera (Starter Kit) | Witbe (WitboxOne) | Witbe (Witbox+) |
|--|--------------------------|-------------------|-----------------|
| Hardware | $3,169 (4 devices) | ~$2,500-5,000 | ~$5,000-10,000 |
| Annual Software | $3,588-47,880/yr ($299-3,999/mo) | $50,000-500,000+/yr | $100,000-500,000+/yr |
| Deployment Time | 1 day (self-serve) | 2-4 weeks (professional install) | 4-8 weeks |
| Scalability | Add units as needed | Enterprise contract | Enterprise contract |

## Optional Upgrades

| Component | Upgrade | Additional Cost |
|-----------|---------|----------------|
| GPU | RTX 4070 (12GB VRAM) for larger models | +$200 |
| RAM | 64GB DDR5 for 8+ concurrent devices | +$100 |
| Camera | Basler ace2 USB 3.0 (higher resolution) | +$200 |
| Storage | 2TB NVMe for extended retention | +$70 |
| Network | 10GbE SFP+ upgrade for multi-node | +$150 |
| UPS | APC Back-UPS 1500VA | +$120 |

## Vendor Recommendations

| Component | Vendor | Link |
|-----------|--------|------|
| AVerMedia GC573 | Amazon / Newegg | avermedia.com/product-detail/GC573 |
| Basler ace2 | Basler directly / DigiKey | baslerweb.com |
| Arduino Nano 33 BLE | Arduino.cc / Seeed Studio | arduino.cc |
| RTX 4060 | Newegg / Amazon | nvidia.com/geforce |
| Ryzen 5 7600X | Newegg / Amazon | amd.com |
