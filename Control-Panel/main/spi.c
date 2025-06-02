/**
 * @file spi.c
 * @brief SPI Master initialization for ESP32 using ESP-IDF.
 * 
 * This file contains the implementation of initializing the SPI bus
 * and setting up the SPI device configuration for communication.
 */

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

/// GPIO pin used for Master Out Slave In
#define MOSI             23
/// GPIO pin used for Master In Slave Out
#define MISO             19
/// GPIO pin used for SPI Clock
#define SCK              18
/// GPIO pin used for Slave Select
#define SS                5

/// Handle to the SPI device
spi_device_handle_t handle;

/// Handle to the SPI task
TaskHandle_t spi_task_handle = NULL;

/**
 * @brief Initialize the SPI bus and configure the SPI device.
 * 
 * This function sets up thew SPI bus with specified parameters and
 * add the SPI device to the bus using the ESP-IDF SPI Master driver.
 * 
 * - Clock speed: 100 kHz
 * - Mode: SPI Mode 0 (CPOL = 0, CPHA = 0)
 * - SS pin: GPIO 5
 * - Max transfer size: 64 bytes
 */
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

