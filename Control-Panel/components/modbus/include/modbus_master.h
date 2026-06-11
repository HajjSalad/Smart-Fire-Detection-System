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

#define REQUEST_MAX_FRAME_LEN               32U 
#define RESPONSE_MAX_FRAME_LEN              32U
#define RESPONSE_MIN_FRAME_LEN              5U          // exception response is smallest = 5

typedef struct {
    uint8_t request[REQUEST_MAX_FRAME_LEN];
    uint8_t req_len;
    uint8_t response[RESPONSE_MAX_FRAME_LEN];
    uint8_t resp_len;
    Modbus_Master_State_t state;
} Modbus_Master_Context_t;

typedef enum {
    MODBUS_MASTER_STATE_IDLE,
    MODBUS_MASTER_STATE_REQUESTING,
    MODBUS_MASTER_STATE_WAITING,
    MODBUS_MASTER_STATE_PROCESSING,
    MODBUS_MASTER_STATE_COMPLETE,
    MODBUS_MASTER_STATE_ERROR,
} Modbus_Master_State_t;

typedef enum {
    MODBUS_MASTER_OK    = 0,
    MODBUS_MASTER_ERROR = 1,
} Modbus_Master_Status_t;

#endif      // MODBUS_MASTER_H_