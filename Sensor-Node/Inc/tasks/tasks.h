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

// Function Prototypes
void vTaskSensorRead(void *pvParameters);
void vTaskAnomalyDetect(void *pvParameters);
void vTaskModbusSlave(void *pvParameters);
void vTaskLogger(void *pvParameters);

#endif // TASKS_H 