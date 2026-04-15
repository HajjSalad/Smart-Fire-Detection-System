## 🚨 Distributed Fire Safety System
Commercial and industrial fire safety systems require continuous, reliable environmental monitoring across distributed building zones - detecting early signs of fire through temperature, smoke, gas, and flame signatures before conditions become critical. This project implements that class of system as a distributed embedded architecture, following the sensor node and central controller pattern used in products by Honeywell, Siemens, and Bosch.  
STM32 sensor node deployed across zones, continuously sample environmental telemetry and report to a central ESP32 Fire Alarm Control Panel (FACP) via MODBUS RTU over RS-485. The FACP aggregates data from the node for remote monitoring and alerting.

The project is organized into four components:
- STM32 Sensor Node - environmental sensing, anomaly detection, and MODBUS slave communication
- MODBUS RTU — industrial communication protocol stack implemented from scratch between sensor nodes and control panel
- ESP32 Fire Alarm Control Panel — MODBUS master polling, and cloud gateway
- Sensor Node PCB Design — KiCad schematic for a custom sensor node PCB with a clear v1/v2 revision roadmap
---
### 🧪 STM32 Sensor Node
The FreeRTOS-based sensor node continuously samples environmental telemetry across multiple sensor interfaces, performs on-device anomaly detection, and responds to MODBUS RTU polling requests from the ESP32 Fire Alarm Control Panel over RS-485.
#### 🔬 Sensor Stack
| Sensor | Measurement | Interface | Status |
|---|---|---|---|
| BME680 | Temperature, Humidity, Pressure, VOC | SPI1 | Real hardware |
| PMSA003I | PM2.5 Particulate Matter | I2C1 | Simulated |
| Gas Sensor | CO2 ppm | — | Simulated |
| Flame Sensor | Flame detected / not detected | GPIO | Real hardware |

Simulated sensors return realistic randomized values within calibrated physical ranges. Driver interfaces are production-ready — replacing simulated implementations with real hardware requires only swapping the driver body, the interface remains unchanged.
#### 🧵 Task Model
| Task | Priority | Responsibility |
|---|---|---|
| `vTaskSensorRead` | 4 | Samples all sensors, writes to `shared_sensor_data`, pushes to `xSensorDataQueue` |
| `vTaskAnomalyDetect` | 3 | Blocks on `xSensorDataQueue`, checks readings against thresholds, raises alert flag |
| `vTaskModbusSlave` | 3 | Polls UART ring buffer, parses MODBUS frames, reads `shared_sensor_data`, sends response |
| `vTaskSystemLogger` | 1 | Sole consumer of `xLogQueue` — drains and prints all log messages to UART terminal |
### 🔗 FreeRTOS Resources
| Resource | Type | Purpose |
|---|---|---|
| `xSensorDataMutex` | Mutex | Guards `shared_sensor_data` between `vTaskSensorRead` and `vTaskModbusSlave` |
| `xSensorDataQueue` | Queue | Passes `SensorData_t` from `vTaskSensorRead` → `vTaskAnomalyDetect` |
| `xLogQueue` | Queue | Passes log strings from all tasks → `vTaskSystemLogger` |
### 🔀 Data Flow
```
┌──────────────────┐
│ vTaskSensorRead  │──── writes ────→ shared_sensor_data (mutex) ←── reads ── vTaskModbusSlave
└────────┬─────────┘
         │ pushes SensorData_t
         ▼
┌────────────────────┐
│ xSensorDataQueue   │
└────────┬───────────┘
         │ blocks on
         ▼
┌──────────────────────┐
│ vTaskAnomalyDetect   │──── raises ────→ anomaly_flag
└──────────────────────┘

All tasks ──→ xLogQueue ──→ vTaskSystemLogger ──→ UART terminal
```
#### 📡 Peripheral Drivers
**SPI1 — BME680**
Bare-metal SPI1 driver with register-level reads. Full duplex master, Mode 0 (CPOL=0, CPHA=0), 1MHz clock. CS manually controlled via PC7 GPIO. Burst read using BME680 auto-increment register pointer.
```
PB3 — SCK  (AF5)
PA7 — MOSI (AF5)
PA6 — MISO (AF5)
PC7 — CS   (GPIO output, active low)
```
**I2C1 — PMSA003I**
Bare-metal I2C1 driver at 100kHz standard mode with 16MHz APB1 clock. Combined master transmitter / master receiver transaction — write register address, repeated start, read response bytes.
```
PB6 — SCL (AF4)
PB7 — SDA (AF4)
```
**UART1 — MODBUS**
Bare-metal UART1 driver at 115200 baud. ISR-driven ring buffer — ISR owns the head, `vTaskModbusSlave` owns the tail. Frame boundary detected via 3.5 character silence timeout (~2ms at 115200 baud).
```
PA9  — TX (AF7)
PA10 — RX (AF7)
```
---
### 📂 Project Code Structure
```
📁 fire-detector/
│── 📁 Sensor-Node/                          (STM32F446RE firmware)
│   ├── 📁 Inc/                              (Header files)
│   │   ├── 📁 comm/                         (Communication drivers)
│   │   ├── 📁 sensors/                      (Sensor interfaces)
│   │   ├── 📁 tasks/                        (FreeRTOS task declarations)
│   │   ├── 📁 utils/                        (Utility headers)
│   │   ├── 📁 CMSIS/                        (ARM CMSIS headers)
│   │   └── 📁 STM32F4xx/                    (STM32 HAL headers)
│   ├── 📁 Src/                              (Source files)
│   │   ├── 📄 main.c                        (Entry point, FreeRTOS scheduler init)
│   │   ├── 📄 syscalls.c                    (System call stubs)
│   │   ├── 📁 comm/                         (Communication driver implementations)
│   │   │   ├── 📄 i2c1_driver.c
│   │   │   ├── 📄 spi1_driver.c
│   │   │   ├── 📄 uart1_driver.c
│   │   │   └── 📄 uart2_driver.c
│   │   ├── 📁 sensors/                      (Sensor driver implementations)
│   │   │   ├── 📄 bme680_enviro_sensor.c
│   │   │   ├── 📄 button_flame_sensor.c
│   │   │   ├── 📄 simulate_smoke_sensor.c
│   │   │   └── 📄 simulate_gas_sensor.c
│   │   ├── 📁 tasks/                        (FreeRTOS task implementations)
│   │   │   ├── 📄 task_1_sensor_read.c
│   │   │   ├── 📄 task_2_anomaly_detect.c
│   │   │   ├── 📄 task_3_modbus_slave.c
│   │   │   └── 📄 task_4_system_logger.c
│   │   └── 📁 utils/                        (Utility implementations)
│   │       ├── 📄 crc_16.c
│   │       └── 📄 demo.cpp
│   ├── 📁 FreeRTOS/                         (FreeRTOS kernel source)
│   ├── 📁 Startup/                          (MCU startup assembly)
│   ├── 📁 Build/                            (Compiled output)
│   ├── 📄 STM32F446RETX_FLASH.ld            (Linker script — flash)
│   ├── 📄 STM32F446RETX_RAM.ld              (Linker script — RAM)
│   ├── 📄 Makefile                          (Build system configuration)
│   └── 📄 Doxyfile                          (Doxygen config)
│
│
│
│── 📁 Control-Panel/                        (ESP32 FACP + cloud node)
│   ├── 📁 main/                             (Application entry point)
│   ├── 📁 components/                       (ESP-IDF custom components)
│   │   ├── 📁 modbus/                       (MODBUS master implementation)
│   │   │   ├── 📁 include/
│   │   │   ├── 📄 modbus_master.c
│   │   │   ├── 📄 crc_16.c
│   │   │   └── 📄 CMakeLists.txt
│   │   └── 📁 uart/                         (UART driver component)
│   │       ├── 📁 include/
│   │       ├── 📄 uart2_driver.c
│   │       └── 📄 CMakeLists.txt
│   ├── 📄 CMakeLists.txt                    (Top-level ESP-IDF build config)
│   ├── 📄 sdkconfig                         (ESP-IDF SDK configuration)
│   ├── 📁 build/                            (Compiled output)
│   └── 📄 Doxyfile                          (Doxygen config)
│── 📄 README.md                             (Project documentation)
│── 📄 LICENSE
│── 📄 gdb_commands.gdb                      (GDB debug helper script)
└── 📄 demo.gif                              (Demo animation)
```
---
### 🎬 Demo
![Demo](./demo.gif)