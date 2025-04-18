## 🚨 Smart Fire Detection System
An IoT-enabled fire safety solution featuring:
- **STM32-based Sensor Nodes** for real-time monitoring of fire/environmental parameters
- **ESP32 Fire Alarm Control Panel** serving as both gateway and cloud interface
- **Modular Architecture** using C++ Abstract Factory Pattern for flexible sensor management
- **Edge-to-Cloud Integration** with AWS IoT for remote monitoring and alerts

### 📌 Project Overview
- **STM32 Sensor Node** continuously monitor all sensors and communicates with the **ESP32 FACP** via **SPI**.
- **FACP conducts heartbeat checks** - pings the Sensor Node, receiving acknowledgments in normal operation.
- On anomaly, **Sensor Node raises an interrupt**, prompting the FACP to **request detailed sensor readings**.
- **Cloud reporting**: FACP transmits health metrics and emergency events via **MQTT (AWS IoT Core)**.

### 🔧 Key Features
✅ **Modular & Scalable Design**  
&nbsp;🔹 **Abstract Factory Pattern** in C++ for dynamic sensor management.  
&nbsp;🔹 Plug-and-play expandability: Add more Sensor Nodes to the FACP for larger deployments.  
✅ **Multi-Sensor Monitoring (Sensor Node STM32)**  
&nbsp;🔥Fire Detection: Temperature, Smoke, Gas, Flame  
&nbsp;💧Environmental: Humidity, VOC  
&nbsp;♨️Smart Sensing: Ambient Light, Thermal IR  
&nbsp;***(Supports up to 8 sensors per node with configurable thresholds)***  
✅ **Robust Communication Stack**  
🔹 **SPI**:  
  - Heartbeat checks (FACP → Node → FACP)  
  - On-demand sensor data transmission (Node → FACP)     
🔹 **Hardware Interrupt Line**:  
  - Low-latency anomaly alerts (Node → FACP)  
🔹 **UART Debugging**:  
  - Serial logs for sensor status, diagnostics, and development.  
✅ **Fire Alarm Control Panel Node (ESP32)**  
  - **Active Monitoring**: Periodically checks sensor node health via SPI.  
  - **Event-Driven Response**: Instantly reacts to interrupt-based anomaly alerts from sensor nodes.  
  - **Selective Data Fetch**: Requests detailed sensor readings only during critical events.  
  - **Scalable Architecture**: Supports daisy-chaining multiple sensor nodes for large-scale deployments.  
✅ **Edge Processing – Local Intelligence**  
  - **On-Node Detection**: Anomalies are identified at the sensor node level.  
  - **Bandwidth Efficiency**: Raw data stays local; only processed alerts/health stats are transmitted.  
✅ **Cloud Integration – Real-Time Visibility**  
- **MQTT Pub/Sub**: Lightweight AWS IoT Core messaging for live sensor status and emergency alerts.  
- **Remote Dashboard**: Web-based monitoring with historical logs and alert triaging.  
- **OTA Updates**: Firmware/configuration pushed to FACP and nodes via cloud orchestration.  



