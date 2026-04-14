#ifndef MODBUS_MASTER_H_
#define MODBUS_MASTER_H_

/**
 * @file  modbus_master.h
 * @brief 
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

void modbus_master_task_init(void);

#endif      // MODBUS_MASTER_H_