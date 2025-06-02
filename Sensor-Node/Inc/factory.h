#ifndef FACTORY_H
#define FACTORY_H

#include "sensor.h"

#include <string>
#include <stdio.h>

#include <memory>
#include <string>
#include <vector>

// Forward declarations for Sensor classes
class Sensor;
class TempSensor;
class SmokeSensor;
class GasSensor;
class FlameSensor;
class HumiditySensor;
class VOCSensor;
class AmbientLightSensor;
class ThermalIRSensor;

// Abstract factory Interface - defines the methods for creating each type of sensor
class SensorFactory {
public:
    // Fire detection Sensors
    virtual std::unique_ptr<Sensor> createTempSensor() = 0;
    virtual std::unique_ptr<Sensor> createSmokeSensor() = 0;
    virtual std::unique_ptr<Sensor> createGasSensor() = 0;
    virtual std::unique_ptr<Sensor> createFlameSensor() = 0;

    // Environment Monitoring Sensors
    virtual std::unique_ptr<Sensor> createHumiditySensor() = 0;
    virtual std::unique_ptr<Sensor> createVOCSensor() = 0;

    // Smart Features
    virtual std::unique_ptr<Sensor> createAmbientLSensor() = 0;
    virtual std::unique_ptr<Sensor> createThermalISensor() = 0;

    virtual ~SensorFactory() = default; // Virtual Destructor for polymorphism
};

// Concrete Factories - implement Sensorfactory interface and create specific set of related sensors

// Concrete Factory for Fire Detection Sensors
class FireSensorFactory : public SensorFactory {
public:
    std::unique_ptr<Sensor> createTempSensor() override;
    std::unique_ptr<Sensor> createSmokeSensor() override;
    std::unique_ptr<Sensor> createGasSensor() override;
    std::unique_ptr<Sensor> createFlameSensor() override;

    std::unique_ptr<Sensor> createHumiditySensor() override { return nullptr; }
    std::unique_ptr<Sensor> createVOCSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createAmbientLSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createThermalISensor() override { return nullptr; }
};

// Concrete Factory for Environment Sensors
class EnvironSensorFactory : public SensorFactory {
public:
    std::unique_ptr<Sensor> createHumiditySensor() override;
    std::unique_ptr<Sensor> createVOCSensor() override;

    std::unique_ptr<Sensor> createTempSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createSmokeSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createGasSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createFlameSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createAmbientLSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createThermalISensor() override { return nullptr; }
};

// Concrete Factory for Smart Features Sensors
class SmartSensorFactory : public SensorFactory {
public:
    std::unique_ptr<Sensor> createAmbientLSensor() override;
    std::unique_ptr<Sensor> createThermalISensor() override;

    std::unique_ptr<Sensor> createTempSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createSmokeSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createGasSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createFlameSensor() override { return nullptr; }
    std::unique_ptr<Sensor> createHumiditySensor() override { return nullptr; }
    std::unique_ptr<Sensor> createVOCSensor() override { return nullptr; }
};

#endif // FACTORY_H
