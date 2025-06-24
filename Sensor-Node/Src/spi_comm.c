
#include "spi.h"
#include "queue.h"
#include "systick.h"
#include "spi_comm.h"
#include "simulate.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f446xx.h"

#define SR_RXNE             (1U<<0)

#define PR_PR4              (1U<<4)


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

void printData() {
    if (isQueueEmpty()) {
        printf("Queue is empty\n\r");
        return;
    }

    int anomalyIndex = dequeue();
    CircularBuffer* buffer = &sensorBuffers[anomalyIndex];

    printf("Sensor %d buffer: [ ", anomalyIndex);

    if (buffer->count == 0) {
        // Buffer is empty, print zeros
        for (int i=0; i < CIRC_BUFFER_SIZE; i++) {
            if (i == CIRC_BUFFER_SIZE - 1) {
                printf("0.00 ]");
            } else {
                printf("0.00, ");
            }
        }
    } else {
        // Print stored values
        for (int i = 0; i < CIRC_BUFFER_SIZE; i++) {
            if (i == CIRC_BUFFER_SIZE - 1) {
                printf("%.2f ]", buffer->data[i]);
            } else {
                printf("%.2f, ", buffer->data[i]);
            }
            // if (buffers[i].count == 0) {
            //     printf("Buffer %d is empty\r\n", i);
            //     continue;
            // }
            // int value = (buffers[i].head - 1 + CIRC_BUFFER_SIZE) % CIRC_BUFFER_SIZE;
            // float lastValue = buffers[i].data[lastIndex];
            // printf("Buffer %d: %.2f\r\n", i, value);
        }
    }
    printf("\n\r");
}

// Handle the Master data received
void process_spi_command(void) {
    //printf("Received from Master: %s\r", rx_buffer);
           
    if (strncmp((char*)rx_buffer, "Are you Alive?", strlen("Are you Alive?")) == 0) {
        queue_response(RESPONSE_TEXT, "I'm Alive", strlen("I'm Alive"));
        printf("Health status to Master: I'm alive\n\r");
    }
    else if (strncmp((char*)rx_buffer, "Data Request", strlen("Data Request")) == 0) {
        if (!isQueueEmpty()) {
            int anomalyIndex = dequeue();
            CircularBuffer* buffer = &sensorBuffers[anomalyIndex];

            // Queue the buffer data
            queue_response(RESPONSE_BUFFER, buffer->data, CIRC_BUFFER_SIZE * sizeof(float));

            printf("Sending buffer from Sensor %d: [ ", anomalyIndex);
            for (int i = 0; i < CIRC_BUFFER_SIZE; i++) {
                if (i == CIRC_BUFFER_SIZE - 1) {
                    printf("%.2f ]\n\r", buffer->data[i]);
                } else {
                    printf("%.2f, ", buffer->data[i]);
                }
            }
        } else {
            queue_response(RESPONSE_TEXT, "No anomaly data", strlen("No anomaly data"));
            printf("Queue is empty\n\r");
        }
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

                printf("Master command received: %s\r", rx_buffer);
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


Where to pick up:
    - Print sensor buffer values - done
    - Send the sensor buffer values to ESP32 - done
    - Check ESP32 and see whether the buffer is being received
    - Clean the codes
    - Add documentation