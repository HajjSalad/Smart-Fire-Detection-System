#ifndef SENSOR_H
#define SENSOR_H

#include <string>
#include <stdio.h>

#include <memory>
#include <string>
#include <vector>

// Base class Sensor
class Sensor {
protected:
    float Value;
public:
    virtual void setValue(float value) = 0;
    virtual float readValue() = 0;
    virtual const char* getName() const = 0;
    virtual ~Sensor() = default;
};

class TempSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
    const char* getName() const override { return "TempSensor"; }
};

class SmokeSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
    const char* getName() const override { return "SmokeSensor"; }
};

class GasSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
    const char* getName() const override { return "GasSensor"; }
};

class FlameSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
    const char* getName() const override { return "FlameSensor"; }
};

class HumiditySensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
    const char* getName() const override { return "HumiditySensor"; }
};

class VOCSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
    const char* getName() const override { return "VOCSensor"; }
};

class AmbientLightSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
    const char* getName() const override { return "AmbientLightSensor"; }
};

class ThermalIRSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
    const char* getName() const override { return "ThermalIRSensor"; }
};

#endif // SENSOR_H