#include "spi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f446xx.h"

#define GPIOAEN				(1U<<0)
#define GPIOBEN             (1U<<1)
#define SPI1EN				(1U<<12)

#define CPHA                (1U<<0)
#define CPOL                (1U<<1)
#define MSTR                (1U<<3)
#define LSBFIRST            (1U<<7)
#define SSI                 (1U<<8)
#define SSM                 (1U<<9)
#define CR1_SPI1EN          (1U<<6)
#define BIDIMODE            (1U<<15)

#define RXNEIE              (1U<<6)
#define ERRIE               (1U<<5)
#define SR_RXNE             (1U<<0)

volatile uint8_t rx_buffer[BUFFER_SIZE];
volatile uint8_t tx_buffer[BUFFER_SIZE];
volatile uint8_t transfer_complete = 0;
volatile uint16_t rx_index = 0;
volatile uint16_t tx_index = 0;

// SPI1 response
void prepare_spi_response(ResponseType type, const void *data, uint16_t length, bool clean_rx) 
{
    if (clean_rx) {
        memset(rx_buffer, 0, BUFFER_SIZE);      // Clear receive buffer
        transfer_complete = 0;                  // reset 
    }

    memset(tx_buffer, 0, BUFFER_SIZE);          // Clear transmit buffer

    switch(type) {
        case RESPONSE_TEXT: {       // Respond to health check
            const char *text = (const char *)data;
            uint16_t len = strnlen(text, BUFFER_SIZE-1);
            memcpy(tx_buffer, text, len);
            break;
        }
        
        case RESPONSE_BUFFER: {     // Respond to buffer request
            uint16_t bytes = length * sizeof(float);
            if (bytes > BUFFER_SIZE) bytes = BUFFER_SIZE;
            memcpy(tx_buffer, data, bytes);
            break;
        }
    }
    tx_index = 0;       // Reset tx state
    rx_index = 0;       // Reset rx state
}

// Interrupt handler for SPI1
void SPI1_IRQHandler(void)
{
    // Check if RX buffer is not empty
    if (SPI1->SR & SR_RXNE) {
        uint8_t received_data = SPI1->DR;               // Read received data
        
        if (rx_index < BUFFER_SIZE) {                   // Store received data
            rx_buffer[rx_index++] = received_data;
        }
        
        // Check if we have data to send back
        SPI1->DR = (tx_index < BUFFER_SIZE) ? tx_buffer[tx_index++] : 0xFF;

        // Check if NSS went high (transfer complete)
        if (GPIOA->IDR & GPIO_IDR_ID4) {                // Polling mode - Will change to EXTI interrupt for PA4
            transfer_complete = 1;
        }
    }
    
    // Handle overrun error if needed
    if (SPI1->SR & SPI_SR_OVR) {
        // Clear overrun by reading DR then SR
        volatile uint8_t temp = SPI1->DR;
        temp = SPI1->SR;
        (void)temp;
    }
}

// Inittialize Interrupt line for emergency notification - Use PB6
void interrupt_line_init() 
{
    RCC->AHB1ENR |= GPIOBEN;            // GPIOB clock

    GPIOB->MODER &= ~(3U<<12);          // Clear 
    GPIOB->MODER |=  (1U<<12);          // Set PB6 to output mode (01)

    GPIOB->OTYPER &= ~(1 << 6);          // Set output type push-pull

    GPIOB->PUPDR &= ~(0x3 << (6 * 2));   // No pull-up, pull-down
}

// Initialize SPI1 slave - Use PA4(NSS), PA5(SCK), PA6(MISO), PA7(MOSI)
void spi1_slave_init() 
{
    RCC->AHB1ENR |= GPIOAEN;            // Clock for GPIOA
    RCC->APB2ENR |= SPI1EN;             // Clock for SPI

    GPIOA->MODER &=~(1U<<8);			// PA4 mode to alternate function
    GPIOA->MODER |= (1U<<9);
    GPIOA->AFR[0] &= ~(0xF<<16);        // Clear
    GPIOA->AFR[0] |= (5U<<16);			// Set PA4 AF to SPI1_NSS (AF05)

    GPIOA->MODER &=~(1U<<10);			// PA5 mode to alternate function
    GPIOA->MODER |= (1U<<11);
    GPIOA->AFR[0] &= ~(0xF<<20);        // Clear
    GPIOA->AFR[0] |= (5U<<20);			// Set PA5 AF to SP1_SCK (AF05)

    GPIOA->MODER &=~(1U<<12);			// PA6 mode to alternate function
    GPIOA->MODER |= (1U<<13);
    GPIOA->AFR[0] &= ~(0xF<<24);        // Clear
    GPIOA->AFR[0] |= (5U<<24);			// Set PA6 AF to SPI1_MISO (AF05)

    GPIOA->MODER &=~(1U<<14);			// PA7 mode to alternate function
    GPIOA->MODER |= (1U<<15);
    GPIOA->AFR[0] &= ~(0xF<<28);
    GPIOA->AFR[0] |= (5U<<28);			// Set PA7 AF to SPI1_MOSI (AF05)

    SPI1->CR1 = 0;                      // Reset control register
    
    SPI1->CR1 &= ~MSTR;                 // Slave mode
    SPI1->CR1 &= ~CPOL;                 // Clock polarity low
    SPI1->CR1 &= ~CPHA;                 // Clock phase 1st edge
    SPI1->CR1 &= ~LSBFIRST;             // MSB first
    
    SPI1->CR1 &= ~SSM;                  // Hardware slave management (external NSS)
    SPI1->CR1 &= ~SSI;                  // Clear internal slave select
    SPI1->CR1 &= ~BIDIMODE;             // 2-line unidirectional data mode

    SPI1->CR2 |= RXNEIE | ERRIE;        // RX buffer not empty and error interrupts
    
    SPI1->CR1 |= CR1_SPI1EN;            // Enable SPI1

    // Configure NVIC
    NVIC_SetPriority(SPI1_IRQn, 1);     // Medium priority
    NVIC_EnableIRQ(SPI1_IRQn);          // Enable Interrupt
}

