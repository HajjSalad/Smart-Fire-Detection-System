/**
 * @file  task_4_system_logger.c
 * @brief System logger task
 * 
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "tasks.h"
#include "uart_driver.h"
#include "shared_resources.h"

#define LOGGER_TASK_PERIOD_MS      (5000U)

void vTaskLogger(void *pvParameters)
{
    (void)pvParameters;                 // Suppress unused parameter warning

    char msg[LOG_MSG_MAX_LEN];
    BaseType_t ret;

    while (1) 
    {
        ret = xQueueReceive(xLogQueue, msg, portMAX_DELAY);
        if (ret == pdTRUE) {
            msg[LOG_MSG_MAX_LEN - 1] = '\0';  // Ensure null termination
            printf("%s\n\r", msg);
        }
        
        // // Sleep until next read cycle
        // vTaskDelay(pdMS_TO_TICKS(LOGGER_TASK_PERIOD_MS));
    }
}

