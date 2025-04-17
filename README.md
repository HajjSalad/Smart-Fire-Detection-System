## 🚨 Smart Fire Detection System
Smart fire detection system with STM32 as the Sensor Node and ESP32 as the Fire Alarm Control Panel and cloud gateway, using C++ Abstract Factory to manage modular sensors with clean abstraction and dynamic instantiation.

### 📌 Project Overview
- The **STM32 Sensor Node** continuously monitors all sensors and communicates with the **ESP32 Fire Alarm Control Panel (FACP)** via **SPI**.
- The **FACP performs periodic health checks** by pinging the Sensor Node; if all is normal, it receives a simple acknowledgment.
- On detecting an anomaly, the **Sensor Node raises an interrupt**, prompting the FACP to **request detailed sensor readings**.
- The **FACP reports sensor health and emergency data** to the cloud via **MQTT**, using **AWS IoT Core** for real-time alerting and remote monitoring.

### 🔧 Features
🔹**Modular & Scalable Architecture**
Utilizes the **Abstract Factory Pattern** in C++ to dynamically create and manage multiple sensors, enabling clean abstraction and easy scalability. Additional Sensor Nodes can be connected to the FACP to expand the network.
🔹**Sensor Node (STM32)**
Supports up to 8 modular sensors to monitor key parameters:
 - **Fire Detection**: Temperature, Smoke, Gas, Flame
 - **Environmental Monitoring**: Humidity, VOC
 - **Smart Sensing**: Ambient Light, Thermal IR
🔹**Reliable Communication Interfaces**
**SPI**:
- Periodic health checks between Sensor Node and FACP
- Transmission of sensor data on request
**Interrupt Line**:
- Sensor Node triggers an interrupt to notify FACP of anomalies
**UART**:
- Logging sensor data, system status, and debugging info via serial terminal
🔹**FACP Node (ESP32)**
Continuously monitors Sensor Nodes
Communicates with the cloud over MQTT
Requests sensor data during anomalies
Can support multiple Sensor Nodes for larger deployments
Edge Processing
Anomaly detection is handled at the Sensor Node level. The FACP sends intelligent alerts to the cloud only when needed, reducing unnecessary data traffic.

Cloud Integration
Real-time monitoring and alerting via AWS IoT Core using MQTT protocol for scalable, remote access and control.

