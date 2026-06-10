/**
 * @file  task_3_modbus_slave.c
 * @brief Modbus RTU Slave task
 * 
 * - Polls UART ring buffer for incoming bytes
 * - Place the bytes in a frame
 * - Process the modbus frames
 * - Builds the modbus response
 * - Send response to master
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

#define REQUEST_MIN_FRAME_LEN             8U      // addr(1) + FC(1) + start_addr(2) + quantity(2) + CRC(2) = 8          
#define MODBUS_FRAME_TIMEOUT_MS           3U      // 3.5 char @ 115200 ≈ 305µs -> 3ms would be sufficient

volatile uint8_t task3_alive = 0U;        // Watchdog heartbeat for Task 3

// Task Handle for vTaskModbusSlave
TaskHandle_t xModbusTaskHandle = NULL;

// Static Function Prototypes
static Modbus_Status_t modbus_process_frame(Modbus_Context_t *mctx);
static Modbus_Status_t modbus_send_response(Modbus_Context_t *mctx);
static Modbus_Status_t modbus_build_read_response(Modbus_Context_t *mctx, uint16_t start_addr, uint16_t quantity);
static Modbus_Status_t modbus_build_exception_response(Modbus_Context_t *mctx, uint8_t func_code, uint8_t except_code);
static Modbus_Status_t modbus_transmit(Modbus_Context_t *mctx);

/**
 * @brief MODBUS RTU slave task
 * 
 * - Place the bytes in a frame
 * - Process the modbus frames
 * - Builds the modbus response
 * - Send response to master
*/
void vTaskModbusSlave(void *pvParameters)
{
    (void)pvParameters;                 // Suppress unused parameter warning

    Modbus_Context_t mctx = {
        .state     = MODBUS_STATE_IDLE,
        .frame_len = 0U,
        .resp_len  = 0U
    };

    uint8_t byte = 0U;
    BaseType_t xRet;
    char msg[LOG_MSG_MAX_LEN];

    while(1) 
    {
        switch (mctx.state)
        {
            case MODBUS_STATE_IDLE:
                // Block until ISR notifies - first byte arrives
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                mctx.state = MODBUS_STATE_RECEIVING;
                break;
            
            case MODBUS_STATE_RECEIVING:
                // Drain ring buffer
                do {
                    while (uart1_ring_buffer_read(&byte)) {
                        if (mctx.frame_len < REQUEST_MAX_FRAME_LEN) {
                            mctx.frame[mctx.frame_len++] = byte;
                        }
                    }
                } while (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(MODBUS_FRAME_TIMEOUT_MS)) > 0);

                // PROCESSING if valid length, IDLE if invalid length
                mctx.state = (mctx.frame_len > 0) ? MODBUS_STATE_PROCESSING : MODBUS_STATE_IDLE;
                break;

            case MODBUS_STATE_PROCESSING:
                // Process frame
                if (modbus_process_frame(&mctx) == MODBUS_OK) {
                    mctx.state = MODBUS_STATE_RESPONDING;
                } else {
                    mctx.state = MODBUS_STATE_ERROR;
                }
                break;
            
            case MODBUS_STATE_RESPONDING:
                // Respond to master
                if (modbus_send_response(&mctx) == MODBUS_OK) {
                    mctx.state = MODBUS_STATE_RESPONDED;
                } else {
                    mctx.state = MODBUS_STATE_ERROR;
                }
                break;
            
            case MODBUS_STATE_RESPONDED:
                // Reset for next frame
                memset(&mctx, 0, sizeof(mctx));
                mctx.state = MODBUS_STATE_IDLE;

                // Set watchdog heartbeat flag
                task3_alive = 1U;

                // Log
                snprintf(msg, sizeof(msg), "[T3] Response sent to master");
                xQueueSend(xLogQueue, msg, 0U);

                break;

            case MODBUS_STATE_ERROR:
                // Reset for next frame
                memset(&mctx, 0, sizeof(mctx));
                mctx.state = MODBUS_STATE_IDLE;

                // Set watchdog heartbeat flag
                task3_alive = 1U;

                // Log
                snprintf(msg, sizeof(msg), "[T3] Frame error — discarded");
                xQueueSend(xLogQueue, msg, 0U);

                break;

            default:
                mctx.state = MODBUS_STATE_IDLE;
                break;
        }
    }
}

/**
 * @brief Process the request frame received from master
 * 
 * Received request frame format:
 * [slave_addr]  [FC]   [start_addr_H][start_addr_L][quantity_H][quantity_L][CRC_L][CRC_H]   
 *     8 bit    8 bit              16 bit                    16 bit           16 bit            total = 8 bytes
*/
static Modbus_Status_t modbus_process_frame(Modbus_Context_t *mctx)
{
    char msg[LOG_MSG_MAX_LEN];
    uint8_t len = mctx->frame_len;

    // 1. Validate the input args
    if (len < REQUEST_MIN_FRAME_LEN) {
        snprintf(msg, sizeof(msg), "[T3] Request frame received too short: %d bytes", len);
        xQueueSend(xLogQueue, msg, 0U);
        return MODBUS_ERROR;
    }

    // 2. Check slave address - Continue only if correct address, ignore otherwise
    if (mctx->frame[SLAVE_ADDR_POS] != MODBUS_SLAVE_ADDR ) {
        return MODBUS_ERROR;
    }

    // 3. Validate CRC
    uint16_t crc_received   = (uint16_t)(mctx->frame[len-2]) | ((uint16_t)(mctx->frame[len-1]) << 8U);
    uint16_t crc_calculated = compute_crc(mctx->frame, len - 2U);
    if (crc_received != crc_calculated) {
        snprintf(msg, sizeof(msg), "[T3] CRC fail — rx:0x%04X calc:0x%04X",
                 crc_received, crc_calculated);
        xQueueSend(xLogQueue, msg, 0U);
        return MODBUS_ERROR;
    }

    // 4. Log request frame received 
    char frame_log[LOG_MSG_MAX_LEN] = {0};
    char temp[8] = {0};

    snprintf(frame_log, sizeof(frame_log), "[T3] Request frame received  : ");
    for (int i = 0; i < len; i++) {
        snprintf(temp, sizeof(temp), "%02X ", mctx->frame[i]);
        strncat(frame_log, temp, sizeof(frame_log) - strlen(frame_log) - 1);
    }
    xQueueSend(xLogQueue, frame_log, 0U);

    return MODBUS_OK;
}

/**
 * @brief Build and send response to master request
 * 
 * Parses function code from validated frame, builds appropriate 
 * response into mctx->resp[], transmits over UART1, and logs.
*/
static Modbus_Status_t modbus_send_response(Modbus_Context_t *mctx) 
{
    if (!mctx) return MODBUS_ERROR;

    // Parse function code
    uint8_t func_code = mctx->frame[FC_ADDR_POS];

    switch(func_code)
    {
        case MODBUS_FC_READ_HOLDING:        // Master requests sensor data from slave
        {
            uint16_t start_addr = (uint16_t)(mctx->frame[2] << 8U) | mctx->frame[3];
            uint16_t quantity   = (uint16_t)(mctx->frame[4] << 8U) | mctx->frame[5];

            // Validate register range
            if (start_addr + quantity > REG_COUNT) {    // Exception - Register addr out of range
                // Build exception response into mctx->resp[]
                if (modbus_build_exception_response(mctx, func_code, MODBUS_EX_ILLEGAL_ADDRESS) != MODBUS_OK) {
                    return MODBUS_ERROR;
                }

                // Transmit the exception response
                if (modbus_transmit(mctx) != MODBUS_OK) {
                    return MODBUS_ERROR;
                }

                return MODBUS_OK;
            }

            // Build read response into mctx->resp[]
            if (modbus_build_read_response(mctx, start_addr, quantity) != MODBUS_OK) {
                return MODBUS_ERROR;
            }

            // Transmit the read response
            if (modbus_transmit(mctx) != MODBUS_OK) {
                return MODBUS_ERROR;
            }

            // Log read response sent
            char frame_log[LOG_MSG_MAX_LEN] = {0};
            char temp[8] = {0};

            snprintf(frame_log, sizeof(frame_log), "[T3] Read response frame sent: ");
            for (int i = 0; i < mctx->resp_len; i++) {
                snprintf(temp, sizeof(temp), "%02X ", mctx->resp[i]);
                strncat(frame_log, temp, sizeof(frame_log) - strlen(frame_log) - 1);
            }
            xQueueSend(xLogQueue, frame_log, 0U);

            break;
        }
        case MODBUS_FC_WRITE_SINGLE:        // Master writes a value into a slave register
            // Future implementation
            break;

        default:                            // Exception - Unsupported function code
            // Build exception response into mctx->resp[]
            if (modbus_build_exception_response(mctx, func_code, MODBUS_EX_ILLEGAL_FUNCTION) != MODBUS_OK) {
                return MODBUS_ERROR;
            }

            // Transmit the exception response
            if (modbus_transmit(mctx) != MODBUS_OK) {
                return MODBUS_ERROR;
            }
            break;
    }

    return MODBUS_OK;
}

/**
 * @brief Build Function Code: FC 0x03 read holding registers response
 * 
 * Response frame format:
 * [slave_addr][FC][byte_count][reg0_H][reg0_L]...[regN_H][regN_L][CRC_H][CRC_L]
*/
static Modbus_Status_t modbus_build_read_response(Modbus_Context_t *mctx, uint16_t start_addr, uint16_t quantity)
{
    if (!mctx) return MODBUS_ERROR;

    SensorData_t data = {0};

    // 1. Get a snapshot of the shared sensor struct
    if (xSemaphoreTake(xSensorMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        data = shared_sensor_data;
        xSemaphoreGive(xSensorMutex);
    }

    // 2. Scale floats to uint16_t registers for transmission
    uint16_t regs[REG_COUNT];
    regs[REG_TEMPERATURE] = FLOAT_TO_REG(data.temp, SCALE_TEMP);
    regs[REG_HUMIDITY]    = FLOAT_TO_REG(data.humi, SCALE_HUMI);
    regs[REG_PRESSURE]    = FLOAT_TO_REG(data.pres, SCALE_PRES);
    regs[REG_VOC]         = FLOAT_TO_REG(data.voc,  SCALE_VOC);
    regs[REG_CO2]         = FLOAT_TO_REG(data.co2,  SCALE_CO2);
    regs[REG_PM25]        = FLOAT_TO_REG(data.pm25, SCALE_PM25);
    regs[REG_FLAME]       = (uint16_t)data.flame;

    // 3. Build response frame
    mctx->resp[mctx->resp_len++] = MODBUS_SLAVE_ADDR;            // at 0
    mctx->resp[mctx->resp_len++] = MODBUS_FC_READ_HOLDING;       // at 1
    mctx->resp[mctx->resp_len++] = (uint8_t)(quantity * 2U);     // byte count = quantity * 2

    // 4. Pack only requested registers (big-endian): start_addr to start_addr+quantity
    for (uint16_t i = start_addr; i < start_addr + quantity; i++) {
        mctx->resp[mctx->resp_len++] = (uint8_t)(regs[i] >> 8U);         // high byte
        mctx->resp[mctx->resp_len++] = (uint8_t)(regs[i] & 0xFFU);       // low byte
    }

    // 5. Append CRC
    uint16_t crc = compute_crc(mctx->resp, mctx->resp_len);
    mctx->resp[mctx->resp_len++] = (uint8_t)(crc & 0xFFU);               // low byte
    mctx->resp[mctx->resp_len++] = (uint8_t)(crc >> 8U);                 // high byte

    return MODBUS_OK;
}

/**
 * @brief Build response for MODBUS exception
*/
static Modbus_Status_t modbus_build_exception_response(Modbus_Context_t *mctx, uint8_t func_code, uint8_t except_code)
{
    char msg[LOG_MSG_MAX_LEN];

    // 1. Pack info
    mctx->resp[mctx->resp_len++] = MODBUS_SLAVE_ADDR;            // at 0
    mctx->resp[mctx->resp_len++] = func_code | 0x80U;            // at 1
    mctx->resp[mctx->resp_len++] = except_code;                  // at 2

    // 2. Append CRC
    uint16_t crc = compute_crc(mctx->resp, mctx->resp_len);
    mctx->resp[mctx->resp_len++] = (uint8_t)(crc & 0xFFU);               // low byte
    mctx->resp[mctx->resp_len++] = (uint8_t)(crc >> 8U);                 // high byte

    // Log exception response sent
    char frame_log[LOG_MSG_MAX_LEN] = {0};
    char temp[8] = {0};

    snprintf(frame_log, sizeof(frame_log), "[T3] Exception response frame sent: ");
    for (int i = 0; i < mctx->resp_len; i++) {
        snprintf(temp, sizeof(temp), "%02X ", mctx->resp[i]);
        strncat(frame_log, temp, sizeof(frame_log) - strlen(frame_log) - 1);
    }
    xQueueSend(xLogQueue, frame_log, 0U);

    return MODBUS_OK;
}

/**
 * @brief Transmit response frame over UART1 byte by byte
*/
static Modbus_Status_t modbus_transmit(Modbus_Context_t *mctx)
{
    // Send one byte at a time
    for (uint8_t i = 0U; i < mctx->resp_len; i++) {
        uart1_write(mctx->resp[i]);     
    }

    return MODBUS_OK;
}
