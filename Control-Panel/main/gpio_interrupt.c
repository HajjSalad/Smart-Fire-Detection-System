
// GPIO Interrupt

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

extern TaskHandle_t spi_task_handle;

#define INTERRUPT_LINE   22
volatile int flagRequest = 0;

void IRAM_ATTR sensor_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(spi_task_handle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

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