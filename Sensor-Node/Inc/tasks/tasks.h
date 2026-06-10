#ifndef TASKS_H
#define TASKS_H

/**
 * @file  tasks.h
 * @brief Tasks interface.
*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "shared_resources.h"

#include <stdio.h>

// Anomaly levels
typedef enum {
    ANOMALY_NONE      = 0,
    ANOMALY_WARNING   = 1,
    ANOMALY_CRITICAL  = 2
} AnomalyLevel_t;

// Sensor thresholds
// Thresholds
#define THRESH_TEMP_WARNING     40.0f    // °C
#define THRESH_TEMP_CRITICAL    80.0f    // °C
#define THRESH_HUMI_HIGH        80.0f    // %RH
#define THRESH_PRES_LOW         970.0f   // hPa
#define THRESH_VOC_POOR         100.0f // Ω — lower = worse air
#define THRESH_CO2_WARNING      1000.0f  // ppm
#define THRESH_CO2_CRITICAL     5000.0f  // ppm
#define THRESH_PM25_WARNING     35.0f    // µg/m³
#define THRESH_PM25_HAZARDOUS   150.0f   // µg/m³

typedef enum {
    MODBUS_STATE_IDLE,
    MODBUS_STATE_RECEIVING,
    MODBUS_STATE_PROCESSING,
    MODBUS_STATE_RESPONDING,
    MODBUS_STATE_RESPONDED,
    MODBUS_STATE_ERROR
} Modbus_State_t;

#define REQUEST_MAX_FRAME_LEN             32U
#define RESPONSE_MAX_FRAME_LEN            32U

typedef struct {
    Modbus_State_t state;
    uint8_t        frame_len;
    uint8_t        frame[REQUEST_MAX_FRAME_LEN];
    uint8_t        resp_len;
    uint8_t        resp[RESPONSE_MAX_FRAME_LEN];
} Modbus_Context_t;

// Function Prototypes
void vTaskWatchdogMonitor(void *pvParameters);
void vTaskSensorRead(void *pvParameters);
void vTaskAnomalyDetect(void *pvParameters);
void vTaskModbusSlave(void *pvParameters);
void vTaskLogger(void *pvParameters);

#endif // TASKS_H 