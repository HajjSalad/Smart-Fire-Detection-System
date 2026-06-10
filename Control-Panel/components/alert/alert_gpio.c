/**
 * @file  alert_gpio.c
 * @brief
 * 
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "string.h"
#include "stdio.h"

#include "alert_gpio.h"


esp_err_t alert_gpio_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << ALERT_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLDOWN_ENABLE,    // idle LOW
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_POSEDGE,       // rising edge - STM32 pulls HIGH on anomaly
    };

    esp_err_t ret;

    ret = gpio_config(&io_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(0);

    // Attach interrupt handler
    gpio_isr_handler_add(ALERT_PIN, alert_isr_handler, NULL);

    return ret;
}

static void IRAM_ATTR alert_isr_handler(void *arg)
{
    printf("Alert received from Sensor Node\n");
}