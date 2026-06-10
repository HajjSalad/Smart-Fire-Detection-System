/**
 * @file  alert_gpio.h
 * @brief 
*/

#ifndef ALERT_GPIO_H
#define ALERT_GPIO_H

#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "esp_err.h"

#define ALERT_PIN         GPIO_NUM_4

// Function prototype
esp_err_t alert_gpio_init(void);
static void IRAM_ATTR alert_isr_handler(void *arg);

#endif      // ALERT_GPIO_H