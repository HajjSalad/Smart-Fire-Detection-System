#ifndef SPI_H
#define SPI_H

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

void Systick_Runner();
void queue_response(ResponseType type, const void *data, uint16_t length);
void prepare_queued_response(bool clean_rx);
void process_spi_command(void);
void interrupt_line_init();
void spi1_slave_init();

#endif // SPI_H


// // Task 2: Request sensor data on anomaly detection
// static void interrupt_handler_task() 
// {
//     uint8_t tx_data[16] = "Data Request\n";
//     uint8_t tx_dummy[42];
//     memset(tx_dummy, 0xFF, 41);
//     tx_dummy[41] = '\n';              
//     const size_t msg_len = strlen((char*)tx_data);  

//     while (1) {
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        // Wait for notification from ISR
//         printf("[Anomaly] Interrupt received. Requesting sensor data.\n");

//         uint8_t rx_phase1[16] = {0};            // DUMMY bytes from slave        
//         uint8_t rx_phase2[42] = {0};            // Actual response from slave

//         // Phase 1: Send command 
//         spi_transaction_t phase1 = {
//             .length = msg_len * 8,
//             .tx_buffer = tx_data,
//             .rx_buffer = rx_phase1
//         };

//         esp_err_t ret1 = spi_device_transmit(handle, &phase1);
//         if (ret1 == ESP_OK) {
//             printf("Phase 1 Command Sent: %s", tx_data);
//             printf("Phase 1 DUMMY received: ");
//             for (int i = 0; i < msg_len; i++) {
//                 printf("%02X ", rx_phase1[i]);
//             }
//             printf("\n\n");
//         } else {
//             printf("Phase 1 Command Send failed\n");
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             continue;
//         }

//         vTaskDelay(50 / portTICK_PERIOD_MS);


//         // Phase 2: Send DUMMY bytes to read response 
//         spi_transaction_t phase2 = {
//             .length = 41 * 8,
//             .tx_buffer = tx_dummy,
//             .rx_buffer = rx_phase2
//         };

//         esp_err_t ret2 = spi_device_transmit(handle, &phase2);
//         if (ret2 == ESP_OK) {
//             printf("Phase 2 DUMMY sent: ");
//             for (int i = 0; i < 41; i++) {
//                 printf("%02X ", tx_dummy[i]);
//             }
//             printf("\n");
//             printf("Phase 2 response: ");
//             for (int i = 0; i < 40; i++) {
//                 printf("%02X ", rx_phase2[2 + i]);
//             }
//             printf("\n\n");
//         }
//     }
//     vTaskDelay(5000 / portTICK_PERIOD_MS);
// }

// static void spi_alive_task()
// {
//     // uint8_t tx_data[16] = "Are you Alive?";  
//     // uint8_t rx_data[16] = {0};
//     //
//     // while (1) {
//     //     const size_t msg_len = strlen((char*)tx_data);
//     //
//     //     spi_transaction_t t = {
//     //         .length = msg_len * 8,
//     //         .tx_buffer = tx_data,
//     //         .rx_buffer = rx_data,
//     //         .rxlength = msg_len * 8
//     //     };
//     //
//     //     esp_err_t ret = spi_device_transmit(handle, &t);
//     //     if (ret == ESP_OK) {
//     //         rx_data[msg_len] = '\0';
//     //         // There's a 00 00 padding at the front - skipped it with +2
//     //         printf("[Alive Check] Received: %s\n", rx_data + 2);  // Print as string
//     //     } else {
//     //         printf("[Alive Check] SPI Transmission Failed\n");
//     //     }
// //
//     //     vTaskDelay(5000 / portTICK_PERIOD_MS);
//     // }

//     uint8_t tx_data[16] = "Data Request\n";
//     // uint8_t tx_dummy[16];
//     // memset(tx_dummy, 0xFF, 16);
//     // tx_dummy[15] = '\n'; 
//     uint8_t tx_dummy[42];
//     memset(tx_dummy, 0xFF, 41);
//     tx_dummy[41] = '\n';              
//     const size_t msg_len = strlen((char*)tx_data);

//     while (1) {
//         uint8_t rx_phase1[16] = {0};            // DUMMY bytes from slave
//         //uint8_t rx_phase2[16] = {0};            // Actual response from slave
//         uint8_t rx_phase2[42] = {0};

//         // Phase 1: Send command 
//         spi_transaction_t phase1 = {
//             .length = msg_len * 8,
//             .tx_buffer = tx_data,
//             .rx_buffer = rx_phase1
//         };

//         esp_err_t ret1 = spi_device_transmit(handle, &phase1);
//         if (ret1 == ESP_OK) {
//             printf("Phase 1 Command Sent: %s", tx_data);
//             printf("Phase 1 DUMMY received: ");
//             for (int i = 0; i < msg_len; i++) {
//                 printf("%02X ", rx_phase1[i]);
//             }
//             printf("\n\n");
//         } else {
//             printf("Phase 1 Command Send failed\n");
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             continue;
//         }

//         vTaskDelay(50 / portTICK_PERIOD_MS);

//         // Phase 2: Send DUMMY bytes to read response 
//         spi_transaction_t phase2 = {
//             .length = 41 * 8,
//             .tx_buffer = tx_dummy,
//             .rx_buffer = rx_phase2
//         };

//         esp_err_t ret2 = spi_device_transmit(handle, &phase2);
//         if (ret2 == ESP_OK) {
//             printf("Phase 2 DUMMY sent: ");
//             for (int i = 0; i < 41; i++) {
//                 printf("%02X ", tx_dummy[i]);
//             }
//             printf("\n");
//             printf("Phase 2 response: ");
//             // int length = sizeof(rx_phase2);
//             // // Check if response is floats (first 4 bytes as float)
//             // if (length >= 41) {
//             //     for (int i = 0; i < 10; i++) {
//             //         float val;
//             //         memcpy(&val, &rx_phase2[i * 4], sizeof(float));
//             //         printf("[%d]: %.2f  ", i, val);
//             //     }
//             // }
//             for (int i = 0; i < 10; i++) {
//                 float val;
//                 memcpy(&val, &rx_phase2[2 + i * 4], sizeof(float));
//                 printf("[%d]: %.2f  ", i, val);
//             }
//             printf("\nRaw bytes: ");
//             for (int i = 0; i < 40; i++) {
//                 printf("%02X ", rx_phase2[2 + i]);
//             }
//             printf("\n\n");
//         }

//         vTaskDelay(5000 / portTICK_PERIOD_MS);
//     }
// }