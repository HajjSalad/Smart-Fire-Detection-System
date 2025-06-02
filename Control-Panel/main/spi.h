/**
 * @file spi.h
 * @brief Interface for SPI Master initialization on ESP32 using ESP-IDF.
 * 
 * This header file declares the SPI initialization function and external 
 * handles required to manage SPI communication.
 */

#ifndef SPI_H
#define SPI_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

/// Handle to the SPI device, initialized by spi_init()
extern spi_device_handle_t handle;

/// Handle to the SPI task, initialized in xTaskCreate()
extern TaskHandle_t spi_task_handle;

/**
 * @brief Initialize the SPI bus and attach the SPI device
 * 
 * This function must be called before any SPI transactions are performed.
 * It sets up the SPI bus and adds the SPI device to the bus with the
 * specified configuration.
 */
void spi_init();

#endif // SPI_H