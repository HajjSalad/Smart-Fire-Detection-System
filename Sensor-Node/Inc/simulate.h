// #ifndef SIMULATE_H
// #define SIMULATE_H

// #include "wrapper.h"

// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <stdbool.h>
// #include "stm32f446xx.h"

// #define NUM_GROUPS              3           // How many groups of sensors you want
// #define MAX_SENSOR_PER_GROUP    4           // Max number of sensor per group
// #define MAX_SENSOR              8           // Assume amx sensor = 8 per sensor node (ie 1 of each group)
// #define CIRC_BUFFER_SIZE        10          // Circular buffer to store the last 10 sensor readings

// typedef struct {
//     float data[CIRC_BUFFER_SIZE];
//     int head;
//     int count;
//     float min;
//     float max;
// } CircularBuffer; 

// extern SensorGroup* group[NUM_GROUPS];
// extern int groupSensorCount[NUM_GROUPS];
// extern bool anomalyFlag[MAX_SENSOR];
// extern CircularBuffer sensorBuffers[MAX_SENSOR];       

// typedef struct {
//     float min;
//     float max;
// } Range;

// Range sensor_ranges[NUM_GROUPS][MAX_SENSOR_PER_GROUP] = {
//     { 
//         { -40.0f, 125.0f },     // TempSensor
//         { 0.0f, 1000.0f },      // SmokeSensor
//         { 0.0f, 10000.0f },     // GasSensor
//         { 0.0f, 1.0f }          // FlameSensor
//     },
//     { 
//         { 0.0f, 100.0f },       // HumiditySensor
//         { 0.0f, 500.0f }        // VOCSensor
//     },
//     { 
//         { 0.0f, 100000.0f },    // AmbientLightSensor
//         { -20.0f, 300.0f }      // ThermalIRSensor
//     }
// };

// void add_to_buffer(CircularBuffer* buffer, float value);
// void trigger_anomaly_interrupt(void);
// float simulate_sensor_value(int group, int sensorIndex);
// void process_sensor_values();

// #endif // SIMULATE_H