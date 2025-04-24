## 🚨 Smart Fire Detection System
An IoT-enabled fire safety solution featuring:
- **STM32-based Sensor Nodes** for real-time monitoring of fire/environmental parameters
- **ESP32 Fire Alarm Control Panel** serving as both gateway and cloud interface
- **Modular Architecture** using C++ Abstract Factory Pattern for flexible sensor management
- **Edge-to-Cloud Integration** with AWS IoT for remote monitoring and alerts
---
### 📌 Project Overview
- **STM32 Sensor Node** continuously monitor all sensors and communicates with the **ESP32 FACP** via **SPI**.
- **FACP conducts heartbeat checks** - pings the Sensor Node, receiving acknowledgments in normal operation.
- On anomaly, **Sensor Node raises an interrupt**, prompting the FACP to **request detailed sensor readings**.
- **Cloud reporting**: FACP transmits health metrics and emergency events via **MQTT (AWS IoT Core)**.
---
### 🔧 Key Features
✅ **Modular & Scalable Design**  
&nbsp;&nbsp;&nbsp;• **Abstract Factory Pattern** in C++ for dynamic sensor management.  
&nbsp;&nbsp;&nbsp;• **Plug-and-play expandability**: Add more Sensor Nodes to the FACP for larger deployments.  

✅ **Multi-Sensor Monitoring (Sensor Node STM32)**  
&nbsp;&nbsp;&nbsp;The sensor node has 3 groups of sensors:  
&nbsp;&nbsp;&nbsp;🔥**Fire Detection**: Temperature, Smoke, Gas, Flame sensors    
&nbsp;&nbsp;&nbsp;💧**Environmental**: Humidity, VOC sensors  
&nbsp;&nbsp;&nbsp;♨️**Smart Sensing**: Ambient Light, Thermal IR sensors    
&nbsp;&nbsp;&nbsp;*(Supports up to 8 sensors per node with configurable thresholds)*   

✅ **Fire Alarm Control Panel Node (ESP32)**  
&nbsp;&nbsp;&nbsp;• **Active Monitoring**: Periodically checks sensor node health via SPI.  
&nbsp;&nbsp;&nbsp;• **Event-Driven Response**: Instantly reacts to interrupt-based anomaly alerts from sensor nodes.  
&nbsp;&nbsp;&nbsp;• **Scalable Architecture**: Supports daisy-chaining multiple sensor nodes for large-scale deployments.  

✅ **Edge Processing**: Anomalies are identified at the sensor node level.   
✅ **Cloud Integration**: Lightweight AWS IoT Core messaging for live sensor status and emergency alerts.  

✅ **Robust Communication Stack**  
&nbsp;&nbsp;🔹 **UART Debugging**:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• Serial logs for sensor status, diagnostics, and development.  
&nbsp;&nbsp;🔹 **Hardware Interrupt Line**:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• Low-latency Sensor Node to FACP anomaly alerts (Node → FACP)  
&nbsp;&nbsp;🔹 **SPI**:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• Heartbeat checks (FACP → Node → FACP)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• On-demand sensor data transmission (Node → FACP)

**Two-Phase Command-Response Protocol SPI**  
&nbsp;&nbsp;➤ **Phase 1:**  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• Master (ESP32) initiates SPI communication and sends command  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• Slave (STM32) receives command, responds with dummy  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• Sensor Node gets the chance to prepare the response  
&nbsp;&nbsp;➤**Phase 2:**  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• Master sends dummy   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;• Slave responds with the actual response
```
|           Master                          |            Slave                              |
|   Phase 1 Command Sent: Are you alive?    |   Phase 1 Command received: Are you alive?    |
|   Phase 1 DUMMY received: FF FF FF FF     |   Phase 1 DUMMY sent: FF FF FF FF             |
|                                           |                                               |
|   Phase 2 Command Sent: FF FF FF FF       |   Phase 2 Command received: FF FF FF FF       |
|   Phase 2 DUMMY received: I'm alive       |   Phase 2 DUMMY sent: I'm alive               |
```

---
### 🏗 System Architecture
```
[Sensors] → [STM32 Sensor Node] → [SPI] → [ESP32 FACP/Cloud Node] → [MQTT] → [Cloud Dashboard]
```
### 🛠️ Tools and Software
𐂷 **Sensor Node**  
&nbsp;&nbsp;&nbsp;⎔ **VS Code** - Code editor for STM32 firmware development       
&nbsp;&nbsp;&nbsp;⎔ OpenOCD - Flashing and debugging via SWD     
&nbsp;&nbsp;&nbsp;⎔ Makefile - Builds and links embedded C code  

🌐 **FACP / Cloud Gateway**   
&nbsp;&nbsp;&nbsp;⎔ ESP-IDF - Framework for ESP32 development   
&nbsp;&nbsp;&nbsp;⎔ VS Code - Development and debugging for the gateway   
&nbsp;&nbsp;&nbsp;⎔ Terraform - Automates AWS infrastructure setup    
&nbsp;&nbsp;&nbsp;⎔ AWS Cloud - Hosts IoT Core, Timestream, and monitoring services  

### **Hardware Connections**
| **STM32 PIN** | **Interface**  | **ESP32 Pin** |
|---------------|----------------|---------------|
|     PA6       |     SPI MISO   |    GPIO19     |
|     PA7       |     SPI MOSI   |    GPIO23     |
|     PA4       |     SPI NSS    |    GPIO5      |
|     PA5       |     SPI SCK    |    GPIO18     |
|     PB6       | GPIO Interrupt |    GPIO22     |
|     GND       |      GND       |     GND       |

---
### 📂 Project Code Structure
```
📁 Smart-Fire-Detection-System/
│── 📁 stm32_sensor_node/
│   ├── 📄 main.c               (Entry point of the program)
│   ├── 📄 factory.cpp / .h     (Abstract Factory pattern implementation)
│   ├── 📄 sensor.cpp / .h      (Base sensor classes and interfaces)
│   ├── 📄 wrapper.cpp / .h     (Hardware abstraction layer wrappers)
│   ├── 📄 simulate.c / .h      (Sensor data simulation)
│   ├── 📄 spi.c / .h           (SPI & GPIO Interrupt Communication)
│   ├── 📄 uart.c / .h          (UART Communication)
│   ├── 📄 systick.c / .h       (Systick Timer)
│   ├── 📄 Makefile             (Build system configuration)
│── 📁 esp32_facp_cloud_node/
│   ├── 📄 main.c               (Entry point of the program, Tasks)
│   ├── 📄 spi.c / .h           (SPI & GPIO Interrupt Communication )
│   ├── 📄 uart.c / .h          (UART Communication)
│   ├── 📄 wifi.c / .h          (WiFi Connectivity)
│   ├── 📄 cloud.c / .h         (MQTT for AWS Connectivity)
│   ├── 📄 CMakeLists.txt       (Build system configuration)
│── 📄 README.md  (Documentation)
```









