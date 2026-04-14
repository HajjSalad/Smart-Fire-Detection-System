/**
 * @file uart2.c
 * @brief UART drive implementation
 * 
 * Provides low-level UART2 initialization and transmit/receive functionality.
*/

#include "stm32f446xx.h"

#include "uart_driver.h"

#define GPIOAEN				(1U<<0)
#define UART2EN				(1U<<17)

#define CR1_TE				(1U<<3)
#define CR1_RE				(1U<<2)
#define CR1_UE				(1U<<13)
#define SR_TXE				(1U<<7)

#define SYS_FREQ        	((uint32_t) 16000000)
#define APB1_CLK        	SYS_FREQ
#define UART_BAUDRATE   	((uint32_t) 115200)

// Function Prototypes
static void 	uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

/**
 * @brief Low-level character output function for printf redirection.
 * 
 * This function is called by the C standard library to output characters
 * when using fucntions such as `printf()`.
 * 
 * @param ch 	Character to transmit
 * @return 		The transmitted character
*/
int __io_putchar(int ch) {
	uart2_write(ch);
	return ch;
}

/**
 * @brief Initialize UART2 peripheral.
 * 
 * Used for debug logging via printf().
*/
void uart2_init(void) 
{
	RCC->AHB1ENR |= GPIOAEN;			// Enable clock GPIOA

	GPIOA->MODER &=~(1U<<4);			// PA2 mode to alternate function
	GPIOA->MODER |= (1U<<5);
	GPIOA->AFR[0] |= (7U<<8);			// Set PA2 AF to UART2_TX (AF07)

	GPIOA->MODER &=~(1U<<6);			// PA3 mode to alternate function
	GPIOA->MODER |= (1U<<7);
	GPIOA->AFR[0] |= (7U<<12);			// Set PA3 AF to UART2_RX (AF07)

	RCC->APB1ENR |= UART2EN;			// Enable clock to UART2

	// Configure baudrate USART2 and USART4
	uart_set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	USART2->CR1 = (CR1_TE | CR1_RE);	// Configure the transfer direction

	USART2->CR1 |= CR1_UE;				// Enable USART Module
}

/**
 * @brief Transmit a single character over UART2.
 * 
 * This function blocks until the transmit data register is empty,
 * then writes the provided character to the UART data register.
 * 
 * @param ch  Character to transmit.
*/
void uart2_write(int ch) 
{
	while(!(USART2->SR & SR_TXE)){};			// Make sure the transmit data register is empty.
	USART2->DR = ((uint32_t)ch & 0xFF);			// Write to transmit data register
}

/** @brief Configure the baud rate for the USART peripheral */
static void uart_set_baudrate(USART_TypeDef *USARTx, 
							  uint32_t PeriphClk, 
							  uint32_t BaudRate) 
{
	USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

/** @brief Compute USART baud rate register (BRR) value */
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate) {

	return ((PeriphClk + (BaudRate / 2U)) / BaudRate);
}