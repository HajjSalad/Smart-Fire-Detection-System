#ifndef SPI_COMM_H
#define SPI_COMM_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stm32f446xx.h"

#define START_BYTE             0x02
#define DUMMY_BYTE             0xFF
#define NO_RESPONSE            0xFE
#define BUFFER_SIZE             41
#define DATA_SIZE               41
#define RESPONSE_QUEUE_SIZE     1

typedef enum {
    RESPONSE_TEXT,
    RESPONSE_BUFFER
} ResponseType;

typedef struct {
    ResponseType type;
    uint8_t data[BUFFER_SIZE];
    uint16_t length;
    bool ready;
} SPI_Response;

// Public variables
extern volatile uint8_t rx_buffer[BUFFER_SIZE];
extern volatile uint8_t tx_buffer[BUFFER_SIZE];
extern volatile uint8_t start_transfer;
extern volatile uint8_t rx_complete;
extern volatile uint8_t command_complete;
extern volatile uint16_t rx_index;
extern volatile uint16_t tx_index;
extern volatile SPI_Response response_queue[RESPONSE_QUEUE_SIZE];

// Function prototypes
void queue_response(ResponseType type, const void *data, uint16_t length);
void prepare_queued_response(bool clean_rx);
void process_spi_command(void);

#endif // SPI_COMM_H