#ifndef I2C_DRIVER_H_
#define I2C_DRIVER_H_

/**
 * @file  i2c_driver.h
 * @brief Public API for I2C1 peripheral.
*/

#include <stdio.h>
#include <stdint.h>

#define I2C_WRITE       0U
#define I2C_READ        1U

// Function Prototyoes
void    i2c1_init(void);
void    i2c1_start(uint8_t address, uint8_t direction);
void    i2c1_stop(void);
void    i2c1_write(uint8_t data);
void    i2c1_write_multiple(uint8_t *data, uint16_t size);
uint8_t i2c1_read(uint8_t ack);

#endif  // I2C_DRIVER_H_