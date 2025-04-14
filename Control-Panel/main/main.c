/*
//  SPI Master Full Duplex
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define MOSI             23
#define MISO             19
#define SCK              18
#define SS                5
#define INTERRUPT_LINE   22

spi_device_handle_t handle;
TaskHandle_t spi_task_handle = NULL;

void spi_init ()
{
    // Set the communication parameters
    const spi_bus_config_t spi_config = {
        .miso_io_num = MISO,
        .mosi_io_num = MOSI,
        .sclk_io_num = SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };

    // Set SPI device interface
    const spi_device_interface_config_t spi_device_config = {
        .clock_speed_hz = 1*1000*1000,  // Set the clock speed (1 MHz)
        .duty_cycle_pos = 128,          // Set the duty cycle, 128 = 50%
        .mode           = 0,            // SPI mode (0: CPOL = 0, CPHA = 0)
        .spics_io_num   = SS,           // Slave Select
        .queue_size     = 1,            // Number of transactions to queue
    };

    spi_bus_initialize(SPI2_HOST, &spi_config, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &spi_device_config, &handle);
}

void IRAM_ATTR sensor_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(spi_task_handle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void interrupt_line_init() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,             // Interrupt on the positive edge of the clock      
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << INTERRUPT_LINE),
        .pull_down_en = 0,
        .pull_up_en = 1
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);  // 0 = default ISR service
    gpio_isr_handler_add(INTERRUPT_LINE, sensor_isr_handler, NULL);
}

// static void spi_alive_task()
// {
//     uint8_t tx_data[32] = "Are you Alive?";  
//     uint8_t rx_data[32] = {0};
//
//     while (1) {
//         const size_t msg_len = strlen((char*)tx_data);
//
//         spi_transaction_t t = {
//             .length = msg_len * 8,
//             .tx_buffer = tx_data,
//             .rx_buffer = rx_data,
//             .rxlength = msg_len * 8
//         };
//
//         esp_err_t ret = spi_device_transmit(handle, &t);
//         if (ret == ESP_OK) {
//             rx_data[msg_len] = '\0';
//             // There's a 00 00 padding at the front - skipped it with +2
//             printf("[Alive Check] Received: %s\n", rx_data + 2);  // Print as string
//         } else {
//             printf("[Alive Check] SPI Transmission Failed\n");
//         }
//
//         vTaskDelay(5000 / portTICK_PERIOD_MS);
//     }
// }

static void interrupt_handler_task() 
{
    uint8_t tx_data[48] = "Data Request";  
    uint8_t rx_data[48] = {0};

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        // Wait for notification from ISR
        printf("[Anomaly] Interrupt received. Requesting buffer...\n");

        spi_transaction_t t = {
            .length = 40 * 8, 
            .tx_buffer = tx_data,
            .rxlength = 40 * 8, // Full buffer size
            .rx_buffer = rx_data
        };

        esp_err_t ret = spi_device_transmit(handle, &t);
        if (ret == ESP_OK) {
            // printf("Raw RX Data: ");
            // for (int i = 0; i < sizeof(rx_data); i++) {
            //     printf("%02X ", rx_data[i]);
            // }
            // printf("\n");
            int length = sizeof(rx_data);;
            // Check if response is floats (first 4 bytes as float)
            if (length >= 48) {
                for (int i = 0; i < 10; i++) {
                    float val;
                    memcpy(&val, &rx_data[2 + i * 4], sizeof(float));
                    printf("[%d]: %.2f  ", i, val);
                }
            } else {
                printf("Received: %s\n", rx_data);
            }
            printf("\n");
        } else {
            printf("[Anomaly] SPI Transmission Failed\n");
        }
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    printf("FACP Master Node Start\n");

    spi_init();     // Initialize spi
    interrupt_line_init();

    // Create a task to poll for sensor node status
    //xTaskCreate(spi_alive_task, "spi_alive_task", 1024 * 2, NULL, 2, NULL);
    // Create a task to request sensor reading upon anomaly detection
    xTaskCreate(interrupt_handler_task, "interrupt_handler_task", 1024 * 2, NULL, 3, &spi_task_handle);
}
