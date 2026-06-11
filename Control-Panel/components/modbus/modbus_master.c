/**
 * @file  modbus_master.c
 * @brief 
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_log.h"
#include <stdint.h>
#include "string.h"

#include "crc_16.h"
#include "uart_driver.h"
#include "modbus_master.h"
#include "modbus_register.h"

#define MODBUS_RESPONSE_TIMEOUT_MS          1000U
#define MODBUS_MASTER_SAMPLE_PERIOD         5000U

// Static Function Prototypes
Modbus_Master_Status_t modbus_build_read_request(Modbus_Master_Context_t *mctx);
Modbus_Master_Status_t modbus_parse_received_response(Modbus_Master_Context_t *mctx);

static void modbus_master_task(void *pvParameters)
{
    ESP_ERROR_CHECK(modbus_uart2_init());                // Initialize UART2 for modbus comm with STM32

    Modbus_Master_Context_t mctx = {
        .req_len  = 0U,
        .resp_len = 0U,
        .state    = MODBUS_MASTER_STATE_IDLE,
    };

    while(1) 
    {
        switch(mctx.state) 
        {
            case MODBUS_MASTER_STATE_IDLE:
                // Reset context for new transaction
                memset(&mctx, 0, sizeof(mctx));
                mctx.state = MODBUS_MASTER_STATE_REQUESTING;
                break;

            case MODBUS_MASTER_STATE_REQUESTING:
                // Build read request 
                if (modbus_build_read_request(&mctx) != MODBUS_MASTER_OK) {
                    printf("Error building read request\n");
                    mctx.state = MODBUS_MASTER_STATE_ERROR;
                    break;
                }

                // Transmit read request
                int written = uart_write_bytes(UART_NUM2, mctx.request, mctx.req_len);
                if (written != (int)mctx.req_len) {
                    printf("Error sending read request\n");
                    mctx.state = MODBUS_MASTER_STATE_ERROR;
                }

                // Log request frame sent
                printf("Request frame sent: ");
                for (int i = 0; i < req_len; i++) {
                    printf("%02X ", request[i]);
                }
                printf("\n");
                
                mctx.state = MODBUS_MASTER_STATE_WAITING;
                break;

            case MODBUS_MASTER_STATE_WAITING:
                // Block until response received or timeout
                int received = uart_read_bytes(UART_NUM2, mctx.response, mctx.resp_len,
                                        pdMS_TO_TICKS(MODBUS_RESPONSE_TIMEOUT_MS));
                if (received <= 0) {
                    printf("Timeout - no response from slave\n\n");
                    mctx.state = MODBUS_MASTER_STATE_ERROR;
                    break;
                }

                mctx.resp_len = (uint8_t)received;
                mctx.state    = MODBUS_MASTER_STATE_PROCESSING;
                break;

            case MODBUS_MASTER_STATE_PROCESSING:
                // Validate and parse response
                if (modbus_parse_received_response(&mctx) != MODBUS_MASTER_OK) {
                    mctx.state = MODBUS_MASTER_STATE_ERROR;
                    break;
                }

                mctx.state = MODBUS_MASTER_STATE_COMPLETE;
                break;

            case MODBUS_MASTER_STATE_COMPLETE:
                printf("Transaction Complete !\n");

                // Delay before next cycle
                vTaskDelay(pdMS_TO_TICKS(MODBUS_MASTER_SAMPLE_PERIOD));
                mctx.state = MODBUS_MASTER_STATE_IDLE;
                
                break;

            case MODBUS_MASTER_STATE_ERROR:
                printf("MODBUS master error - retrying next cycle !\n"); 

                // Delay before retry
                vTaskDelay(pdMS_TO_TICKS(MODBUS_MASTER_SAMPLE_PERIOD));
                mctx.state = MODBUS_MASTER_STATE_IDLE;

                break;

            default:
                mctx.state = MODBUS_MASTER_STATE_IDLE;
                break;
        }
    }
}
/**
 * @brief Build FC 0x03 read holding registers request
 * 
 * Read request frame format:
 * [slave_addr][FC][start_addr_H][start_addr_L][quantity_H][quantity_L][CRC_L][CRC_H]
*/
Modbus_Master_Status_t modbus_build_read_request(Modbus_Master_Context_t *mctx)
{
    mctx->request[mctx->req_len++] = MODBUS_SLAVE_ADDR;                     // at 0
    mctx->request[mctx->req_len++] = MODBUS_FC_READ_HOLDING;                // at 1
    mctx->request[mctx->req_len++] = (uint8_t)(MODBUS_START_ADDR >> 8U);    // at 2 - start addr - high
    mctx->request[mctx->req_len++] = (uint8_t)(MODBUS_START_ADDR & 0xFFU);  // at 3 - start addr - low
    mctx->request[mctx->req_len++] = (uint8_t)(MODBUS_REG_COUNT >> 8U);     // at 4 - quantity - high
    mctx->request[mctx->req_len++] = (uint8_t)(MODBUS_REG_COUNT & 0xFFU);   // at 5 - quantity - low

    // Append CRC
    uint16_t crc   = compute_crc(mctx->request, mctx->req_len);
    mctx->request[mctx->req_len++] = (uint8_t)(crc & 0xFFU);        // low byte
    mctx->request[mctx->req_len++] = (uint8_t)(crc >> 8U);          // high byte

    return MODBUS_MASTER_OK;
}

/**
 * @brief Parse the received frame from slave
 * 
 * Response format:
 * [slave_addr][FC][byte_count][reg0_H][reg0_L]...[regN_H][regN_L][CRC_H][CRC_L]
*/
Modbus_Master_Status_t modbus_parse_received_response(Modbus_Master_Context_t *mctx)
{
    uint8_t len = mctx->resp_len;

    // 1. Minimum reponse is exception: addr(1) + fc(1) + except(1) + crc(2) = 5
    if (len < RESPONSE_MIN_FRAME_LEN) {
        printf("Response too short: %d bytes\n", len);
        return MODBUS_MASTER_ERROR;
    }

    // 2. Validate slave addr
    if (mctx->response[SLAVE_ADDR_POS] != MODBUS_SLAVE_ADDR) {
        printf("Wrong slave address: 0x%02X\n", mctx->response[SLAVE_ADDR_POS]);
        return MODBUS_MASTER_ERROR;
    }

    // 3. Check for exception response - FC has MSB set
    if (mctx->response[FC_ADDR_POS] & 0x80U) {
        printf("Slave exception: 0x%02X\n", mctx->response[EX_ADDR_POS]);
        return MODBUS_MASTER_OK;
    }

    // 4. Validate CRC
    uint16_t crc_received   = (uint16_t)(mctx->response[len-2]) | ((uint16_t)(mctx->response[len-1]) << 8U);
    uint16_t crc_calculated = compute_crc(mctx->response, len - 2U);
    if (crc_received != crc_calculated) {
        printf("CRC failed — rx:0x%04X calc:0x%04X\n", crc_received, crc_calculated);
        return MODBUS_MASTER_ERROR;
    }

    // Print response frame received
    printf("Response frame received: ");
    for (int i = 0; i < len; i++) {
        printf("%02X ", mctx->response[i]);
    }
    printf("\n");

    // 5. Extract register values - big endian, starting at byte 3
    uint8_t *data = &mctx->response[3];

    uint16_t raw_temp  = ((uint16_t)data[0]  << 8U) | data[1];
    uint16_t raw_humi  = ((uint16_t)data[2]  << 8U) | data[3];
    uint16_t raw_pres  = ((uint16_t)data[4]  << 8U) | data[5];
    uint16_t raw_voc   = ((uint16_t)data[6]  << 8U) | data[7];
    uint16_t raw_co2   = ((uint16_t)data[8]  << 8U) | data[9];
    uint16_t raw_pm25  = ((uint16_t)data[10] << 8U) | data[11];
    uint16_t raw_flame = ((uint16_t)data[12] << 8U) | data[13];

    // 6. Scale back to physical values
    float temp  = (float)raw_temp  / SCALE_TEMP;
    float humi  = (float)raw_humi  / SCALE_HUMI;
    float pres  = (float)raw_pres  / SCALE_PRES;
    float voc   = (float)raw_voc   / SCALE_VOC;
    float co2   = (float)raw_co2   / SCALE_CO2;
    float pm25  = (float)raw_pm25  / SCALE_PM25;
    uint8_t flame = (uint8_t)raw_flame;

    /* Log parsed values */
    ESP_LOGI(TAG, "──────────────────────────────────");
    ESP_LOGI(TAG, "Temperature : %.2f °C",   temp);
    ESP_LOGI(TAG, "Humidity    : %.2f %%RH", humi);
    ESP_LOGI(TAG, "Pressure    : %.1f hPa",  pres);
    ESP_LOGI(TAG, "VOC         : %.0f Ω",    voc);
    ESP_LOGI(TAG, "CO2         : %.0f ppm",  co2);
    ESP_LOGI(TAG, "PM2.5       : %.1f µg/m³",pm25);
    ESP_LOGI(TAG, "Flame       : %s",         flame ? "DETECTED" : "None");
    ESP_LOGI(TAG, "──────────────────────────────────");

    return MODBUS_MASTER_OK;
}

void modbus_master_task_init(void) 
{
    xTaskCreate(modbus_master_task, "modbus_master_task", 4096, NULL, 4, NULL);
}

