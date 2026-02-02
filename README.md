# 🍊 Orange Sentry

[![Status](https://img.shields.io/badge/Status-Pre--Alpha-orange?style=for-the-badge)](https://github.com/JoaoNasMonteiro/Orange-Sentry/milestones)
[![Platform](https://img.shields.io/badge/Platform-Orange_Pi_Zero_3-blue?style=for-the-badge&logo=linux)](http://www.orangepi.org/)

> **A tactical, hardware-controlled IoT Honeypot & Edge NIDS built for hostile networks.**

---

## Introduction

Welcome to Orange Sentry, a Free and Open Source Embedded IoT Honeypot device based around the Orange PI Zero 3 SBC.

It is designed to be a portable, lightweight and highly hackable (in the original sense of the word) embedded IoT Honeypot Device and NIDS.

This word salad means that it is a standalone device that can be easily deployed and (with some pre-configuration) operated mostly without the need for a separate computer to make remote access.

It is also a passion project, and a way for me to study more about all things IoT development, such as:
  1. C and python development,
  2. Systems architecture and design, 
  3. Project management, and 
  4. Embedded hardening and secure development.

**Project Page & DevLogs:** [GitHub Pages](https://joaonasmonteiro.github.io/projects/iot-honeypot/)

## Directory structure
The directory structure is as follows:

The folder assets contains schematics, diagrams, images, and other useful things that may or may not help someone (including future me) understand how this thing is structured and how it operates.

The folder cfig-files is a mostly backup folder for any and all configuration files for important software running on the board, such as cowrie.

The folder code contains all of the actual code of this project, and you can compile the binaries from there.
 
## Key Features 

> Please note that this project is in pre-alpha, and many of the features are not yet implemented.

1. MQTT client integration: the board is able to send alerts and notifications to a broker with authentication.
2. Suricata NIDS to generate alerts and Cowrie SSH/Telnet Honeypot to study attacks on hostile IoT networks
3. Physical buttons and OLED display for HMI

## Architecture

Orange Sentry's architecture is based on a Finite State Machine (FSM) implementation with the following states: 


| Mode | Description |
| :--- | :--- |
| **🔒 Closed Mode** | All network ports are closed via Nftables. Interaction is limited to the physical user interface. |
| **👂 Passive Listen** | Opens ports but silently drops connection attempts, logging the traffic meta-data. |
| **🍯 Honeypot** | Engages the Cowrie (SSH/Telnet) honeypot and Suricata NIDS to generate alerts and capture attacker sessions. |
| **🛠️ Development** | Disengages the honeypot/IDS and allows direct legitimate SSH connections for maintenance. |

Additionally, the code is structured broadly into the Control Pane and the Enforcement Plane (stolen from Zero Trust Security Architectures). The control plane provides control functionality to the device, meanwhile the enforcement plane is called by the control plane to actually execute tasks and change the behaviour of the device.

A good example would be the man controller vs the MQTT client. While the main controller integrates calls upon the client, who is actually tasked with sending the message is the client. 

## The Roadmap

This project follows (loosely) a sprint-based development cycle, where each sprint aims to develop a specific feature or set of features that are kind of self contained. Each sprint is supposed to represent one or two weeks of work.

Alas, I am but learning about each one of the topics covered as I go, and because of that some of this is very much subject to change

### Milestone 0: Pre-Alpha (Infrastructure and Minimal Viable Product)

- [x] Sprint 0: Initial setup (OS, Cowrie, Firewall rules, initial control pane mocking for validation, ETC.)
- [x] Sprint 1: Build System, Cross-Compilation & Toolchain
	- Goal: Create a solid build system with GNU make that is flexible and able to handle:
    1. Vendor libraries such as Paho MQTT C Library, and
    2. Internal util libraries.
	- Tasks:
		- Refactor Makefile stack to acocunt for cross compilation and linking
- [ ] Sprint 2: MQTT Connectivity and initial implementation
	- Goal: Implement a functional pub MQTT client using the Paho C Library and the Mosquitto Broker
	- Tasks:
		- Implement PUB.
		- Implement sending Heartbeats to the broker 
		- Recieve simple commands by subbing to command topics (ex: "change_mode on topic /board/control"). // added to next sprint
- Sprint 2.5: MQTT subscribing and security. Also cleanup
	- Goal: Functional pub/sub client reasonably bug-secure using callbacks and input sanitization
		- Tasks:
			- Implement sub with callbacks
			- Debug and validate MQTT functionality on ARM
			- Mock receiving commands via the MQTT sub input
			- Implement account segregation such that the account that runs the MQTT service isn't root
			- Implement input sanitization on the client both to parse messages to send and commands to receive (important security stuff)
- [ ] Sprint 3: Initial hardware interaction (buttons)
  - Goal: Implement button-press detection with adequate debouncing in a kernel module. Implement some sort of manager service for the hardware implementation.
  - Tasks:
    - Make a button press change the state of the FSM
    - Make the change in the FSM state trigger a log on MQTT
    - Small refactor on the main controller such that it can orchestrate cowrie's functionality according to the FSM state
- [ ] Sprint 4: OLED screen integration
  - Goal: integrate an OLED screen into the program that shows the current state and some actions you can take based on the state. You can select those actions via the buttons and they will meaningfully act on the board's functionality (e.g. changing states or closing current connections within suricata)
  - Tasks:
    - Write a C program that generates the information that is going to be rendered to the screen based on the context and sends it over via a jason on FIFO pipes 
    - Write a small rendering server with python that can read the JSON information, render it appropriately to a framebuffer and send it over to a simple I2C controlled OLED screen
- [ ] Sprint 5: Board security hardening and final controller implementation
  - Goal: make the board as secure as reasonably possible with both Linux security best practices and C Programming secuirity best practices 
    - Tasks:
      - Implement strong MAC via something like AppArmor
      - Make the production version of the board ROFS
      - Implement authentication on the MQTT client
      - Final refactoring of the controller
- [ ] Sprint 6: Final pre-alpha review
  - Goal: Make the project ready for the first alpha deliverables
  - Tasks:
    - Thorough testing of the board, including manual logic validation and automated benchmarking
    - Consolidating documentation and build instructions 
    - Consolidating README, milestone DevLog and first LinkedIn post about the project
    - Consolidating presentation strategy
    
### Milestone 1: Alpha (Device Reliability)
*Focus: Local Persistence, Hardware Watchdog, Thermal Management, security and Crash Recovery.*

### Milestone 2: Beta (Ecosystem)
*Focus: Webapp Backend, Fleet Management (IaC), and Auto-Enrollment.*

## Building from source
TODO

---

*By [JoaoNasMonteiro](https://github.com/JoaoNasMonteiro).*

