/**
 * @file  main.c
 * @brief Main entry point for the STM32 Sensor Node application.
 * 
 * Initializes peripherals, creates FreeRTOS resources (mutexes, queues), 
 * spawns all application tasks, and starts the scheduler.
*/

#include <stdint.h>

#include "stm32f446xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "tasks.h"
#include "sensor_drivers.h"
#include "i2c_driver.h"
#include "uart_driver.h"
#include "shared_resources.h"

// Local function prototypes
static void check_reset_cause(void);

QueueHandle_t        xSensorQueue       = NULL;      
QueueHandle_t        xLogQueue          = NULL;
SemaphoreHandle_t    xSensorMutex       = NULL;

SensorData_t         shared_sensor_data = {0};

/**
 * @brief FreeRTOS stack overflow hook.
 * 
 * Called automatically when stack overflow is detected. 
 * Logs the offending task name and halts the system.
*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;                // Suppress unused parameter warning
    uart2_write('!');           // Indicate stack overflow error
    while(1) {}
}

/**
 * @brief Application entry point.
 * 
 * Initializes UART peripherals, creates FreeRTOS synchronization
 * primitives and tasks, then starts the scheduler.
*/
int main(void) 
{
    BaseType_t xRet = pdFALSE;

    uart2_init();               // Initialize UART2 for logging
    uart1_init();               // Initialize UART1 for ESP32 communication
    i2c1_init();                //
    tmp102_init();

    // Initialize sensor drivers
    // flame_sensor_init();
    

    check_reset_cause();        // Log the cause of the last reset

    LOG("*** STM32 Sensor Node Starting ***");

    xSensorMutex = xSemaphoreCreateMutex();
    configASSERT(xSensorMutex != NULL);

    xSensorQueue = xQueueCreate(SENSOR_QUEUE_DEPTH, sizeof(SensorData_t));
    configASSERT(xSensorQueue != NULL);

    xLogQueue = xQueueCreate(LOG_QUEUE_DEPTH, LOG_MSG_MAX_LEN);
    configASSERT(xLogQueue != NULL);

    /**
     * Priorities  (in FreeRTOS, 0 = lowest priority)
     * MAX_PRIORITIES = 16
     * 
     * Priority 6  → Sensor read
     * Priority 5  → Anomaly detect
     * Priority 4  → Modbus Slave
     * Priority 3  → Logger
     * Priority 0  → Idle task
    */

    // Create task
    xRet = xTaskCreate(vTaskSensorRead,    "SensorRead",    512, NULL, 6, NULL);
    configASSERT(xRet == pdPASS);
    xRet = xTaskCreate(vTaskAnomalyDetect, "AnomalyDetect", 512, NULL, 5, NULL);
    configASSERT(xRet == pdPASS);
    xRet = xTaskCreate(vTaskModbusSlave,    "ModbusSlave",  512, NULL, 4, NULL);
    configASSERT(xRet == pdPASS);
    xRet = xTaskCreate(vTaskLogger,         "Logger",       512, NULL, 3, NULL);
    configASSERT(xRet == pdPASS);

    LOG("Tasks created. Free heap: %u bytes", xPortGetFreeHeapSize());
    LOG("Starting scheduler...");

    vTaskStartScheduler();  

    // Should never reach here - halt if scheduler exits
    LOG("Scheduler exited unexpectedly!");
    while (1) {}
}

/**
 * @brief Check reset cause and log it over UART.
 * Must be called before any other initialization to ensure accurate logging of reset causes.
*/
static void check_reset_cause(void) 
{
    uint32_t cause = RCC->CSR;
    RCC->CSR |= RCC_CSR_RMVF;           // Clear reset flags

    if (cause & RCC_CSR_IWDGRSTF) { LOG("Reset: Watchdog"); }
    if (cause & RCC_CSR_SFTRSTF)  { LOG("Reset: Software"); }
    if (cause & RCC_CSR_PORRSTF)  { LOG("Reset: Power-On"); }
    if (cause & RCC_CSR_PINRSTF)  { LOG("Reset: External Pin"); }
}