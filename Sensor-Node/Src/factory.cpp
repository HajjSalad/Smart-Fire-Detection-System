
// #include "factory.h"
// #include "sensor.h"

// #include <string>
// #include <stdio.h>

// #include <memory>
// #include <string>
// #include <vector>

// // Concrete Factory for Fire Detection Sensors
// std::unique_ptr<Sensor> FireSensorFactory::createTempSensor() {
//     return std::make_unique<TempSensor>();
// }

// std::unique_ptr<Sensor> FireSensorFactory::createSmokeSensor() {
//     return std::make_unique<SmokeSensor>();
// }

// std::unique_ptr<Sensor> FireSensorFactory::createGasSensor() {
//     return std::make_unique<GasSensor>();
// }

// std::unique_ptr<Sensor> FireSensorFactory::createFlameSensor() {
//     return std::make_unique<FlameSensor>();
// }

// // Concrete Factory for Environment Sensors
// std::unique_ptr<Sensor> EnvironSensorFactory::createHumiditySensor() {
//     return std::make_unique<HumiditySensor>();
// }

// std::unique_ptr<Sensor> EnvironSensorFactory::createVOCSensor() {
//     return std::make_unique<VOCSensor>();
// }

// // Concrete Factory for Smart Features Sensors
// std::unique_ptr<Sensor> SmartSensorFactory::createAmbientLSensor() {
//     return std::make_unique<AmbientLightSensor>();
// }

// std::unique_ptr<Sensor> SmartSensorFactory::createThermalISensor() {
//     return std::make_unique<ThermalIRSensor>();
// }







// // // Abstract factory Interface - define the methods for creating each type of sensor
// // class SensorFactory {
// // public:
// //     // Fire detection Sensors
// //     virtual std::unique_ptr<Sensor> createTempSensor() = 0;
// //     virtual std::unique_ptr<Sensor> createSmokeSensor() = 0;
// //     virtual std::unique_ptr<Sensor> createGasSensor() = 0;
// //     virtual std::unique_ptr<Sensor> createFlameSensor() = 0;

// //     // Environment Monitoring Sensors
// //     virtual std::unique_ptr<Sensor> createHumiditySensor() = 0;
// //     virtual std::unique_ptr<Sensor> createVOCSensor() = 0;

// //     // Smart Features
// //     virtual std::unique_ptr<Sensor> createAmbientLSensor() = 0;
// //     virtual std::unique_ptr<Sensor> createThermalISensor() = 0;
// // }

// // // Concrete Factories - implement Sensorfactory interface and create specific set of related sensors

// // // Concrete Factory for Fire Detection Sensors
// // class FireSensorFactory : public SensorFactory {
// // public:
// //     std::unique_ptr<Sensor> createTempSensor() override {
// //         return std::make_unique<TemperatureSensor>();               // Concrete sensor creation
// //     }

// //     std::unique_ptr<Sensor> createSmokeSensor() override {
// //         return std::make_unique<SmokeSensor>(); 
// //     }

// //     std::unique_ptr<Sensor> createGasSensor() override {
// //         return std::make_unique<GasSensor>(); 
// //     }

// //     std::unique_ptr<Sensor> createFlameSensor() override {
// //         return std::make_unique<FlameSensor>(); 
// //     }

// //     // Other sensors are not part of this factory, so they return nullptr
// //     std::unique_ptr<Sensor> createHumiditySensor() override { return nullptr; }
// //     std::unique_ptr<Sensor> createVOCSensor() override { return nullptr; }
// //     std::unique_ptr<Sensor> createAmbientLSensor() override { return nullptr; }
// //     std::unique_ptr<Sensor> createThermalISensor() override { return nullptr; }
// // };

// // // Concrete Factory for Environment Sensors
// // class EnvironSensorFactory : public SensorFactory {
// //     public:
// //         std::unique_ptr<Sensor> createHumiditySensor() override {
// //             return std::make_unique<HumiditySensor>();                  
// //         }
    
// //         std::unique_ptr<Sensor> createVOCSensor() override {
// //             return std::make_unique<VOCSensor>();                  
// //         }
    
// //         std::unique_ptr<Sensor> createTempSensor() override { return nullptr; }
// //         std::unique_ptr<Sensor> createSmokeSensor() override { return nullptr; }
// //         std::unique_ptr<Sensor> createGasSensor() override { return nullptr; }
// //         std::unique_ptr<Sensor> createFlameSensor() override { return nullptr; }
// //         std::unique_ptr<Sensor> createAmbientLSensor() override { return nullptr; }
// //         std::unique_ptr<Sensor> createThermalISensor() override { return nullptr; }
// //     };

// // // Concrete Factory for Smart Features Sensors
// // class SmartSensorFactory : public SensorFactory {
// // public:
// //     std::unique_ptr<Sensor> createAmbientLSensor() override {
// //         return std::make_unique<AmbientLightSensor>(); // Concrete sensor creation
// //     }

// //     std::unique_ptr<Sensor> createThermalISensor() override {
// //         return std::make_unique<ThermalIRSensor>(); 
// //     }

// //     // Other sensors are not part of this factory, so they return nullptr
// //     std::unique_ptr<Sensor> createTempSensor() override { return nullptr; }
// //     std::unique_ptr<Sensor> createSmokeSensor() override { return nullptr; }
// //     std::unique_ptr<Sensor> createGasSensor() override { return nullptr; }
// //     std::unique_ptr<Sensor> createFlameSensor() override { return nullptr; }
// //     std::unique_ptr<Sensor> createHumiditySensor() override { return nullptr; }
// //     std::unique_ptr<Sensor> createVOCSensor() override { return nullptr; }
// // };