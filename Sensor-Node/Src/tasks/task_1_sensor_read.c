/**
 * @file  task_1_sensor_read.c
 * @brief Sensor data reading task
*/

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "tasks.h"
#include "sensor_drivers.h"
#include "shared_resources.h"

#define SENSOR_READ_TASK_PERIOD_MS    (5000U)

/**
 * @brief Sensor read task entry point
 * 
 * Sensors Used:
 *  1. TMP102:      Temp
 *  2. BME680:      Temperature, Pressure, Humidity & VOC
 *  3. Simulation:  CO2 & PM2.5
 *  4. GPIO Button: Flame 
*/
void vTaskSensorRead(void *pvParameters)
{
    (void)pvParameters;                 // Suppress unused parameter warning

    SensorData_t data = {0};
    char msg[LOG_MSG_MAX_LEN];
    BaseType_t xRet = pdFALSE;

    while (1) 
    {
        // snprintf(msg, sizeof(msg), "Reading TMP102");
        // xRet = xQueueSend(xLogQueue, (const void *)msg, 0U);
        // if (xRet != pdTRUE) {
        //     /* Log queue full — drop message */
        // }

        // 1. Sample all sensors
        //tmp102_read(&data.temp);
        gas_sensor_read(&data.co2);
        smoke_sensor_read(&data.pm25);
        //flame_sensor_read(&data.flame);
        data.timestamp = xTaskGetTickCount();

        // 2. Write to shared struct
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            shared_sensor_data = data;
            xSemaphoreGive(xSensorMutex);
        }

        // 3. Push to anomaly queue for Task 2
        xRet = xQueueSend(xSensorQueue, &data, pdMS_TO_TICKS(10));
        if (xRet != pdTRUE) {
            /* Queue full — drop reading */
        }

        // 4. Log 
        snprintf(msg, sizeof(msg),
                 "[T1] Temp:%.1fC Hum:%.1f%% Pres:%.1fhPa VOC:%.1fΩ CO2:%.0fppm PM25:%.1f Flame:%d Time:%ld",
                 data.temp, data.humi, data.pres, data.voc, data.co2, data.pm25, data.flame, data.timestamp);

        xQueueSend(xLogQueue, msg, 0U);
        
        // Sleep until next read cycle
        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_TASK_PERIOD_MS));
    }

}