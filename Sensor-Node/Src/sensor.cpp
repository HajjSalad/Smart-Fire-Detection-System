
#include "sensor.h"
#include "factory.h"

#include <string>
#include <stdio.h>

#include <memory>
#include <string>
#include <vector>

// Sensor implementations
void TempSensor::setValue(float value) { Value = value; }
float TempSensor::readValue() { return Value; }
// const char* TempSensor::getName() const { return "TempSensor"; }

void SmokeSensor::setValue(float value) { Value = value; }
float SmokeSensor::readValue() { return Value; }
// const char* SmokeSensor::getName() const { return "SmokeSensor"; }

void GasSensor::setValue(float value) { Value = value; }
float GasSensor::readValue() { return Value; }
// const char* GasSensor::getName() const { return "GasSensor"; }

void FlameSensor::setValue(float value) { Value = value; }
float FlameSensor::readValue() { return Value; }
// const char* FlameSensor::getName() const { return "FlameSensor"; }

void HumiditySensor::setValue(float value) { Value = value; }
float HumiditySensor::readValue() { return Value; }
// const char* HumiditySensor::getName() const { return "HumiditySensor"; }

void VOCSensor::setValue(float value) { Value = value; }
float VOCSensor::readValue() { return Value; }
// const char* VOCSensor::getName() const { return "VOCSensor"; }

void AmbientLightSensor::setValue(float value) { Value = value; }
float AmbientLightSensor::readValue() { return Value; }
// const char* AmbientLightSensor::getName() const { return "AmbientLightSensor"; }

void ThermalIRSensor::setValue(float value) { Value = value; }
float ThermalIRSensor::readValue() { return Value; }
// const char* ThermalIRSensor::getName() const { return "ThermalIRSensor"; }