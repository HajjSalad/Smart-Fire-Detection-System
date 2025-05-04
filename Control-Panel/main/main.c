/*
//  Entry point to the program
//  Task Creation
*/

#include "spi.h"
#include "uart.h"
#include "wifi.h"
#include "cloud.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "gpio_interrupt.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"

#define MAX_NODES      8

typedef struct {
    uint8_t node_id;
    gpio_num_t ss_pin;
    gpio_num_t shared_pin;
    bool has_pending_request;
} sensor_node_t;

sensor_node_t nodes[MAX_NODES];
uint8_t node_count = 0;

extern volatile int flagRequest;

void publish_health(void)
{
    const char *message = "Node 1 is alive";
    printf("Publishing to AWS (health): %s\n", message);
    esp_mqtt_client_handle_t client = get_mqtt_client();
    esp_mqtt_client_publish(client, "sensor/health", message, 0, 1, 0);
}

void publish_sensor_data(char *response)
{
    char mqtt_payload[42] = {0};
    strncpy(mqtt_payload, response, sizeof(mqtt_payload) - 1);  // Safe copy
    printf("Publishing to AWS (sensor data): %s\n", mqtt_payload);
    esp_mqtt_client_handle_t client = get_mqtt_client();
    esp_mqtt_client_publish(client, "sensor/data", mqtt_payload, 0, 1, 0);
}

// Poll for health status
void poll_health(void)
{
    uint8_t tx_alive[16] = "Are you Alive?\n";
    uint8_t tx_dummy[16];
    memset(tx_dummy, 0xFF, 16);
    tx_dummy[15] = '\n';              
    const size_t msg_len = strlen((char*)tx_alive);

    uint8_t rx_phase1[16] = {0};            // DUMMY bytes from slave
    uint8_t rx_phase2[16] = {0};            // Actual response from slave

    // Phase 1: Send command 
    spi_transaction_t phase1 = {
        .length = msg_len * 8,
        .tx_buffer = tx_alive,
        .rx_buffer = rx_phase1
    };

    esp_err_t ret1 = spi_device_transmit(handle, &phase1);
    if (ret1 == ESP_OK) {
        printf("[Alive check] Sent: %s", tx_alive);
    } else {
        printf("Phase 1 Command Send failed\n");
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);

    // Phase 2: Send DUMMY bytes to read response 
    spi_transaction_t phase2 = {
        .length = msg_len * 8,
        .tx_buffer = tx_dummy,
        .rx_buffer = rx_phase2
    };

    esp_err_t ret2 = spi_device_transmit(handle, &phase2);
    if (ret2 == ESP_OK) {
        rx_phase2[10] = '\0';
        // There's a 00 padding at the front - skipped it with +1
        printf("[Alive Check] Received: %s\n\n", rx_phase2 + 1);  // Print the response
        publish_health(); 
    } else {
        printf("[Alive Check] SPI Transmission Failed\n");
    }
}

// Request sensor data on anomaly detection
void request_data(void)
{
    uint8_t tx_data[16] = "Data Request\n";
    uint8_t tx_dummy[42];
    memset(tx_dummy, 0xFF, 41);
    tx_dummy[41] = '\n';              
    const size_t msg_len = strlen((char*)tx_data);

    uint8_t rx_phase1[16] = {0};            // DUMMY bytes from slave
    uint8_t rx_phase2[42] = {0};            // Actual response from slave

    // Phase 1: Send command 
    spi_transaction_t data_phase1 = {
        .length = msg_len * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_phase1
    };

    esp_err_t ret1 = spi_device_transmit(handle, &data_phase1);
    if (ret1 == ESP_OK) {
        printf("Phase 1 Command Sent: %s", tx_data);
        printf("Phase 1 DUMMY received: ");
        for (int i = 0; i < msg_len; i++) {
            printf("%02X ", rx_phase1[i]);
        }
        printf("\n\n");
    } else {
        printf("Phase 1 Command Send failed\n");
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);

    // Phase 2: Send DUMMY bytes to read response 
    spi_transaction_t data_phase2 = {
        .length = 41 * 8,
        .tx_buffer = tx_dummy,
        .rx_buffer = rx_phase2
    };

    esp_err_t ret2 = spi_device_transmit(handle, &data_phase2);
    if (ret2 == ESP_OK) {
        printf("Phase 2 DUMMY sent: ");
        for (int i = 0; i < 41; i++) {
            printf("%02X ", tx_dummy[i]);
        }
        printf("\n");
        printf("Phase 2 response: ");
        for (int i = 0; i < 40; i++) {
            printf("%02X ", rx_phase2[2 + i]);
        }
        publish_sensor_data((char *)(rx_phase2 + 2));
        printf("\n\n");
    }
}

// Task: Poll for health status or Request Data
static void spi_master_task()
{
    while (1) {
        if(ulTaskNotifyTake(pdTRUE, 0)) {       // Wait for notification - non-blocking
            flagRequest = 1;
            printf("[Anomaly] Interrupt received. Requesting sensor data.\n");
        }

        if (flagRequest) {          // Check flag
            request_data();         // request data
            flagRequest = 0;        // Reset the flag
        } else {
            poll_health();          // Poll for health status
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    printf("FACP Master Node Start\n");

    spi_init();                 // Init SPI
    interrupt_line_init();      // Init Interrupt Line
    nvs_flash_init();           // Init Non-Volatile Storage
    wifi_connection();          // Connect Wi-Fi
    
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n");
    
    mqtt_app_start();
    
    // Task: Poll for health status or request data
    xTaskCreate(spi_master_task, "spi_alive_task", 1024 * 2, NULL, 1, &spi_task_handle);
}

















// // Task 2: Request sensor data on anomaly detection
// static void interrupt_handler_task()
// {
//     uint8_t tx_request[16] = "Data Request\n";
//     uint8_t tx_dumm[42];
//     memset(tx_dumm, 0xFF, 41);
//     tx_dumm[41] = '\n';              
//     const size_t msg_len = strlen((char*)tx_request);

//     while (1) {
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        // Wait for notification from ISR
//         printf("[Anomaly] Interrupt received. Requesting sensor data.\n");

//         uint8_t rx_response1[16] = {0};            // DUMMY bytes from slave
//         uint8_t rx_response2[42] = {0};            // Actual response from slave

//         // Phase 1: Send command 
//         spi_transaction_t data_phase1 = {
//             .length = msg_len * 8,
//             .tx_buffer = tx_request,
//             .rx_buffer = rx_response1
//         };

//         esp_err_t ret1 = spi_device_transmit(handle, &data_phase1);
//         if (ret1 == ESP_OK) {
//             printf("Phase 1 Command Sent: %s", tx_request);
//             printf("Phase 1 DUMMY received: ");
//             for (int i = 0; i < msg_len; i++) {
//                 printf("%02X ", rx_response1[i]);
//             }
//             printf("\n\n");
//         } else {
//             printf("Phase 1 Command Send failed\n");
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             continue;
//         }

//         vTaskDelay(50 / portTICK_PERIOD_MS);

//         // Phase 2: Send DUMMY bytes to read response 
//         spi_transaction_t data_phase2 = {
//             .length = 41 * 8,
//             .tx_buffer = tx_dumm,
//             .rx_buffer = rx_response2
//         };

//         esp_err_t ret2 = spi_device_transmit(handle, &data_phase2);
//         if (ret2 == ESP_OK) {
//             printf("Phase 2 DUMMY sent: ");
//             for (int i = 0; i < 41; i++) {
//                 printf("%02X ", tx_dumm[i]);
//             }
//             printf("\n");
//             printf("Phase 2 response: ");
//             for (int i = 0; i < 40; i++) {
//                 printf("%02X ", rx_response2[2 + i]);
//             }
//             printf("\n\n");
//         }

//         vTaskDelay(5000 / portTICK_PERIOD_MS);
//     }
// }


// // Task 1: Poll for health status
// static void spi_alive_task()
// {
//     uint8_t tx_alive[16] = "Are you Alive?\n";
//     uint8_t tx_dummy[16];
//     memset(tx_dummy, 0xFF, 16);
//     tx_dummy[15] = '\n';              
//     const size_t msg_len = strlen((char*)tx_alive);
//     int flagRequest = 0;

//     while (1) {

//         if(ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
//             flagRequest = 1;
//         }

//         uint8_t rx_phase1[16] = {0};            // DUMMY bytes from slave
//         uint8_t rx_phase2[16] = {0};            // Actual response from slave

//         // Phase 1: Send command 
//         spi_transaction_t phase1 = {
//             .length = msg_len * 8,
//             .tx_buffer = tx_alive,
//             .rx_buffer = rx_phase1
//         };

//         esp_err_t ret1 = spi_device_transmit(handle, &phase1);
//         if (ret1 == ESP_OK) {
//             printf("[Alive check] Sent: %s", tx_alive);
//             // printf("Phase 1 DUMMY received: ");
//             // for (int i = 0; i < msg_len; i++) {
//             //     printf("%02X ", rx_phase1[i]);
//             // }
//             // printf("\n\n");
//         } else {
//             printf("Phase 1 Command Send failed\n");
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             continue;
//         }

//         vTaskDelay(50 / portTICK_PERIOD_MS);

//         // Phase 2: Send DUMMY bytes to read response 
//         spi_transaction_t phase2 = {
//             .length = msg_len * 8,
//             .tx_buffer = tx_dummy,
//             .rx_buffer = rx_phase2
//         };

//         esp_err_t ret2 = spi_device_transmit(handle, &phase2);
//         if (ret2 == ESP_OK) {
//             rx_phase2[10] = '\0';
//             // There's a 00 padding at the front - skipped it with +1
//             printf("[Alive Check] Received: %s\n\n", rx_phase2 + 1);  // Print as string
//         } else {
//             printf("[Alive Check] SPI Transmission Failed\n");
//         }
//         // if (ret2 == ESP_OK) {
//         //     printf("Phase 2 DUMMY sent: ");
//         //     for (int i = 0; i < 41; i++) {
//         //         printf("%02X ", tx_dummy[i]);
//         //     }
//         //     printf("\n");
//         //     printf("Phase 2 response: ");
//         //     // int length = sizeof(rx_phase2);
//         //     // // Check if response is floats (first 4 bytes as float)
//         //     // if (length >= 41) {
//         //     //     for (int i = 0; i < 10; i++) {
//         //     //         float val;
//         //     //         memcpy(&val, &rx_phase2[i * 4], sizeof(float));
//         //     //         printf("[%d]: %.2f  ", i, val);
//         //     //     }
//         //     // }
//         //     for (int i = 0; i < 10; i++) {
//         //         float val;
//         //         memcpy(&val, &rx_phase2[2 + i * 4], sizeof(float));
//         //         printf("[%d]: %.2f  ", i, val);
//         //     }
//         //     printf("\nRaw bytes: ");
//         //     for (int i = 0; i < 40; i++) {
//         //         printf("%02X ", rx_phase2[2 + i]);
//         //     }
//         //     printf("\n\n");
//         // }

//         vTaskDelay(5000 / portTICK_PERIOD_MS);
//     }
// }

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

// static void interrupt_handler_task() 
// {
//     uint8_t tx_data[16] = "Data Request";  
//     uint8_t rx_data[48] = {0};

//     while (1) {
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        // Wait for notification from ISR
//         printf("[Anomaly] Interrupt received. Requesting buffer...\n");

//         const size_t msg_len = strlen((char*)tx_data);
//         const size_t rx_len = strlen((char*)rx_data);

//         spi_transaction_t t = {
//             .length = msg_len * 8, 
//             .tx_buffer = tx_data,
//             .rxlength =  rx_len* 8,
//             .rx_buffer = rx_data
//         };

//         esp_err_t ret = spi_device_transmit(handle, &t);
//         if (ret == ESP_OK) {
//             // printf("Raw RX Data: ");
//             // for (int i = 0; i < sizeof(rx_data); i++) {
//             //     printf("%02X ", rx_data[i]);
//             // }
//             // printf("\n");
//             int length = sizeof(rx_data);
//             // Check if response is floats (first 4 bytes as float)
//             if (length >= 48) {
//                 for (int i = 0; i < 10; i++) {
//                     float val;
//                     memcpy(&val, &rx_data[2 + i * 4], sizeof(float));
//                     printf("[%d]: %.2f  ", i, val);
//                 }
//             } else {
//                 printf("Received: %s\n", rx_data);
//             }
//             printf("\n\n");
//         } else {
//             printf("[Anomaly] SPI Transmission Failed\n");
//         }
//     }
// }

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"

// #include "driver/spi_master.h"
// #include "driver/gpio.h"
// #include "sdkconfig.h"
// #include "esp_log.h"

// #define MOSI             23
// #define MISO             19
// #define SCK              18
// #define SS                5
// #define INTERRUPT_LINE   22

// spi_device_handle_t handle;
// TaskHandle_t spi_task_handle = NULL;

// void spi_init ()
// {
//     // Set the communication parameters
//     const spi_bus_config_t spi_config = {
//         .miso_io_num = MISO,
//         .mosi_io_num = MOSI,
//         .sclk_io_num = SCK,
//         .quadwp_io_num = -1,
//         .quadhd_io_num = -1,
//         .max_transfer_sz = 32
//     };

//     // Set SPI device interface
//     const spi_device_interface_config_t spi_device_config = {
//         .clock_speed_hz = 1*1000*1000,  // Set the clock speed (1 MHz)
//         .duty_cycle_pos = 128,          // Set the duty cycle, 128 = 50%
//         .mode           = 0,            // SPI mode (0: CPOL = 0, CPHA = 0)
//         .spics_io_num   = SS,           // Slave Select
//         .queue_size     = 1,            // Number of transactions to queue
//     };

//     spi_bus_initialize(SPI2_HOST, &spi_config, SPI_DMA_CH_AUTO);
//     spi_bus_add_device(SPI2_HOST, &spi_device_config, &handle);
// }

// void IRAM_ATTR sensor_isr_handler(void* arg) {
//     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//     vTaskNotifyGiveFromISR(spi_task_handle, &xHigherPriorityTaskWoken);
//     if (xHigherPriorityTaskWoken) {
//         portYIELD_FROM_ISR();
//     }
// }

// void interrupt_line_init() {
//     gpio_config_t io_conf = {
//         .intr_type = GPIO_INTR_POSEDGE,             // Interrupt on the positive edge of the clock      
//         .mode = GPIO_MODE_INPUT,
//         .pin_bit_mask = (1ULL << INTERRUPT_LINE),
//         .pull_down_en = 0,
//         .pull_up_en = 1
//     };
//     gpio_config(&io_conf);

//     gpio_install_isr_service(0);  // 0 = default ISR service
//     gpio_isr_handler_add(INTERRUPT_LINE, sensor_isr_handler, NULL);
// }

// static void spi_alive_task()
// {
//     uint8_t tx_data[32] = "Are you Alive?";  
//     uint8_t rx_data[32] = {0};

//     while (1) {
//         const size_t msg_len = strlen((char*)tx_data);

//         spi_transaction_t t = {
//             .length = msg_len * 8,
//             .tx_buffer = tx_data,
//             .rx_buffer = rx_data,
//             .rxlength = msg_len * 8
//         };

//         esp_err_t ret = spi_device_transmit(handle, &t);
//         if (ret == ESP_OK) {
//             rx_data[msg_len] = '\0';
//             // There's a 00 00 padding at the front - skipped it with +2
//             printf("[Alive Check] Received: %s\n", rx_data + 2);  // Print as string
//         } else {
//             printf("[Alive Check] SPI Transmission Failed\n");
//         }

//         vTaskDelay(5000 / portTICK_PERIOD_MS);
//     }
// }

// static void interrupt_handler_task() 
// {
//     uint8_t tx_data[48] = "Data Request";  
//     uint8_t rx_data[48] = {0};

//     while (1) {
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        // Wait for notification from ISR
//         printf("[Anomaly] Interrupt received. Requesting buffer...\n");

//         spi_transaction_t t = {
//             .length = 40 * 8, 
//             .tx_buffer = tx_data,
//             .rxlength = 40 * 8, // Full buffer size
//             .rx_buffer = rx_data
//         };

//         esp_err_t ret = spi_device_transmit(handle, &t);
//         if (ret == ESP_OK) {
//             // printf("Raw RX Data: ");
//             // for (int i = 0; i < sizeof(rx_data); i++) {
//             //     printf("%02X ", rx_data[i]);
//             // }
//             // printf("\n");
//             int length = sizeof(rx_data);;
//             // Check if response is floats (first 4 bytes as float)
//             if (length >= 48) {
//                 for (int i = 0; i < 10; i++) {
//                     float val;
//                     memcpy(&val, &rx_data[2 + i * 4], sizeof(float));
//                     printf("[%d]: %.2f  ", i, val);
//                 }
//             } else {
//                 printf("Received: %s\n", rx_data);
//             }
//             printf("\n");
//         } else {
//             printf("[Anomaly] SPI Transmission Failed\n");
//         }
//         //vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

// void app_main(void)
// {
//     printf("FACP Master Node Start\n");

//     spi_init();     // Initialize spi
//     interrupt_line_init();

//     // Create a task to poll for sensor node status
//     // xTaskCreate(spi_alive_task, "spi_alive_task", 1024 * 2, NULL, 2, NULL);
//     // Create a task to request sensor reading upon anomaly detection
//     // xTaskCreate(interrupt_handler_task, "interrupt_handler_task", 1024 * 2, NULL, 3, &spi_task_handle);
// }
