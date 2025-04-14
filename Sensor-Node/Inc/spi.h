#ifndef SPI_H
#define SPI_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stm32f446xx.h"

#define BUFFER_SIZE  64
#define BUFFER_SIZE  64

typedef enum {
    RESPONSE_TEXT,
    RESPONSE_BUFFER
} ResponseType;

// Public variables
extern volatile uint8_t rx_buffer[BUFFER_SIZE];
extern volatile uint8_t tx_buffer[BUFFER_SIZE];
extern volatile uint8_t transfer_complete;
extern volatile uint16_t rx_index;
extern volatile uint16_t tx_index;

// Function prototypes

void prepare_spi_response(ResponseType type, const void *data, uint16_t length, bool clean_rx);
void interrupt_line_init();
void spi1_slave_init();

#endif // SLAVE_H