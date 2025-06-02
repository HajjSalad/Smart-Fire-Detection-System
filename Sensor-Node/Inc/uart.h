/**
 * @file uart.h
 * @brief UART2 initialization and I/O function declarations for STM32F446xx.
 *
 * This header provides function prototypes for initializing UART2
 * and performing basic character-based I/O operations.
 */

#ifndef UART_H_
#define UART_H_

#include <stdio.h>
#include "stm32f446xx.h"

/**
 * @brief Initialize UART2 with both transmitter and receiver enabled.
 *
 * Configures the appropriate GPIO pins (PA2 and PA3) and sets the baud rate.
 */
void uart2_rxtx_init(void);

char uart2_read(void);
void uart2_write(int ch);

#endif /* UART_H_ */