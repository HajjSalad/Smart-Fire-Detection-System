/**
 * @file  task_1_sensor_read.c
 * @brief Sensor data reading task
*/

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "tasks.h"

#include "spi_driver.h"
#include "sensor_drivers.h"
#include "shared_resources.h"

#define SENSOR_READ_TASK_PERIOD_MS    (5000U)

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

    uint8_t chip_id = spi1_read_reg(0xD0);
    printf("BME680 chip ID: 0x%02X (expected 0x61)\n\r", chip_id);
    bme680_init();                      // Init BME680 - needs vTask for us hence here after scheduler

    SensorData_t data = {0};
    char msg[LOG_MSG_MAX_LEN];
    BaseType_t xRet = pdFALSE;

    while (1) 
    {
        // snprintf(msg, sizeof(msg), "In Task 1");
        // xRet = xQueueSend(xLogQueue, (const void *)msg, 0U);
        // if (xRet != pdTRUE) {
        //     /* Log queue full — drop message */
        // }

        // Test CS toggle manually
        printf("CS going LOW\n\r");
        spi1_cs_low();
        vTaskDelay(pdMS_TO_TICKS(100));
        printf("CS going HIGH\n\r");
        spi1_cs_high();
        vTaskDelay(pdMS_TO_TICKS(100));

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
        
        // Sleep until next read cycle
        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_TASK_PERIOD_MS));
    }

}