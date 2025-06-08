
#include "simulate.h"
#include "wrapper.h"
#include "systick.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stm32f446xx.h"

bool createAnomaly = false;

SensorGroup* group[NUM_GROUPS];                         // Array pointers for sensor groups
bool anomalyFlag[MAX_SENSOR];                           // Instantiate a flag for each sensor
CircularBuffer sensorBuffers[MAX_SENSOR];               // Instantiate a circular buffer for each sensor

Range sensor_ranges[NUM_GROUPS][MAX_SENSOR_PER_GROUP] = {
    { 
        { -40.0f, 125.0f },     // TempSensor
        { 0.0f, 1000.0f },      // SmokeSensor
        { 0.0f, 10000.0f },     // GasSensor
        { 0.0f, 1.0f }          // FlameSensor
    },
    { 
        { 0.0f, 100.0f },       // HumiditySensor
        { 0.0f, 500.0f }        // VOCSensor
    },
    { 
        { 0.0f, 100000.0f },    // AmbientLightSensor
        { -20.0f, 300.0f }      // ThermalIRSensor
    }
};

void print_stored_sensor_values(CircularBuffer* buffers, int numBuffers) {
    for (int i = 0; i < numBuffers; i++) {
        if (buffers[i].count == 0) {
            printf("Buffer %d is empty\r\n", i);
            continue;
        }
        int lastIndex = (buffers[i].head - 1 + CIRC_BUFFER_SIZE) % CIRC_BUFFER_SIZE;
        float lastValue = buffers[i].data[lastIndex];
        printf("Buffer %d - Last Value: %.2f\r\n", i, lastValue);
    }
}

// Add to buffer for temporary storage
void add_to_buffer(CircularBuffer* buffer, float value) {
    if(!buffer) return;         // check for null pointer

    buffer->data[buffer->head] = value;                     // Add new value at the head position
    buffer->head = (buffer->head + 1) % CIRC_BUFFER_SIZE;   // Move head to the next position 

    // If not full, increment the count of stored values
    if (buffer->count < CIRC_BUFFER_SIZE) {                 
        buffer->count++;       // update count
    }

    // Recalculate min and max for the current buffer data 
    buffer->min = buffer->max = buffer->data[0];
    for (int i=0; i<buffer->count; i++) {
        float v = buffer->data[i];
        if (v < buffer->min) buffer->min = v;
        if (v > buffer->max) buffer->max = v;
    }
}

// Notify the ESP32 of the anomaly
// void trigger_anomaly_interrupt(void) {
//     GPIOB->ODR |= (1 << 6);
//     for (volatile int d = 0; d < 1000; d++);
//     GPIOB->ODR &= ~(1 << 6);
// }

// For simulation purposes, we assume all sensors generate float data 
float simulate_sensor_value(int group, int sensorIndex, int anomalyIndex) {
    Range r = sensor_ranges[group][sensorIndex];

    if (sensorIndex == anomalyIndex) {        // Generate a value out of range
        // createAnomaly = false;                  // Reset flag
        float offset = (rand() % 2 == 0) ? -50.0f : 50.0f;
        return r.max + offset;
    } else {                    // Generate random float within range
        float scale = rand() / (float) RAND_MAX; 
        float value = r.min + scale * (r.max - r.min);
        return value;
    }
}

// Call the function every 10 seconds
void Systick_Runner(void) {
    ms_ticks++;
    
    if (ms_ticks >= 10000) {        // Every 10sec
        process_sensor_values();
        ms_ticks = 0;  // Reset timer
    }
}

// void sendTrigger(void) {
//     printf("\nSet interrupt line high\r\n");
//         GPIOB->ODR |= (1<<6);       // Set interrupt line high
//   
//         // systickDelayMs(50);
//         GPIOB->ODR &= ~(1<<6);      // Set interrupt line low 
//         printf("Set interrupt line low\r\n");
// }

// Set the simulated value into the sensor and check for anomaly
void process_sensor_values() {
    int sensorIndex = 0;
    int anomalyIndex = (rand() % MAX_SENSOR);
    printf("\nAnomaly will be on sensor Index: %d\r\n", anomalyIndex);

    for (int i = 0; i < NUM_GROUPS; i++) {
        int count = get_sensor_count(group[i]);

        for (int j = 0; j < count; j++) {
            float value = simulate_sensor_value(i, j, anomalyIndex);
            const char* name = get_sensor_name(group[i], j);

            // Set the value into the sensor 
            set_sensor_value(group[i], j, value);
            // printf("Group %d Sensor %d: %.2f", i, j+1, value);
            printf("%-20s: %10.2f", name, value);

            // Check for any anomaly
            Range r = sensor_ranges[i][j];
            if (value >= r.min && value <= r.max) {     // Within range
                printf(" - within range\r\n");
            } else {
                printf(" - out of range\r\n");          // Sensor value out of range
                anomalyFlag[anomalyIndex] = true;        // Anomaly detected - raise flag
            }
            //add_to_buffer(&sensorBuffers[sensorIndex], value);     // Add sensor value to its circular buffer

            if (anomalyFlag[anomalyIndex]) {
                //sendTrigger();                          // Trigger the interrupt line to notify ESP32
                // Print the last 10 values of that sensor
                //print_stored_sensor_values(&sensorBuffers[sensorIndex], 1);
                anomalyFlag[anomalyIndex] = false;       // Reset flag
            }
            sensorIndex++;
        }
    }
}






// bool createAnomaly = false;

// SensorGroup* group[NUM_GROUPS];                         // Array pointers for sensor groups
// int groupSensorCount[NUM_GROUPS];                       // Number of sensors in each group
// bool anomalyFlag[MAX_SENSOR];                           // Instantiate a flag for each sensor
// CircularBuffer sensorBuffers[MAX_SENSOR];               // Instantiate a circular buffer for each sensor

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

// void print_last_10_values(CircularBuffer* buffers, int numBuffers) {
//     for (int i = 0; i < numBuffers; i++) {
//         if (buffers[i].count == 0) {
//             printf("Buffer %d is empty\r\n", i);
//             continue;
//         }
//         int lastIndex = (buffers[i].head - 1 + CIRC_BUFFER_SIZE) % CIRC_BUFFER_SIZE;
//         float lastValue = buffers[i].data[lastIndex];
//         printf("Buffer %d - Last Value: %.2f\r\n", i, lastValue);
//     }
// }

// // Add to buffer for temporary storage
// void add_to_buffer(CircularBuffer* buffer, float value) {
//     if(!buffer) return;         // check for null pointer

//     buffer->data[buffer->head] = value;                     // Add new value at the head position
//     buffer->head = (buffer->head + 1) % CIRC_BUFFER_SIZE;   // Move head to the next position 

//     // If not full, increment the count of stored values
//     if (buffer->count < CIRC_BUFFER_SIZE) {                 
//         buffer->count++;       // update count
//     }

//     // Recalculate min and max for the current buffer data 
//     buffer->min = buffer->max = buffer->data[0];
//     for (int i=0; i<buffer->count; i++) {
//         float v = buffer->data[i];
//         if (v < buffer->min) buffer->min = v;
//         if (v > buffer->max) buffer->max = v;
//     }
// }

// // Notify the ESP32 of the anomaly
// // void trigger_anomaly_interrupt(void) {
// //     GPIOB->ODR |= (1 << 6);
// //     for (volatile int d = 0; d < 1000; d++);
// //     GPIOB->ODR &= ~(1 << 6);
// // }

// // For simulation purposes, we assume all sensors generate float data 
// float simulate_sensor_value(int group, int sensorIndex) {
//     Range r = sensor_ranges[group][sensorIndex];

//     if (createAnomaly) {        // Generate a value out of range
//         createAnomaly = false;                  // Reset flag
//         float offset = (rand() % 2 == 0) ? -50.0f : 50.0f;
//         return r.max + offset;
//     } else {                    // Generate random float within range
//         float scale = rand() / (float) RAND_MAX; 
//         float value = r.min + scale * (r.max - r.min);
//         return value;
//     }
// }

// // Send an interrupt every 20 sec to simulate anomaly
// void Systick_Runner(void) {
//     ms_ticks++;
    
//     if (ms_ticks >= 20000) {        // Every 20sec
//         createAnomaly = true;
//         ms_ticks = 0;  // Reset timer
//     }
// }

// void sendTrigger(void) {
//     printf("\nSet interrupt line high\r\n");
//         GPIOB->ODR |= (1<<6);       // Set interrupt line high
    
//         // systickDelayMs(50);
//         GPIOB->ODR &= ~(1<<6);      // Set interrupt line low 
//         printf("Set interrupt line low\r\n");
// }

// // Set the simulated value into the sensor and check for anomaly
// void process_sensor_values() {
//     int sensorIndex = 0;
//     for (int i = 0; i < NUM_GROUPS; i++) {
//         for (int j = 0; j < groupSensorCount[i]; j++) {
//             float value = simulate_sensor_value(i, j);

//             // Set the value into the sensor 
//             set_sensor_value(group[i], j, value);
//             printf("Group %d Sensor %d: %.2f", i, j+1, value);

//             // Check for any anomaly
//             Range r = sensor_ranges[i][j];
//             if (value >= r.min && value <= r.max) {     // Within range
//                 printf(" - within range\r\n");
//             } else {
//                 printf(" - out of range\r\n");          // Sensor value out of range
//                 anomalyFlag[sensorIndex] = true;        // Anomaly detected - raise flag
//             }
//             add_to_buffer(&sensorBuffers[sensorIndex], value);     // Add sensor value to its circular buffer

//             if (anomalyFlag[sensorIndex]) {
//                 sendTrigger();                          // Trigger the interrupt line to notify ESP32
//                 // Print the last 10 values of that sensor
//                 print_last_10_values(&sensorBuffers[sensorIndex], 1);
//                 anomalyFlag[sensorIndex] = false;       // Reset flag
//             }
//             sensorIndex++;
//         }
//     }
// }




// SensorGroup* group[NUM_GROUPS];                         // Array pointers for sensor groups
// int groupSensorCount[NUM_GROUPS];                       // Number of sensors in each group
// bool anomalyFlag[MAX_SENSOR];                           // Instantiate a flag for each sensor
// CircularBuffer sensorBuffers[MAX_SENSOR];               // Instantiate a circular buffer for each sensor

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

// void print_last_values(CircularBuffer* buffers, int numBuffers) {
//     for (int i = 0; i < numBuffers; i++) {
//         if (buffers[i].count == 0) {
//             printf("Buffer %d is empty\r\n", i);
//             continue;
//         }
//         int lastIndex = (buffers[i].head - 1 + CIRC_BUFFER_SIZE) % CIRC_BUFFER_SIZE;
//         float lastValue = buffers[i].data[lastIndex];
//         printf("Buffer %d - Last Value: %.2f\r\n", i, lastValue);
//     }
// }

// // Add to buffer for temporary storage
// void add_to_buffer(CircularBuffer* buffer, float value) {
//     if(!buffer) return;         // check for null pointer

//     buffer->data[buffer->head] = value;                     // Add new value at the head position
//     buffer->head = (buffer->head + 1) % CIRC_BUFFER_SIZE;   // Move head to the next position 

//     // If not full, increment the count of stored values
//     if (buffer->count < CIRC_BUFFER_SIZE) {                 
//         buffer->count++;       // update count
//     }

//     // Recalculate min and max for the current buffer data 
//     buffer->min = buffer->max = buffer->data[0];
//     for (int i=0; i<buffer->count; i++) {
//         float v = buffer->data[i];
//         if (v < buffer->min) buffer->min = v;
//         if (v > buffer->max) buffer->max = v;
//     }
// }

// // Notify the ESP32 of the anomaly
// // void trigger_anomaly_interrupt(void) {
// //     GPIOB->ODR |= (1 << 6);
// //     for (volatile int d = 0; d < 1000; d++);
// //     GPIOB->ODR &= ~(1 << 6);
// // }

// // For simulation purposes, we assume all sensors generate float data 
// float simulate_sensor_value(int group, int sensorIndex) {
//     Range r = sensor_ranges[group][sensorIndex];

//     // Generate random float within range
//     float scale = rand() / (float) RAND_MAX; 
//     float value = r.min + scale * (r.max - r.min);
    
//     return value;
// }

// // Set the simulated value into the sensor and check for anomaly
// void process_sensor_values() {
//     int sensorIndex = 0;
//     for (int i = 0; i < NUM_GROUPS; i++) {
//         for (int j = 0; j < groupSensorCount[i]; j++) {
//             float value = simulate_sensor_value(i, j);

//             // Set the value into the sensor 
//             set_sensor_value(group[i], j, value);
//             printf("Group %d Sensor %d: %.2f", i, j+1, value);

//             // Check for any anomaly
//             Range r = sensor_ranges[i][j];
//             if (value >= r.min && value <= r.max) {     // Within range
//                 printf(" - within range\r\n");
//             } else {
//                 printf(" - out of range\r\n");          // Sensor value out of range
//                 // anomalyFlag[sensorIndex] = true;        // Anomaly detected - raise flag
//             }
//             add_to_buffer(&sensorBuffers[sensorIndex], value);     // Add sensor value to its circular buffer

//             if (anomalyFlag[sensorIndex]) {
//                 //trigger_anomaly_interrupt();            // Trigger the interrupt line to notify ESP32
//                 anomalyFlag[sensorIndex] = false;       // Reset flag
//             }
//             sensorIndex++;
//         }
//     }
// }