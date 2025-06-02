/**
 * @file uart.h
 * @brief Header file for UART initialization on ESP32.
 *
 * This header declares the uart_init function used to configure
 * and initialize UART communication.
 */

#ifndef UART_H
#define UART_H

/**
 * @brief Initialize UART0 with default settings.
 *
 * Must be called before performing any UART communication.
 */
void uart_init();

#endif // UART_H