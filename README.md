# 🍊 Orange Sentry

[![Status](https://img.shields.io/badge/Status-Pre--Alpha-orange?style=for-the-badge)](https://github.com/JoaoNasMonteiro/Orange-Sentry/milestones)
[![Platform](https://img.shields.io/badge/Platform-Orange_Pi_Zero_3-blue?style=for-the-badge&logo=linux)](http://www.orangepi.org/)

> **A tactical, hardware-controlled IoT Honeypot & Edge NIDS built for hostile networks.**

---

## 📋 The Mission
**Orange Sentry** bridges the gap between high-level threat intelligence and low-level embedded engineering.

It is a portable, standalone security device designed to be dropped into untrusted networks. Unlike standard software honeypots, Orange Sentry treats security as a **physical state**: modes (Honeypot, Passive Listen, Maintenance) are toggled via hardware buttons and orchestrated by a deterministic Finite State Machine (FSM).

**Project Page & DevLogs:** [GitHub Pages](https://joaonasmonteiro.github.io/projects/iot-honeypot/)

## ✨ Key Features (Planned/Implemented)

* **🛡️ Tactical Control Plane:** Security modes are controlled via physical GPIO interrupts.
* **🧠 Deterministic Memory:** Custom **Arena Allocators** (`arena.h`) replace standard `malloc/free` to prevent heap fragmentation in 24/7 operations.
* **🔌 Secure C2:** Telemetry is exfiltrated via MQTT over TLS with mutual authentication.
* **🍯 Hybrid Detection:** Combines **Cowrie** (Interaction Honeypot) and **Suricata** (Signature NIDS) for layered detection.

## 🏗️ Architecture

The system operates on two distinct planes to ensure stability under attack:

| Plane | Responsibility | Tech Stack |
| :--- | :--- | :--- |
| **Control Plane** | The "Brain". Manages FSM, hardware inputs, and orchestration. | **C**, GPIO, MQTT, Arena Allocators |
| **Enforcement Plane** | The "Muscle". Handles packet filtering and trapping. | **Nftables**, Systemd, Cowrie, Suricata |

## 🚀 Getting Started
--- Build Instructions Under Construction ---

## 🗺️ Roadmap
Follows a "Vertical Slice" development strategy.

- [x] **Milestone 0: Infrastructure** (Current Focus)
    - [x] MVP 0: Core Setup & Systemd Integration.
    - [ ] MVP 1.5: Cross-Compilation Toolchain.
    - [ ] MVP 2: MQTT Control Plane.
- [ ] **Milestone 1: Alpha (Reliability)**
    - Hardware Watchdog & Thermal Management.
    - Local Persistence.
- [ ] **Milestone 2: Beta (Ecosystem)**
    - Webapp Backend & Auto-enrollment.

## 📜 License
Distributed under the GPLv3 License. See `LICENSE.md` for more information.

---
*Crafted by [JoaoNasMonteiro](https://github.com/JoaoNasMonteiro).*
