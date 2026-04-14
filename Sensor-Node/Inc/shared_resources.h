#ifndef SHARED_RESOURCES_H
#define SHARED_RESOURCES_H

/**
 * @file  shared_resources.h
 * @brief Shared constants, macros, and type definitions used across all modules.
*/

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>

/** @brief Format for printf */
#define LOG(fmt, ...)  printf( (fmt "\n\r"), ##__VA_ARGS__)

// Sensor data queue between Task 1 and Task 2
#define SENSOR_QUEUE_DEPTH      (20U)
extern QueueHandle_t            xSensorQueue;

// Logger queue 
#define LOG_MSG_MAX_LEN         (144U)
#define LOG_QUEUE_DEPTH         (20U)
extern QueueHandle_t            xLogQueue;

// Sensor data
typedef struct {
    float    temp;       /* Temperature       : °C     — TMP102    */
    float    humi;       /* Humidity          : %RH    — BME680    */
    float    pres;       /* Pressure          : hPa    — BME680    */
    float    voc;        /* VOC               : Ω      — BME680    */
    float    co2;        /* CO2               : ppm    — Simulated */
    float    pm25;       /* Particulate PM2.5 : µg/m³  — Simulated */
    uint8_t  flame;      /* Flame detected    : 0/1    — GPIO      */
    uint32_t timestamp;  /* Tick count        : ms     — FreeRTOS  */
} SensorData_t;

// Shared struct between Task 1 and Modbus
extern SensorData_t             shared_sensor_data;
// Mutex to protect the shared struct between Task 1 and Modbus
extern SemaphoreHandle_t        xSensorMutex;

#endif  // SHARED_RESOURCES_H