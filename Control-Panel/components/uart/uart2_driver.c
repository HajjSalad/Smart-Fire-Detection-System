/**
 * @file uart2_driver.c
 * @brief
 * 
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "string.h"
#include "stdio.h"

#include "uart_driver.h"

QueueHandle_t uart_2_queue;

esp_err_t modbus_uart2_init(void)
{
    const uart_config_t uart_config = {             
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    esp_err_t ret;
    
    ret = uart_param_config(UART_NUM2, &uart_config);
    if (ret != ESP_OK) return ret;

    ret = uart_set_pin(UART_NUM2, UART_2_TX, UART_2_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) return ret;

    ret = uart_driver_install(UART_NUM2, BUF_SIZE, BUF_SIZE, 10, &uart_2_queue, 0);
    return ret;
}

