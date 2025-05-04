
//   SPI Master

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

#define MOSI             23
#define MISO             19
#define SCK              18
#define SS                5

spi_device_handle_t handle;
TaskHandle_t spi_task_handle = NULL;

void spi_init()
{
    // Set the communication parameters
    const spi_bus_config_t spi_config = {
        .miso_io_num = MISO,
        .mosi_io_num = MOSI,
        .sclk_io_num = SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64
    };

    // Set SPI device interface
    const spi_device_interface_config_t spi_device_config = {
        .clock_speed_hz = 1*100*1000,   // Set the clock speed (100 kHz)
        .duty_cycle_pos = 128,          // Set the duty cycle, 128 = 50%
        .mode           = 0,            // SPI mode (0: CPOL = 0, CPHA = 0)
        .spics_io_num   = SS,           // Slave Select
        .queue_size     = 1,            // Number of transactions to queue
    };

    spi_bus_initialize(SPI2_HOST, &spi_config, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &spi_device_config, &handle);
}

