/**
 * @file  uart1.c
 * @brief UART1 driver implementation
 * 
 * Provides low-level UART1 initialization and transmit/receive functionality.
*/

#include "stm32f446xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "shared_resources.h"
#include "uart_driver.h"
#include "tasks.h"

#include <stdio.h>
#include <string.h>

#define GPIOAEN				(1U<<0)
#define UART1EN				(1U<<4)

#define CR1_TE				(1U<<3)
#define CR1_RE				(1U<<2)
#define CR1_UE				(1U<<13)
#define CR1_RXNE			(1U<<5)
#define SR_TXE				(1U<<7)
#define SR_RXNE				(1U<<5)
#define SR_ORE              (1U<<3)

#define SYS_FREQ        	((uint32_t) 16000000)
#define APB2_CLK        	SYS_FREQ
#define UART_BAUDRATE   	((uint32_t) 115200)

// Ring buffer
#define RING_BUFFER_SIZE                256
volatile uint8_t  rBuffer[RING_BUFFER_SIZE];
volatile uint16_t rHead = 0;
volatile uint16_t rTail = 0;

// Function Prototypes
static void 	uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

/**
 * @brief Initialize UART1 peripheral.
 * 
 * UART1 is configured for communication with ESP32
*/
void uart1_init(void) 
{
	RCC->AHB1ENR |= GPIOAEN;			// Enable clock GPIOA
    RCC->APB2ENR |= UART1EN;			// Enable clock to UART1

	GPIOA->MODER &=~(1U<<18);			// PA9 to alternate function mode
	GPIOA->MODER |= (1U<<19);
	GPIOA->AFR[1] |= (7U<<4);			// Set PA9 AF to UART1_TX (AF07)
    GPIOA->OSPEEDR |= (3<<18);			// High Speed for PA9

	GPIOA->MODER &=~(1U<<20);			// PA10 to alternate function mode
	GPIOA->MODER |= (1U<<21);
	GPIOA->AFR[1] |= (7U<<8);			// Set PA10 AF to UART1_RX (AF07)
    GPIOA->OSPEEDR |= (3<<20);			// High Speed for PA10
	
    USART1->CR1 = 0x00;   				// Clear ALL

	// Configure baudrate USART1
	uart_set_baudrate(USART1, APB2_CLK, UART_BAUDRATE);

	USART1->CR1 = (CR1_TE | CR1_RE);	// Configure the transfer direction
	USART1->CR1 |= CR1_RXNE;       		// Enable RXNE interrupt
	NVIC_SetPriority(USART1_IRQn, 6);
	NVIC_EnableIRQ(USART1_IRQn);     	// Enable USART1 in NVIC
	USART1->CR1 |= CR1_UE;				// Enable USART Module
}

/**
 * @brief Transmit a single byte over UART1.
 * 
 * Blocks until the transmit register is empty.
 * 
 * DR is only 8bits wide for transmission and transmits 1 byte. Using uint16_t or 
 * uint32_t, the upper limits will be ignored. So need to make sure uint8_t is transmitted.
 * 
 * Send will be responsible for breaking down multi-byte data into byte for uart1_write to
 * transmit byte by byte.
 * 
 * @ref RM0390 USART_DR
 * 
 * @param byte  Byte to transmit.
*/
void uart1_write(uint8_t byte) 
{
	while(!(USART1->SR & SR_TXE)){};	// Make sure the transmit data register is empty.
	USART1->DR = (byte);				// Write to transmit data register
}	

/**
 * @brief Send full frame via the uart1_write
*/
void uart1_send(uint8_t *buf, uint16_t len) 
{
	for (uint16_t i=0; i<len; i++) {
		uart1_write(buf[i]);
	}
}

/**
 * @brief USART1 Interrupt Service Routine
 * 
 * Fires on every received byte.
 * Reads the byte from the data register and adds it to the ring buffer.
 * 
 * The ISR needs to be very short -> immediately read DR and get ready for the next byte
*/
void USART1_IRQHandler(void)
{
	if (USART1->SR & SR_RXNE)
	{
		uint8_t byte = USART1->DR;

		uint16_t next = (rHead + 1) % RING_BUFFER_SIZE;

		if (next != rTail) 		// Buffer not full
		{
			rBuffer[rHead] = byte;
			rHead = next;
		}
	}
}

uint8_t uart1_ring_buffer_read(uint8_t *byte)
{
	if (rHead == rTail) return 0;		// Buffer empty

	*byte  = rBuffer[rTail];						// Get byte from the tail
	rTail = (rTail + 1) % RING_BUFFER_SIZE;		// Advance the tail

	return 1;
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
