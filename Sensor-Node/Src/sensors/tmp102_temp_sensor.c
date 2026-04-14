/**
 * @file  tmp102_temp_sensor.c
 * @brief TMP102 temperature sensor driver over I2C1
 * 
 * I2C address:   0x48 (ADD0 -> GND)
 * Temp register: 0x00
 * Resolution:    12-bit, 0.0625°C per LSB
*/

#include "sensor_drivers.h"
#include "i2c_driver.h"

#define TMP102_ADDR         0x48U       // I2C address
#define TMP102_TEMP_REG     0x00U       // Temperature register
#define TMP102_RESOLUTION   0.0625f     // °C per LSB

/**
 * @brief Initialize TMP102
 * 
 * Starts measuring on power-up, no config needed
*/
TMP102_Status_t tmp102_init(void)
{
    return TMP102_OK;
}

/**
 * @brief Read temperature from TMP102
*/
TMP102_Status_t tmp102_read(float *temp)
{
    printf("In TMP102 read\n");
    if (temp == NULL) return TMP102_ERROR;

    uint8_t msb, lsb;
    int16_t raw;

    // 1. Point to temperature register
    i2c1_start(TMP102_ADDR, I2C_WRITE);
    i2c1_write(TMP102_TEMP_REG);

    printf("Step 1 okay\n");

    // 2. Read 2 bytes
    i2c1_start(TMP102_ADDR, I2C_READ);
    msb = i2c1_read(1);         // ACK  - more bytes coming
    lsb = i2c1_read(0);         // NACK - last byte
    i2c1_stop();

    // 3. Reconstruct 12-bit raw value
    raw = (int16_t)((msb << 8) | lsb) >> 4;

    // Convert to °C
    *temp = (float)raw * TMP102_RESOLUTION;

    return TMP102_OK;
}
