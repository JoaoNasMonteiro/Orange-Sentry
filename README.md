# 🍊 Orange Sentry

[![Status](https://img.shields.io/badge/Status-Pre--Alpha-orange?style=for-the-badge)](https://github.com/JoaoNasMonteiro/Orange-Sentry/milestones)
[![Platform](https://img.shields.io/badge/Platform-Orange_Pi_Zero_3-blue?style=for-the-badge&logo=linux)](http://www.orangepi.org/)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)
[![Build](https://img.shields.io/badge/Build-Passing-brightgreen?style=for-the-badge)]()

> **A tactical, hardware-controlled IoT Honeypot & Edge NIDS built for hostile networks.**

---

## 📋 The Mission
**Orange Sentry** bridges the gap between high-level threat intelligence and low-level embedded engineering.

It is a portable, standalone security device designed to be dropped into untrusted networks. Unlike standard software honeypots, Orange Sentry treats security as a **physical state**: modes (Honeypot, Passive Listen, Maintenance) are toggled via hardware buttons and orchestrated by a deterministic Finite State Machine (FSM).

**Project Page & DevLogs:** [YourWebsiteLink Here](https://your-website-link)

## ✨ Key Features (Planned/Implemented)

* **🛡️ Tactical Control Plane:** Security modes are controlled via physical GPIO interrupts, not just SSH config files.
* **🧠 Deterministic Memory:** Custom **Arena Allocators** (`arena.h`) replace standard `malloc/free` to prevent heap fragmentation in 24/7 operations.
* **🔌 Secure C2:** Telemetry is exfiltrated via MQTT over TLS with mutual authentication.
* **🍯 Hybrid Detection:** Combines **Cowrie** (Interaction Honeypot) and **Suricata** (Signature NIDS) for layered detection.
* **🏗️ Cross-Platform Build:** Development environment decoupled from hardware via a robust cross-compilation toolchain.

## 🏗️ Architecture

The system operates on two distinct planes to ensure stability under attack:

| Plane | Responsibility | Tech Stack |
| :--- | :--- | :--- |
| **Control Plane** | The "Brain". Manages FSM, hardware inputs, and orchestration. | **C**, GPIO, MQTT, Arena Allocators |
| **Enforcement Plane** | The "Muscle". Handles packet filtering and trapping. | **Nftables**, Systemd, Cowrie, Suricata |

## 🚀 Getting Started

### Prerequisites
* **Host:** Linux (x86_64) for cross-compilation.
* **Target:** Orange Pi Zero 3 (running DietPi/Armbian).
* **Dependencies:** `build-essential`, `gcc-aarch64-linux-gnu`, `mosquitto-dev`.

### Build Instructions
This project uses a custom Makefile to handle cross-compilation.

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/JoaoNasMonteiro/Orange-Sentry.git](https://github.com/JoaoNasMonteiro/Orange-Sentry.git)
    cd Orange-Sentry
    ```

2.  **Build for Host (Testing/Mocked Hardware):**
    Runs the Control Plane logic on your PC using mocked GPIO/Display drivers.
    ```bash
    make host
    ./build/orange_sentry_host
    ```

3.  **Build for Target (Production):**
    Compiles the binary for the Orange Pi ARM architecture.
    ```bash
    make cross
    # Output: ./build/orange_sentry_arm
    ```

4.  **Deploy:**
    ```bash
    scp ./build/orange_sentry_arm user@192.168.x.x:~/bin/
    ```

## 🗺️ Roadmap
We follow a "Vertical Slice" development strategy.

- [x] **Milestone 0: Infrastructure** (Current Focus)
    - [x] MVP 0: Core Setup & Systemd Integration.
    - [ ] MVP 1.5: Cross-Compilation Toolchain.
    - [ ] MVP 2: MQTT Control Plane.
- [ ] **Milestone 1: Alpha (Reliability)**
    - Hardware Watchdog & Thermal Management.
    - Local Persistence.
- [ ] **Milestone 2: Beta (Ecosystem)**
    - Webapp Backend & Auto-enrollment.

## 🤝 Philosophy
> "If you can buy the part off-the-shelf, you don't make the part—**unless you want to learn how to make the part.**"

This project is an exploration of **Embedded Linux Internals**. We prioritize understanding *why* things work (building custom drivers, memory arenas) over taking shortcuts.

## 📜 License
Distributed under the MIT License. See `LICENSE` for more information.

---
*Crafted by [JoaoNasMonteiro](https://github.com/JoaoNasMonteiro).*