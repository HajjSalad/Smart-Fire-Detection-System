/**
 * @file main.c
 * @brief Master node for polling health and requesting sensor data via SPI,
 *        and publishing data to AWS MQTT
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

#define MQTT_TOPIC "sensor_node_1"

extern volatile int flagRequest;

/**
 * @brief Publishes a health-check message ("Node 1 alive") to AWS MQTT.
 */
void publish_health(void)
{
    const char *message = "Node 1 alive";
    const char json_payload[] = "{\"message\": \"Node 1 alive\"}";
    printf("Publish health to AWS: %s\n", message);
    esp_mqtt_client_handle_t client = get_mqtt_client();
    int msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC, json_payload, 0, 1, 0);
    if (msg_id == -1) {
        printf("MQTT publish health failed.");
    }
}

/**
 * @brief Publishes parsed float sensor datato AWS MQTT.
 * 
 * @param data Pointer to raw byte buffer containing float values.
 * @param len Length of the byte buffer (40 bytes = 10 floats).
 */
void publish_sensor_data(const uint8_t *data, size_t len)
{
    if (!data || len == 0) return;

    float values[10] = {0};

    // Convert each 4-byte chunk into a float
    for (int i = 0; i < 10; i++) {
        uint32_t temp = (data[i*4 + 3] << 24) | (data[i*4 + 2] << 16) |
                        (data[i*4 + 1] << 8)  | (data[i*4 + 0]);
        memcpy(&values[i], &temp, sizeof(float));
    }

    // Format into JSON
    char json_payload[256] = {0};
    snprintf(json_payload, sizeof(json_payload),
             "[%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f]",
             values[0], values[1], values[2], values[3], values[4],
             values[5], values[6], values[7], values[8], values[9]);

    printf("JSON Payload: %s\n", json_payload);

    esp_mqtt_client_handle_t client = get_mqtt_client();
    int msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC, json_payload, 0, 1, 0);
    if (msg_id == -1) {
        printf("MQTT publish sensor data failed.");
    }
}

/**
 * @brief Polls slave device for health status via SPI
 */
void poll_health(void)
{
    static uint8_t tx_alive[16] = "Are you Alive?\n";
    static uint8_t tx_dummy[16];
    memset(tx_dummy, 0xFF, 16);
    tx_dummy[15] = '\n';              
    const size_t msg_len = strlen((char*)tx_alive);

    static uint8_t rx_phase1[16] = {0};            // DUMMY bytes from slave
    static uint8_t rx_phase2[16] = {0};            // Actual response from slave

    // Phase 1: Send command 
    spi_transaction_t phase1 = {
        .length = msg_len * 8,
        .tx_buffer = tx_alive,
        .rx_buffer = rx_phase1
    };

    esp_err_t ret1 = spi_device_transmit(handle, &phase1);
    if (ret1 == ESP_OK) {
        printf("\n[Alive check] Sent: %s", tx_alive);
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
        printf("[Alive Check] Received: %s\n", rx_phase2 + 1);  // Print the response
        publish_health(); 
    } else {
        printf("[Alive Check] SPI Transmission Failed\n");
    }
}

/**
 * @brief Sends a sensor data request to the slave device via SPI
 */
void request_data(void)
{
    static uint8_t tx_data[16] = "Data Request\n";
    static uint8_t tx_dummy[42];
    memset(tx_dummy, 0xFF, 41);
    tx_dummy[41] = '\n';              
    const size_t msg_len = strlen((char*)tx_data);

    static uint8_t rx_phase1[16] = {0};            // DUMMY bytes from slave
    static uint8_t rx_phase2[42] = {0};            // Actual response from slave

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
            printf("%02X ", rx_phase2[1 + i]);
        }
        printf("\n\n");
        publish_sensor_data(rx_phase2 + 1, 40);
    }
}

/**
 * @brief FreeRTOS task to periodically check fo anomaly, or check otherwise check health.
 */
static void main_spi_loop()
{
    while (1) {
        if(ulTaskNotifyTake(pdTRUE, 0)) {       // Wait for notification - non-blocking
            flagRequest = 1;
            printf("\n[Anomaly] Interrupt received. Requesting sensor data.\n");
        }

        // Check if anomaly trigger, otherwise poll for health
        if (flagRequest) {          // Check flag
            request_data();         // Request sensor data
            flagRequest = 0;        // Reset the flag
        } else {
            poll_health();          // Poll for health status
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Entry point of the master node application.
 * 
 * Initializes peripherals, WiFi, MQTT, then starts the polling task.
 */
void app_main(void)
{
    printf("FACP Master Node Start\n");

    spi_init();                 // Init SPI
    interrupt_line_init();      // Init Interrupt Line
    nvs_flash_init();           // Init Non-Volatile Storage
    wifi_connection();          // Connect Wi-Fi
    
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n");

    mqtt_app_start();           // Start mqtt connection
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
    // Task: Poll for health status or request data
    xTaskCreate(main_spi_loop, "spi_alive_task", 4096, NULL, 1, &spi_task_handle);
}


