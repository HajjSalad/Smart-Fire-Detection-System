/**
 * @file  task_1_sensor_read.c
 * @brief Sensor data reading task
*/

#include "stm32f446xx.h"
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "tasks.h"

#include "spi_driver.h"
#include "sensor_drivers.h"
#include "shared_resources.h"

#define SENSOR_READ_TASK_PERIOD_MS    (5000U)
volatile uint8_t task1_alive = 0U;

/**
 * @brief Sensor read task entry point
 * 
 * Sensors Used:
 *  1. BME680:      Temperature, Pressure, Humidity & VOC
 *  2. Simulation:  CO2 & PM2.5
 *  3. GPIO Button: Flame 
*/
void vTaskSensorRead(void *pvParameters)
{
    (void)pvParameters;                 // Suppress unused parameter warning

    bme680_init();                      // Init BME680 - needs vTask for delay_us hence here after scheduler

    SensorData_t data = {0};
    char msg[LOG_MSG_MAX_LEN];
    BaseType_t xRet = pdFALSE;

    while (1) 
    {
        // 1. Sample all sensors
        bme680_read(&data.temp, &data.humi, &data.pres, &data.voc);
        gas_sensor_read(&data.co2);
        smoke_sensor_read(&data.pm25);
        data.flame = flame_detected;
        data.timestamp = xTaskGetTickCount();

        // 2. Write to shared struct - shared with modbus slave
        if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            shared_sensor_data = data;
            xSemaphoreGive(xSensorMutex);
        }

        // 3. Push to anomaly queue for vTaskAnomalyDetect (Task 2)
        xRet = xQueueSend(xSensorQueue, &data, pdMS_TO_TICKS(10));
        if (xRet != pdTRUE) {
            /* Queue full — drop reading */
        }

        // 4. Log to LogQueue
        snprintf(msg, sizeof(msg),
                 "[T1] Temp:%.1fC Hum:%.1f%% Pres:%.1fhPa VOC:%.1fΩ CO2:%.0fppm PM25:%.1f Flame:%d Time:%ld",
                 data.temp, data.humi, data.pres, data.voc, data.co2, data.pm25, data.flame, data.timestamp);

        xQueueSend(xLogQueue, msg, 0U);
        
        task1_alive = 1U;       // Set alive flag

        // Sleep until next read cycle
        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_TASK_PERIOD_MS));
    }

}