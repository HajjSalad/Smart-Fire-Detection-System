#ifndef SIMULATE_H
#define SIMULATE_H

#include "wrapper.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stm32f446xx.h"

#define NUM_GROUPS              3           // How many groups of sensors you want
#define MAX_SENSOR_PER_GROUP    4           // Max number of sensor per group
#define MAX_SENSOR              8           // Assume max sensor = 8 per sensor node (ie 1 of each group)
#define CIRC_BUFFER_SIZE        10          // Circular buffer to store the last 10 sensor readings

typedef struct {
    float data[CIRC_BUFFER_SIZE];
    int head;
    int count;
    float min;
    float max;
} CircularBuffer; 

extern SensorGroup* group[NUM_GROUPS];
extern bool anomalyFlag[MAX_SENSOR];
extern bool queuedItems[MAX_SENSOR];
extern CircularBuffer sensorBuffers[MAX_SENSOR];       

typedef struct {
    float min;
    float max;
} Range;

extern Range sensor_ranges[NUM_GROUPS][MAX_SENSOR_PER_GROUP];

void init_all_buffers();
void start_simulation();
void stop_simulation();
void print_stored_sensor_values(CircularBuffer* buffers, int numBuffers);
void add_to_buffer(CircularBuffer* buffer, float value);
float simulate_sensor_value(int group, int localIndex, int sensorIndex, int anomaly_index);
void process_sensor_values(bool injectAnomaly);
void systick_simulation();

#endif // SIMULATE_H
