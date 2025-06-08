#include "spi.h"
#include "systick.h"
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
#define CR1_CRCEN           (1U<<13)

#define PR_PR4              (1U<<4)
#define EXTI4_PA4           (0U<<0)
#define IMR_IM4             (1U<<4)
#define FTSR_TR4            (1U<<4)

volatile uint8_t rx_buffer[BUFFER_SIZE];
volatile uint8_t tx_buffer[BUFFER_SIZE];
volatile uint8_t start_transfer = 0;
volatile uint8_t rx_complete = 0;
volatile uint8_t command_complete = 0;
volatile uint16_t rx_index = 0;
volatile uint16_t tx_index = 0;

volatile SPI_Response response_queue[RESPONSE_QUEUE_SIZE];

void queue_response(ResponseType type, const void *data, uint16_t length) {
    response_queue[0].type = type;
    response_queue[0].length = (length > BUFFER_SIZE) ? BUFFER_SIZE : length;

    memcpy((void *)response_queue[0].data, data, response_queue[0].length);
    response_queue[0].ready = true;
    //printf("Response Length: %d\r\n", response_queue[0].length);
}

void prepare_queued_response(bool clean_rx) {
    if (!response_queue[0].ready) {
        return;       // Exit if response is not ready
        printf("Error: Response not ready\r\n");
    }
    if (clean_rx) {
        memset((void *)rx_buffer, 0, response_queue[0].length);
        rx_index = 0;                       // Reset index
        start_transfer = 0;
    }
    memcpy((void *)tx_buffer, (void *)response_queue[0].data, response_queue[0].length);
    
    tx_index = 0;
    rx_index = 0;
    //printf("Response is ready 1.\r\n");
}

// Handle the Master data received
void process_spi_command(void) {
    //printf("Received from Master: %s\r", rx_buffer);
           
    if (strncmp((char*)rx_buffer, "Are you Alive?", strlen("Are you Alive?")) == 0) {
        queue_response(RESPONSE_TEXT, "I'm Alive", strlen("I'm Alive"));
    }
    else if (strncmp((char*)rx_buffer, "Data Request", strlen("Data Request")) == 0) {
        __attribute__((aligned(4))) float sensor_data[10] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.10f};
        queue_response(RESPONSE_BUFFER, sensor_data, 10 * sizeof(float));
        
    }
    else {
        queue_response(RESPONSE_TEXT, "Unknown command", strlen("Unknown command"));
    }
    prepare_queued_response(true);
}

// SPI1 Global Interrupt Handler
void SPI1_IRQHandler(void) {
    if (SPI1->SR & SR_RXNE) {
        uint8_t received = SPI1->DR;

        if (command_complete == 0) {
            // Phase 1: Slave receives command, responds with dummy
            // - This allows the slave time to prepare the correct reponse
            rx_buffer[rx_index++] = received;
            //printf("Received: 0x%02X\r\n", received);
            if (received == '\n') {
                rx_buffer[rx_index] = '\0';
                rx_index = 0;

                //printf("\nMaster command received.\r\n");
                process_spi_command();     // Only queue response, Sent in next SPI transaction
            }
            SPI1->DR = DUMMY_BYTE;         // Respond with dummy
        }
        else if (command_complete == 1) {
            // Phase 2: Master sends dummy. Slave responds with the correct response
            // - The slave now has the correct response prepared
            rx_buffer[rx_index++] = received;

            if (response_queue[0].ready && tx_index < response_queue[0].length) {
                SPI1->DR = tx_buffer[tx_index++];
            } else {
                SPI1->DR = DUMMY_BYTE;
            }

            if (tx_index >= response_queue[0].length) {
                //printf("Response sent to Master: ");
                // print the data sent to master here
                for (uint16_t i = 0; i < response_queue[0].length; i++) {
                    //printf("%02X ", tx_buffer[i]);
                }
                printf("\r\n");
                response_queue[0].ready = false;
                command_complete = 0;
                tx_index = 0;
                rx_index = 0;
            }
        }
    }
}

// NSS (Slave Select) Interrupt handler
void EXTI4_IRQHandler(void) {
    if (EXTI->PR & PR_PR4) {            // NSS falling edge
        EXTI->PR |= PR_PR4;             // Clear interrupt flag

        if (response_queue[0].ready) {
            // Enter Phase 2 - Prepare to start response phase
            // prepare_queued_response(false);   
            command_complete = 1;
            tx_index = 0;
        } else {
            // Enter Phase 1 - Prepare to receive command 
            command_complete = 0;
            rx_index = 0;
        }        
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

//  NSS (Slave Select) – Tells slave when to listen or stop. Low (0) to start communication and high (1) to end it.
//  MOSI (Master Out Slave In) – Carries data from master to slave.
//  MISO (Master In Slave Out) – Carries data from slave to master.
//  SCK (Serial Clock) – Clock signal generated by the master to synchronize data transfer.

// Instead of polling NSS, use EXTI4_IRQHandle. Use EXTI4 to detect the NSS falling edge

// Initialize SPI1 slave - Use PA4(NSS), PA5(SCK), PA6(MISO), PA7(MOSI)
void spi1_slave_init() 
{
    RCC->AHB1ENR |= GPIOAEN;            // Clock for GPIOA
    RCC->APB2ENR |= SPI1EN;             // Clock for SPI

    GPIOA->MODER &=~(1U<<8);			// PA4 mode to alternate function
    GPIOA->MODER |= (1U<<9);
    GPIOA->AFR[0] &= ~(0xF<<16);        // Clear
    GPIOA->AFR[0] |= (5U<<16);			// Set PA4 AF to SPI1_NSS (AF05)

    SYSCFG->EXTICR[1] |= EXTI4_PA4;     // Configure External Interrupt for PA4
    EXTI->IMR |= IMR_IM4;               // Enable the Interrupt Mask for PA4
    EXTI->FTSR |= FTSR_TR4;             // Falling edge trigger for PA4

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

    SPI1->CR1 |= CR1_SPI1EN;            // Enable SPI1

    SPI1->CR2 |= RXNEIE | ERRIE;        // RX buffer not empty and error interrupts
    
    SPI1->CR1 |= CR1_CRCEN;             // Enable CRC

    // Clear any pending data
    (void)SPI1->DR;
    (void)SPI1->SR;

    // Configure NVIC
    NVIC_SetPriority(SPI1_IRQn, 1);     // Medium priority
    NVIC_EnableIRQ(SPI1_IRQn);          // Enable SPI1 Interrupt
    NVIC_EnableIRQ(EXTI4_IRQn);         // Enable PA4 Interrupt - for SPI1 NSS 
}












// void prepare_queued_response(bool clean_rx) {
//     if (!response_queue[0].ready) {
//         printf("No response ready.\r\n");
//         return;
//     }

//     // Sanity check: prevent overflows or invalid memory access
//     if (response_queue[0].length == 0 || response_queue[0].length > sizeof(tx_buffer)) {
//         printf("ERROR: Invalid response length: %d\r\n", response_queue[0].length);
//         return;
//     }

//     if (clean_rx) {
//         memset((void *)rx_buffer, 0, sizeof(rx_buffer));  // Clean entire buffer to avoid partial leftovers
//         rx_index = 0;
//         start_transfer = 0;
//     }

//     // Log details before memcpy
//     printf("Preparing response of %d bytes...\r\n", response_queue[0].length);
    
//     memcpy((void *)tx_buffer, (void *)response_queue[0].data, response_queue[0].length);
    
//     tx_index = 0;
//     rx_index = 0;

//     printf("Response is ready.\r\n");
// }


// // EXTI4 Interrupt Handler: triggered when NSS interrupt flag is set
// void EXTI4_IRQHandler(void) {
//     if (EXTI->PR & PR_PR4) {            // NSS falling edge
//         EXTI->PR |= PR_PR4;             // Clear interrupt flag

//         if (response_queue[0].ready) {
//             // Enter Phase 2 - Prepare to start response phase
//             command_complete = 1;
//             tx_index = 0;
//         } else {
//             // Enter Phase 1 - Prepare to receive command 
//             command_complete = 0;
//             rx_index = 0;
//         }        
//     }
// }

        // // Phase 2: Master sends dummy. Slave responds with the correct response
        // // - The slave now has the correct response prepared
        // if (command_complete == 1) {
        //     rx_buffer[rx_index++] = received;            // Move to next byte
        //     if (received == '\n') {                      // Check if at the end
        //         rx_buffer[rx_index] = '\0';
        //         printf("Response to Master sent.\r\n");

        //         // Reset for next command
        //         response_queue[0].ready = false;
        //         tx_index = 0; 
        //         rx_index = 0;                          
        //         rx_complete = 0; 
        //         command_complete = 0;                       
        //     }

        //     // For each DUMMY byte received, respond with byte of Response
        //     if (response_queue[0].ready && tx_index < response_queue[0].length) {                  
        //         SPI1->DR = tx_buffer[tx_index++];
        //     } else {
        //         SPI1->DR = (response_queue[0].ready) ? DUMMY_BYTE : NO_RESPONSE;
        //     }
        //     return;
        // }

        // // Phase 1: Master sends the data. Slave responds with dummy
        // // - This allows the slave to prepare the correct reponse
        // if (command_complete == 0 && rx_index < BUFFER_SIZE) {
        //     rx_buffer[rx_index++] = received;               // Move to next byte
        //     if (received == '\n') {
        //         rx_buffer[rx_index] = '\0';
        //         rx_complete = 1; 
        //         command_complete = 1;
        //         rx_index = 0;

        //         printf("Master command received.\r\n");
        //         process_spi_command();
        //     }
        //     respond = 2;
        // }
        // // Slave responds with DUMMY for Phase 1
        // if (respond == 2) {
        //     SPI1->DR = DUMMY_BYTE;
        // }




// SPI1 response
// void prepare_spi_response(ResponseType type, const void *data, uint16_t length, bool clean_rx) 
// {
//     if (clean_rx) {
//         memset(rx_buffer, 0, BUFFER_SIZE);      // Clear receive buffer
//         transfer_complete = 0;                  // reset 
//     }
//
//     memset(tx_buffer, 0, BUFFER_SIZE);          // Clear transmit buffer
//
//     switch(type) {
//         case RESPONSE_TEXT: {       // Respond to health check
//             const char *text = (const char *)data;
//             uint16_t len = strnlen(text, BUFFER_SIZE-1);
//             memcpy(tx_buffer, text, len);
//             break;
//         }
//        
//         case RESPONSE_BUFFER: {     // Respond to buffer request
//             uint16_t bytes = length * sizeof(float);
//             if (bytes > BUFFER_SIZE) bytes = BUFFER_SIZE;
//             memcpy(tx_buffer, data, bytes);
//             break;
//         }
//     }
//     tx_index = 0;       // Reset tx state
//     rx_index = 0;       // Reset rx state
// }



// SPI1 Interrupt Handler: triggered when a byte is received (RXNE flag set) via SPI1
// void SPI1_IRQHandler(void) {
//     if (SPI1->SR & SR_RXNE) {                   // Byte received on RXNE
//         uint8_t received = SPI1->DR;            // Copy from Data Register
        
//         if (command_complete == 0) {
//             // Phase 1: Master sends the data. Slave responds with dummy
//             // - This allows the slave to prepare the correct reponse
//             rx_buffer[rx_index++] = received;               // Move to next byte
//             if (received == '\n') {
//                 rx_buffer[rx_index] = '\0';
//                 rx_index = 0;

//                 printf("\nMaster command received.\r\n");
//                 process_spi_command();
//             }
//             // Slave responds with DUMMY for Phase 1
//             SPI1->DR = DUMMY_BYTE;
//         }
//         else if (command_complete == 1) {
//             // Phase 2: Master sends dummy. Slave responds with the correct response
//             // - The slave now has the correct response prepared
//             rx_buffer[rx_index++] = received;            // Move to next byte
//             if (received == '\n') {                      // Check if at the end
//                 rx_buffer[rx_index] = '\0';
//                 printf("Response to Master sent.\r\n");

//                 // Reset for next command
//                 response_queue[0].ready = false;
//                 command_complete = 0;                       
//             }

//             // For each DUMMY byte received, respond with byte of Response
//             if (response_queue[0].ready && tx_index < response_queue[0].length) {                  
//                 SPI1->DR = tx_buffer[tx_index++];
//             } else {
//                 SPI1->DR = DUMMY_BYTE;
//             }
//         }
//     }
// }

