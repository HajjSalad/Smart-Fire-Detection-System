/**
 * @file  uart_driver.h
 * @brief Header file for UART initialization on ESP32.
*/

#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "esp_err.h"

#define UART_2_TX           17
#define UART_2_RX           16
#define UART_NUM2           UART_NUM_2
#define BUF_SIZE            1024

extern QueueHandle_t uart_2_queue;

// Function Prototype
esp_err_t modbus_uart2_init(void);

#endif // UART_DRIVER_H