#ifndef WRAPPER_H
#define WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void* SensorGroup;

// Create a group of sensors
SensorGroup create_sensor_group(const char* type);

// Destroy the sensor group
void destroy_sensor_group(SensorGroup group);

// Get the number of sensors in the group
int get_sensor_count(SensorGroup group);

// Set a sensor's value
void set_sensor_value(SensorGroup group, int index, float value);

// Get a sensor's value
float get_sensor_value(SensorGroup group, int index);

#ifdef __cplusplus
}
#endif

#endif // WRAPPER_H