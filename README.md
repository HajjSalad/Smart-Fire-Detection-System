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
🔹 **Abstract Factory Pattern** in C++ for dynamic sensor management.
🔹 Plug-and-play expandability: Add more Sensor Nodes to the FACP for 
This project is designed to monitor critical **industrial parameters** such as:
✅ 🔹 Abstract Factory Pattern in C++ for dynamic sensor management
✅ **Vibration** (MPU6050)  
✅ **Distance/Motion Detection** (HC-SR04)  
✅ **Additional Analog Sensors via SPI ADC**  
