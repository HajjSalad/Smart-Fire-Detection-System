#ifndef SPI_DRIVER_H_
#define SPI_DRIVER_H_

/**
 * @file  spi_driver.h
 * @brief Public API for SPI1 peripheral.
*/

#include <stdio.h>
#include <stdint.h>

// Function Prototyoes
void spi1_init(void);
void spi1_cs_low(void);
void spi1_cs_high(void);
uint8_t spi1_transmit_receive(uint8_t data);
void spi1_write_regs(uint8_t reg, const uint8_t *data, uint8_t len);
void spi1_read_regs(uint8_t reg, uint8_t *buf, uint8_t len);
void spi1_read_regs_dma(uint8_t reg, uint8_t *buf, uint8_t len);

#endif  // SPI_DRIVER_H_