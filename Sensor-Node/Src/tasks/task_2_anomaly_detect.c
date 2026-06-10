/**
 * @file  task_2_anomaly_detect.c
 * @brief 
 * 
*/

#include "stm32f446xx.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "alert_pin_driver.h"
#include "uart_driver.h"
#include "tasks.h"
#include "shared_resources.h"

#define ANOMALY_DETECT_TASK_PERIOD_MS      (5000U)

volatile uint8_t task2_alive = 0U;

void vTaskAnomalyDetect(void *pvParameters)
{
    (void)pvParameters;                 // Suppress unused parameter warning

    SensorData_t     data = {0};
    AnomalyLevel_t  level = ANOMALY_NONE;

    char msg[LOG_MSG_MAX_LEN];
    BaseType_t xRet = pdFALSE;

    while (1) 
    {   
        // Block on sensor queue
        xQueueReceive(xSensorQueue, &data, portMAX_DELAY);

        level = ANOMALY_NONE;

        // Check temp
        if (data.temp >= THRESH_TEMP_CRITICAL) {
            level = ANOMALY_CRITICAL;
            snprintf(msg, sizeof(msg), "[T2] CRITICAL: Temp %.1fC", data.temp);
        } else if (data.temp >= THRESH_TEMP_WARNING) {
            level = ANOMALY_WARNING;
            snprintf(msg, sizeof(msg), "[T2] WARNING: Temp %.1fC", data.temp);
        }

        // Check humidity
        if (data.humi >= THRESH_HUMI_HIGH) {
            level = ANOMALY_WARNING;
            snprintf(msg, sizeof(msg), "[T2] WARNING: Humi %.1f%%", data.humi);
        }

        // Check CO2
        if (data.co2 >= THRESH_CO2_CRITICAL) {
            level = ANOMALY_CRITICAL;
            snprintf(msg, sizeof(msg), "[T2] CRITICAL: CO2 %.0fppm", data.co2);
        } else if (data.co2 >= THRESH_CO2_WARNING) {
            level = ANOMALY_WARNING;
            snprintf(msg, sizeof(msg), "[T2] WARNING: CO2 %.0fppm", data.co2);
        }

        // Check PM2.5
        if (data.pm25 >= THRESH_PM25_HAZARDOUS) {
            level = ANOMALY_CRITICAL;
            snprintf(msg, sizeof(msg), "[T2] CRITICAL: PM25 %.1f", data.pm25);
        } else if (data.pm25 >= THRESH_PM25_WARNING) {
            level = ANOMALY_WARNING;
            snprintf(msg, sizeof(msg), "[T2] WARNING: PM25 %.1f", data.pm25);
        }

        // Check VOC
        if (data.voc < THRESH_VOC_POOR) {
            level = ANOMALY_WARNING;
            snprintf(msg, sizeof(msg), "[T2] WARNING: VOC %.0fΩ", data.voc);
        }

        // Check flame
        if (data.flame == 1U) {
            level = ANOMALY_CRITICAL;
            snprintf(msg, sizeof(msg), "[T2] CRITICAL: Flame detected");
        }

        // Drive alert GPIO pin
        if (level != ANOMALY_NONE) {
            alert_pin_set();    // HIGH — alert ESP32
            xQueueSend(xLogQueue, msg, 0U);
        } else {
            alert_pin_clear();   // LOW — all clear
            snprintf(msg, sizeof(msg), "[T2] All normal");
            xQueueSend(xLogQueue, msg, 0U);
        }

        task2_alive = 1U;       // Set alive flag
        snprintf(msg, sizeof(msg), "[T2] Sent alive heartbeat");
        xRet = xQueueSend(xLogQueue, (const void *)msg, 0U);
        if (xRet != pdTRUE) {
            /* Log queue full — drop message */
        }

        // Sleep until next read cycle
        //vTaskDelay(pdMS_TO_TICKS(ANOMALY_DETECT_TASK_PERIOD_MS));
    }
}

