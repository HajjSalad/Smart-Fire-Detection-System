/**
 * @file  task_3_modbus_slave.c
 * @brief Modbus RTU Slave task
 * 
 * - Polls UART ring buffer for incoming bytes
 * - Place the bytes in a frame
 * - Process the modbus frames
 * - Builds the modbus response
 * - 
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "tasks.h"
#include "crc_16.h"
#include "uart_driver.h"
#include "modbus_register.h"
#include "shared_resources.h"

#define MODBUS_SLAVE_TASK_PERIOD_MS      (5000U)
#define MODBUS_MAX_FRAME_LEN              256U
#define MODBUS_MIN_FRAME_LEN              4U      // addr + FC + DATA + CRC          
#define MODBUS_FRAME_TIMEOUT_MS           2U      // 3.5 char @ 115200 ≈ 305µs

// Function Prototypes
static void modbus_process_frame(uint8_t *frame, uint8_t len);
static uint8_t modbus_build_read_response(uint8_t *resp, uint16_t start_addr, uint16_t quantity);
static void modbus_build_exception_response(uint8_t func_code, uint8_t except_code);
static void modbus_send_response(uint8_t *resp, uint8_t len);

/**
 * @brief MODBUS slave task entry point
*/
void vTaskModbusSlave(void *pvParameters)
{
    (void)pvParameters;                 // Suppress unused parameter warning

    uint8_t  byte                        = 0U;
    uint8_t  frame_len                   = 0U;
    uint32_t last_rx_tick                = 0U;
    uint8_t  frame[MODBUS_MAX_FRAME_LEN] = {0};

    BaseType_t xRet = pdFALSE;
    char msg[LOG_MSG_MAX_LEN];

    while (1) 
    {
        // snprintf(msg, sizeof(msg), "In Task 3");
        // xRet = xQueueSend(xLogQueue, (const void *)msg, 0U);
        // if (xRet != pdTRUE) {
        //     /* Log queue full — drop message */
        // }

        // Poll ring buffer
        if (uart1_ring_buffer_read(&byte))              // new byte found
        {
            // Byte received - add to frame buffer
            if (frame_len < MODBUS_MAX_FRAME_LEN) {
                frame[frame_len++] = byte;
            }
            last_rx_tick = xTaskGetTickCount();         // record timestamp
        } 
        else                                            // no new byte
        {
            // No new byte - Check silence gap 
            if (frame_len > 0 && (xTaskGetTickCount() - last_rx_tick) >= MODBUS_FRAME_TIMEOUT_MS)
            {
                // Silence detected: Frame receiving complete - Can process the frame now
                snprintf(msg, sizeof(msg), "[T3] Frame received, %d bytes", frame_len);
                xQueueSend(xLogQueue, msg, 0U);

                // Process the frame
                modbus_process_frame(frame, frame_len);

                // Reset for next frame
                memset(frame, 0, sizeof(frame));
                frame_len = 0U;
            }
        }
        // Sleep until next read cycle
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief 
 * 
 * Received frame format:
 * [slave_addr][FC][start_addr_H][start_addr_L][quantity_H][quantity_L][CRC_L][CRC_H]
*/
static void modbus_process_frame(uint8_t *frame, uint8_t len)
{
    char msg[LOG_MSG_MAX_LEN];

    // 1. Validate the input args
    if (len < MODBUS_MIN_FRAME_LEN) {
        snprintf(msg, sizeof(msg), "[T3] Frame too short: %d bytes", len);
        xQueueSend(xLogQueue, msg, 0U);
        return;
    }

    // 2. Check slave address - Continue only if correct address, ignore otherwise
    if (frame[SLAVE_ADDR_POS] != MODBUS_SLAVE_ADDR ) {
        return;
    }

    // 3. Validate CRC
    uint16_t crc_received   = (uint16_t)(frame[len-2]) | ((uint16_t)(frame[len-1]) << 8U);
    uint16_t crc_calculated = compute_crc(frame, len - 2U);
    if (crc_received != crc_calculated) {
        snprintf(msg, sizeof(msg), "[T3] CRC fail — rx:0x%04X calc:0x%04X",
                 crc_received, crc_calculated);
        xQueueSend(xLogQueue, msg, 0U);
        return;
    }

    // 4. Parse function code
    uint8_t func_code = frame[FC_ADDR_POS];

    switch(func_code)
    {
        case MODBUS_FC_READ_HOLDING:        // Master requests sensor data from slave
        {
            uint16_t start_addr = (uint16_t)(frame[2] << 8U) | frame[3];
            uint16_t quantity   = (uint16_t)(frame[4] << 8U) | frame[5];

            // 1. Validate register range
            if (start_addr + quantity > REG_COUNT) {
                // Send exception - Register addr out of range
                modbus_build_exception_response(func_code, MODBUS_EX_ILLEGAL_ADDRESS);
                return;
            }

            // 2. Build and send response
            uint8_t resp[32] = {0};
            uint8_t resp_len = modbus_build_read_response(resp, start_addr, quantity);
            modbus_send_response(resp, resp_len);

            // Log
            snprintf(msg, sizeof(msg), "[T3] Read regs addr:0x%04X qty:%d", 
                     start_addr, quantity);
            xQueueSend(xLogQueue, msg, 0U);
            break;
        }
        case MODBUS_FC_WRITE_SINGLE:        // Master writes a value into a slave register
        {
            break;
        }
        default:
            modbus_build_exception_response(func_code, MODBUS_EX_ILLEGAL_FUNCTION);
            break;
    }
}

/**
 * @brief Build Function Code: FC 0x03 read holding registers response
 * 
 * Response format:
 * [slave_addr][FC][byte_count][reg0_H][reg0_L]...[regN_H][regN_L][CRC_H][CRC_L]
*/
static uint8_t modbus_build_read_response(uint8_t *resp, uint16_t start_addr, uint16_t quantity)
{
    SensorData_t data = {0};

    // 1. Get a snapshot of the shared sensor struct
    if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        data = shared_sensor_data;
        xSemaphoreGive(xSensorMutex);
    }

    // 2. Scale floats to uint16_t registers for transportation
    uint16_t regs[REG_COUNT];
    regs[REG_TEMPERATURE] = FLOAT_TO_REG(data.temp, SCALE_TEMP);
    regs[REG_HUMIDITY]    = FLOAT_TO_REG(data.humi, SCALE_HUMI);
    regs[REG_PRESSURE]    = FLOAT_TO_REG(data.pres, SCALE_PRES);
    regs[REG_VOC]         = FLOAT_TO_REG(data.voc,  SCALE_VOC);
    regs[REG_CO2]         = FLOAT_TO_REG(data.co2,  SCALE_CO2);
    regs[REG_PM25]        = FLOAT_TO_REG(data.pm25, SCALE_PM25);
    regs[REG_FLAME]       = (uint16_t)data.flame;

    // 3. Build response frame
    uint8_t idx = 0U;
    resp[idx++] = MODBUS_SLAVE_ADDR;            // at 0
    resp[idx++] = MODBUS_FC_READ_HOLDING;       // at 1
    resp[idx++] = (uint8_t)(quantity * 2U);     // byte count = quantity * 2

    // 4. Pack only requested registers (big-endian): start_addr to start_addr+quantity
    for (uint16_t i = start_addr; i < start_addr + quantity; i++) {
        resp[idx++] = (uint8_t)(regs[i] >> 8U);         // high byte
        resp[idx++] = (uint8_t)(regs[i] & 0xFFU);       // low byte
    }

    // 5. Append CRC
    uint16_t crc = compute_crc(resp, idx);
    resp[idx++] = (uint8_t)(crc & 0xFFU);               // low byte
    resp[idx++] = (uint8_t)(crc >> 8U);                 // high byte
    

    return idx;         // Return length of response frame
}

/**
 * @brief Build response for MODBUS exception
 */
static void modbus_build_exception_response(uint8_t func_code, uint8_t except_code)
{
    uint8_t resp[5] = {0};
    uint8_t idx     = 0U;

    // 1. Pack info
    resp[idx++] = MODBUS_SLAVE_ADDR;            // at 0
    resp[idx++] = func_code;                    // at 1
    resp[idx++] = except_code;                  // at 2

    // 2. Append CRC
    uint16_t crc = compute_crc(resp, idx);
    resp[idx++] = (uint8_t)(crc & 0xFFU);               // low byte
    resp[idx++] = (uint8_t)(crc >> 8U);                 // high byte

    modbus_send_response(resp, idx);
}

/**
 * @brief Transmit response frame over UART1
*/
static void modbus_send_response(uint8_t *resp, uint8_t len)
{
    for (uint8_t i = 0U; i < len; i++) {
        uart1_write(resp[i]);               // Send one byte at a time
    }
}
