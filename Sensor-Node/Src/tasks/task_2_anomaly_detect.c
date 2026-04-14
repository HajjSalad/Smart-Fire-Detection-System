/**
 * @file  task_2_anomaly_detect.c
 * @brief 
 * 
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "uart_driver.h"
#include "tasks.h"
#include "shared_resources.h"

#define ANOMALY_DETECT_TASK_PERIOD_MS      (5000U)

void vTaskAnomalyDetect(void *pvParameters)
{
    (void)pvParameters;                 // Suppress unused parameter warning

    char msg[LOG_MSG_MAX_LEN];
    BaseType_t xRet = pdFALSE;

    while (1) 
    {
        // snprintf(msg, sizeof(msg), "In Task 2");
        // xRet = xQueueSend(xLogQueue, (const void *)msg, 0U);
        // if (xRet != pdTRUE) {
        //     /* Log queue full — drop message */
        // }
        
        // Sleep until next read cycle
        vTaskDelay(pdMS_TO_TICKS(ANOMALY_DETECT_TASK_PERIOD_MS));
    }
}

