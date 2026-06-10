/**
 * @file  bme680_enviro_sensor.c
 * @brief BME680 environmental sensor driver
 * 
 * Thin HAL wrapper around the Bosch BME68x SensorAPI.
 * Bridges the Bosch driver interface to the bare-metal SPI1 
 * driver and FreeRTOS delay.
 * 
 * SPI1 SCK  — PB3 (AF5)
 * SPI1 MOSI — PA7 (AF5)
 * SPI1 MISO — PA6 (AF5)
 * SPI1 CS   — PC7 (GPIO, active low)
*/
#include "FreeRTOS.h"
#include "task.h"

#include "bme68x.h"
#include "bme68x_defs.h"
#include "sensor_drivers.h"

#include "spi_driver.h"

// Bosch driver instance
static struct bme68x_dev bme = {0};

/*  ---  HAL callbacks required by Bosch driver  ---  */

/**
 * @brief SPI read callback for Bosch driver
*/
static BME68X_INTF_RET_TYPE bme680_spi_read(uint8_t reg, uint8_t *data,
                                            uint32_t len, void *intf_ptr) {
    (void)intf_ptr;
    spi1_read_regs(reg, data, (uint8_t)len);
    //spi1_read_regs_dma(reg, data, (uint8_t)len);
    return BME68X_OK;
}

/**
 * @brief SPI write callback for Bosch driver
*/
static BME68X_INTF_RET_TYPE bme680_spi_write(uint8_t reg, const uint8_t *data,
                                            uint32_t len, void *intf_ptr) {
    (void)intf_ptr;
    spi1_write_regs(reg, data, (uint8_t)len);
    return BME68X_OK;
}

/**
 * @brief Microsecond delay callback for Bosch driver
*/
static void bme680_delay_us(uint32_t period_us, void *intf_ptr) {
    (void)intf_ptr;
    vTaskDelay(pdMS_TO_TICKS(period_us / 1000U));
}

/**
 * @brief Initialize BME680 via Bosch SensorAPI
 * 
 * Configures SPI interface, loads factory calibration, sets
 * oversampling and IIR filter coefficients.
*/
BME680_Status_t bme680_init(void)
{
    // Wire up HAL callbacks
    bme.read        = bme680_spi_read;
    bme.write       = bme680_spi_write;
    bme.delay_us    = bme680_delay_us;
    bme.intf        = BME68X_SPI_INTF;
    bme.amb_temp    = 25;                   // ambient temp estimate for heater calc

    int8_t ret = bme68x_init(&bme);
    if (ret != BME68X_OK) {
        printf("BME68x_init unsuccessful\n\r");
        return BME680_ERROR;
    }
    printf("BME68x_init successful.\n\r");

    return BME680_OK;
}

/**
 * @brief Read all sensor values from BME680
 * 
 * Configures oversampling, triggers forced mode measurement, waits for
 * completion, reads and returns compensated values.
*/
BME680_Status_t bme680_read(float *temp, float *humi, float *pres, float *voc)
{
    if (temp == NULL || humi == NULL || pres == NULL || voc == NULL) {
        return BME680_ERROR;
    }

    struct bme68x_conf       conf      = {0};
    struct bme68x_heatr_conf heatr     = {0};
    struct bme68x_data       data      = {0};

    int8_t   ret;
    uint8_t  n_fields  = 0U;
    uint32_t delay_us  = 0U;

    // 1. Configure oversampling and IIR filter
    conf.os_temp = BME68X_OS_2X;
    conf.os_pres = BME68X_OS_4X;
    conf.os_hum  = BME68X_OS_1X;
    conf.filter  = BME68X_FILTER_SIZE_3;
    conf.odr     = BME68X_ODR_NONE;

    ret = bme68x_set_conf(&conf, &bme);
    if (ret != BME68X_OK) return BME680_ERROR;

    // 2. Configure gas heater - 300°C for 100ms
    heatr.enable     = BME68X_ENABLE;
    heatr.heatr_temp = 300U;
    heatr.heatr_dur  = 100U;

    ret = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr, &bme);
    if (ret != BME68X_OK) return BME680_ERROR;

    // 3. Trigger forced mode — single measurement then sleep
    ret = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);
    if (ret != BME68X_OK) return BME680_ERROR;

    // Wait for measurement to complete 
    delay_us = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme)
               + ((uint32_t)heatr.heatr_dur * 1000U);
    bme.delay_us(delay_us, bme.intf_ptr);

    // 4. Read compensated data
    ret = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme);
    if (ret != BME68X_OK || n_fields == 0U) return BME680_ERROR;

    // Extract physical values
    *temp = data.temperature;                   // °C
    *humi = data.humidity;                      // %RH
    *pres = data.pressure / 100.0f;             // Pa → hPa
    *voc  = data.gas_resistance / 1000.0f;      // Ω  → kΩ

    return BME680_OK;
}
