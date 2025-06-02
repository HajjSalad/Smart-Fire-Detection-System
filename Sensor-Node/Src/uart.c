/**
 * @file uart.c
 * @brief UART communication setup and I/O functions for STM32F446xx.
 *
 * This file contains functions to initialize UART2 for full-duplex communication
 * and perform basic transmit/receive operations. It includes GPIO pin
 * configuration and baud rate setting.
 */

#include "stm32f446xx.h"
#include "uart.h"
#include <stdio.h>

#define GPIOAEN				(1U<<0)
#define UART2EN				(1U<<17)

#define CR1_TE				(1U<<3)
#define CR1_RE				(1U<<2)
#define CR1_UE				(1U<<13)

#define SR_TXE				(1U<<7)
#define SR_RXNE				(1U<<5)

#define SYS_FREQ			16000000
#define APB1_CLK			SYS_FREQ

#define UART_BAUDRATE		115200

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

/**
 * @brief System write override for low-level output via UART2.
 *
 * Used by `printf` internally to send output through UART2.
 *
 * @param file File descriptor (unused)
 * @param ptr Pointer to data buffer
 * @param len Number of characters to write
 * @return int Number of characters written
 */
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        uart2_write(ptr[i]);
    }
    return len;
}

/**
 * @brief Initialize UART2 with both transmitter and receiver enabled.
 *
 * Configures GPIOA pins PA2 and PA3 for UART alternate function,
 * sets the baud rate, enables UART2 peripheral clock, and enables
 * the UART module for transmission and reception.
 */
void uart2_rxtx_init(void) {

    /******   Configure UART GPIO Pin    ******/
    // 1.Enable clock access to GPIOA
    RCC->AHB1ENR |= GPIOAEN;

    // 2.Set PA2 mode to alternate function mode. PA2 = tx line of UART2. For Alternate function mode, need 10 (RM pg157)
    GPIOA->MODER &=~(1U<<4);
    GPIOA->MODER |= (1U<<5);

    // 3.Set PA2 alternate function type to UART_TX (AF07). (Alternate Function Mapping, DT pg47).
    // AFRL for Pin2 (AFRL2) -> 11,10,9,8 (RM pg161).
    GPIOA->AFR[0] |= (1U<<8);								// AFR[0] - AFRL	AFR[1] - AFRLH
    GPIOA->AFR[0] |= (1U<<9);
    GPIOA->AFR[0] |= (1U<<10);
    GPIOA->AFR[0] &=~(1U<<11);

    // 2.Set PA3 mode to alternate function mode. PA3 = rx line of UART2
    GPIOA->MODER &=~(1U<<6);
    GPIOA->MODER |= (1U<<7);

    // 3.Set PA2 alternate function type to UART_RX (AF07). (Alternate Function Mapping, DT pg47).
    // AFRL for Pin3 (AFRL3) -> 15,14,13,12. AFRLy selection -> 0111: AF7 (RM pg161).
    GPIOA->AFR[0] |= (1U<<12);								// AFR[0] - AFRL	AFR[1] - AFRLH
    GPIOA->AFR[0] |= (1U<<13);
    GPIOA->AFR[0] |= (1U<<14);
    GPIOA->AFR[0] &=~(1U<<15);

    /******   Configure UART Module     *******/
    // 1.Enable clock access to UART2
    RCC->APB1ENR |= UART2EN;

    // 2.Configure baudrate
    uart_set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

    // 3.Configure the transfer direction
    // USART2->CR1 = CR1_TE;		// Clears everything in CR1 and set the new value - only when config tx
    USART2->CR1 = (CR1_TE | CR1_RE);		// Enable both - Duplex transmission mode

    // 4.Enable the UART Module
    USART2->CR1 |= CR1_UE;		// Doesn't clear CR1, but adds new value to existing values
}

/**
 * @brief Read a character from UART2.
 *
 * Waits until the receive data register is not empty,
 * then returns the received character.
 *
 * @return char Received character
 */
char uart2_read(void) {

    // Make sure the receive data register is not empty. Status register (USART_SR) for RXNE=5 (RM pg547)
    while(!(USART2->SR & SR_RXNE)){};				// Stuck in the while loop till its true

    // Return the read data
    return USART2->DR;
}

/**
 * @brief Write a character to UART2.
 *
 * Waits until the transmit data register is empty,
 * then writes the character to the UART data register.
 *
 * @param ch Character to be transmitted
 */
void uart2_write(int ch) {

    // Make sure the transmit data register is empty. Status register (USART_SR) for TXE=7 (RM pg547)
    while(!(USART2->SR & SR_TXE)){};				// Stuck in the while loop till its true

    // Write to transmit data register
    USART2->DR = (ch & 0xFF);
}

/**
 * @brief Set the baud rate for a USART peripheral.
 *
 * Computes and writes the baud rate register (BRR) value.
 *
 * @param USARTx Pointer to USART peripheral
 * @param PeriphClk Peripheral clock frequency
 * @param BaudRate Desired baud rate
 */
static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate) {

    USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

/**
 * @brief Compute the baud rate divisor.
 *
 * Used internally to compute the BRR value from the clock frequency and desired baud rate.
 *
 * @param PeriphClk Peripheral clock frequency
 * @param BaudRate Desired baud rate
 * @return uint16_t Computed BRR value
 */
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate) {

    return ((PeriphClk + (BaudRate/2U))/BaudRate);
}
