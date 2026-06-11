/**
 * @file  task_watchdog_monitor.c
 * @brief Watchdog timer task
 * 
 * Highest priority task — verifies all other tasks are alive
 * every 10 seconds by checking alive flags set by each task.
 * Kicks IWDG only if all tasks healthy. If any task fails to
 * set its flag — IWDG expires and resets the MCU.
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "iwdg_driver.h"
#include "tasks.h"
#include "shared_resources.h"

#define WATCHDOG_TIMER_TASK_PERIOD_MS      (10000U)

/**
 * @brief Watchdog monitor task entry point
 * 
 * Checks alive flags from Tasks 2-5 every 10 seconds.
 * Kicks IWDG if all flags set, logs fault and withholds
 * kick if any flag missing — triggering MCU reset via IWDG.
*/
void vTaskWatchdogMonitor(void *pvParameters)
{
    (void)pvParameters;                 // Suppress unused parameter warning

    char msg[LOG_MSG_MAX_LEN];
    BaseType_t xRet = pdFALSE;

    while (1) 
    {
        // snprintf(msg, sizeof(msg), "In Task WatchdogTimer");
        // xRet = xQueueSend(xLogQueue, (const void *)msg, 0U);
        // if (xRet != pdTRUE) {
        //     /* Log queue full — drop message */
        // }

        // Check all tasks are alive
        if (task1_alive && task2_alive && task3_alive) 
        {
            iwdg_kick();            // Kick watchdog - all tasks healthy

            // Reset flags
            task1_alive = 0U;
            task2_alive = 0U;
            task3_alive = 0U;
            task4_alive = 0U;

            snprintf(msg, sizeof(msg), "[WDM] All tasks alive — kicked");
            xRet = xQueueSend(xLogQueue, (const void *)msg, 0U);
            if (xRet != pdTRUE) { /* drop */ }
        }
        else 
        {
            // Log which tasks responses
            snprintf(msg, sizeof(msg), "[WDM] FAULT T1:%d T2:%d T3:%d T4:%d",
                     task1_alive, task2_alive, task3_alive, task4_alive);
            xRet = xQueueSend(xLogQueue, (const void *)msg, 0U);
            if (xRet != pdTRUE) { /* drop */ }

            // IWDG NOT kicked - watchdog will expire and reset MCU
        }
        
        // Sleep until next read cycle
        vTaskDelay(pdMS_TO_TICKS(WATCHDOG_TIMER_TASK_PERIOD_MS));
    }
}
