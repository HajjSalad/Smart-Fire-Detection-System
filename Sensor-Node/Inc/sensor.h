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
    virtual ~Sensor() = default;
};

class TempSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
};

class SmokeSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
};

class GasSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
};

class FlameSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
};

class HumiditySensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
};

class VOCSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
};

class AmbientLightSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
};

class ThermalIRSensor : public Sensor {
public:
    void setValue(float value) override;
    float readValue() override;
};

#endif // SENSOR_H