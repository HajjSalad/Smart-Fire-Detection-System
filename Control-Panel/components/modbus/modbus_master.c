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

#define MODBUS_MAX_FRAME_LEN              256U
#define MODBUS_MIN_FRAME_LEN              4U      // addr + FC + DATA + CRC   
#define MODBUS_RESPONSE_TIMEOUT_MS        1000U

static const char *TAG = "MODBUS_MASTER";

// Static Function Prototypes
uint8_t modbus_build_read_request(uint8_t *request);
void modbus_parse_received_response(uint8_t *response, uint8_t len);

static void modbus_master_task(void *pvParameters)
{
    ESP_ERROR_CHECK(modbus_uart2_init());                // Initialize UART2 for modbus comm with STM32
    printf("MODBUS Master started\n");

    uint8_t request[8]          = {0};
    uint8_t req_len             = 0U;

    uint8_t response[MODBUS_MAX_FRAME_LEN] = {0};
    int byte_received = 0;

    while(1) 
    {
        //printf("Modbus Task running\n");

        // 1. Build request frame
        req_len = modbus_build_read_request(request);

        // 2. Flush any stale data in RX buffer
        //uart_flush(UART_NUM2);

        // 3. Send request to STM32
        uart_write_bytes(UART_NUM2, request, req_len);
        printf("Request sent - waiting for response...\n");

        // 4. Wait for response
        memset(response, 0, sizeof(response));
        byte_received = uart_read_bytes(UART_NUM2, response, sizeof(response),
                                        pdMS_TO_TICKS(MODBUS_RESPONSE_TIMEOUT_MS));

        // 5. Parse response
        if (byte_received > 0) {
            modbus_parse_received_response(response, (uint8_t)byte_received);
        } else {
            printf("Timeout - no response from slave\n\n");
        }

        // 6. Sleep until next read cycle
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
/**
 * @brief Build FC 0x03 read holding registers request
 * 
 * Read request format:
 * [slave_addr][FC][start_addr_H][start_addr_L][quantity_H][quantity_L][CRC_L][CRC_H]
*/
uint8_t modbus_build_read_request(uint8_t *request)
{
    uint8_t idx = 0U;

    request[idx++] = MODBUS_SLAVE_ADDR;                     // at 0
    request[idx++] = MODBUS_FC_READ_HOLDING;                // at 1
    request[idx++] = (uint8_t)(MODBUS_START_ADDR >> 8U);    // at 2 - start addr - high
    request[idx++] = (uint8_t)(MODBUS_START_ADDR & 0xFFU);  // at 3 - start addr - low
    request[idx++] = (uint8_t)(MODBUS_REG_COUNT >> 8U);     // at 4 - quantity - high
    request[idx++] = (uint8_t)(MODBUS_REG_COUNT & 0xFFU);   // at 5 - quantity - low

    // Append CRC
    uint16_t crc   = compute_crc(request, idx);
    request[idx++] = (uint8_t)(crc & 0xFFU);        // low byte
    request[idx++] = (uint8_t)(crc >> 8U);          // high byte

    return idx;
}

/**
 * @brief
 * 
 * Response format:
 * [slave_addr][FC][byte_count][reg0_H][reg0_L]...[regN_H][regN_L][CRC_H][CRC_L]
*/
void modbus_parse_received_response(uint8_t *response, uint8_t len)
{
    // 1. Minimum reponse: addr(1)+FC(1)+byte_count(1)+data(2)+CRC(2) = 7
    if (len < 7U) {
        printf("Response too short: %d bytes\n", len);
        return;
    }

    // 2. Validate slave addr
    if (response[0] != MODBUS_SLAVE_ADDR) {
        printf("Wrong slave address: 0x%02X\n", response[0]);
        return;
    }

    // 3. Check for exception response - FC has MSB set
    if (response[1] & 0x80U) {
        printf("Slave exception: 0x%02X\n", response[2]);
        return;
    }

    // 4. Validate CRC
    uint16_t crc_received   = (uint16_t)(response[len-2]) | ((uint16_t)(response[len-1]) << 8U);
    uint16_t crc_calculated = compute_crc(response, len - 2U);
    if (crc_received != crc_calculated) {
        printf("CRC failed — rx:0x%04X calc:0x%04X\n", crc_received, crc_calculated);
        return;
    }

    // 5. Extract register values - big endian, starting at byte 3
    uint8_t *data = &response[3];

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
}

void modbus_master_task_init(void) 
{
    xTaskCreate(modbus_master_task, "modbus_master_task", 4096, NULL, 4, NULL);
}

