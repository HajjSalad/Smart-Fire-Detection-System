/**
 * @file gpio_interrupt.c
 * @brief GPIO interrupt handling for sensor input signal detection.
 *
 * This module configures a GPIO pin to trigger an interrupt on a positive edge,
 * notifying a FreeRTOS task to handle the event.
 */

#include "spi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

extern TaskHandle_t spi_task_handle;     /**< Handle of the task to notify on interrupt */

#define INTERRUPT_LINE   22              /**< GPIO pin number used for the interrupt line */

volatile int flagRequest = 0;            /**< Flag indicating an interrupt request */

/**
 * @brief ISR handler for the GPIO interrupt triggered by the sensor.
 *
 * This handler notifies the spi_task_handle task from the ISR and yields if
 * a higher priority task was woken.
 *
 * @param arg Pointer to user data.
 */
void IRAM_ATTR sensor_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(spi_task_handle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

/**
 * @brief Initialize the interrupt line GPIO pin and configure ISR.
 *
 * Sets up the GPIO pin as input with pull-up enabled and configures it to trigger
 * interrupts on the positive edge. Registers the ISR handler to the interrupt line.
 */
void interrupt_line_init() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,             // Interrupt on the positive edge of the clock      
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << INTERRUPT_LINE),
        .pull_down_en = 0,
        .pull_up_en = 1
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);  // 0 = default ISR service
    gpio_isr_handler_add(INTERRUPT_LINE, sensor_isr_handler, NULL);
}