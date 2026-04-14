#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_

/**
 * @file  uart_driver.h
 * @brief Public API for UART1 and UART2 peripheral.
*/

#include <stdio.h>
#include <stdint.h>

// UART1 Function Prototypes
void uart1_init(void);
void uart1_write(uint8_t byte);
void uart1_send(uint8_t *buf, uint16_t len);
uint8_t uart1_ring_buffer_read(uint8_t *byte);

// UART2 Function Prototypes
void uart2_init(void);
void uart2_write(int ch);

#endif /* UART_DRIVER_H_ */
