/**
 * @file cloud.c
 * @brief MQTT over TLS communication logic
 *
 * Implements secure MQTT client initialization, event handling,
 * and cloud communication for ESP32 IoT devices.
 */

#include "cloud.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <sys/param.h>

#include "esp_tls.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "sdkconfig.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "mqtt_client.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

static const char *TAG = "MQTT";

/**
 * @brief Externally linked embedded certificate, key, and root CA for TLS.
 */
extern const uint8_t device_cert_pem_start[] asm("_binary_device_cert_pem_start");
extern const uint8_t private_key_pem_start[] asm("_binary_private_key_pem_start");
extern const uint8_t root_ca_pem_start[]     asm("_binary_root_ca_pem_start");

/**
 * @brief Global MQTT client handle.
 */
esp_mqtt_client_handle_t client = NULL;

/**
 * @brief Callback for handling MQTT events.
 *
 * This function processes connection, subscription, data, and error events
 * from the MQTT client.
 *
 * @param event Pointer to the MQTT event structure.
 * @return esp_err_t ESP_OK on success.
 */
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Connected to MQTT broker");
        esp_mqtt_client_subscribe(client, "cloud_comments", 0);
        esp_mqtt_client_publish(client, "sensor_node_1", "{\"message\": \"Sensor Node 1 connected\"}", 0, 1, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Disconnected from MQTT broker");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "Subscribed successfully, msg_id=%d", (int)event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "Unsubscribed, msg_id=%d", (int)event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "Message published, msg_id=%d", (int)event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Received MQTT data");
        printf("TOPIC: %.*s\n", event->topic_len, event->topic);
        printf("DATA: %.*s\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT error occurred");
        break;
    default:
        ESP_LOGI(TAG, "Unhandled event id: %d", event->event_id);
        break;
    }
    return ESP_OK;
}

/**
 * @brief Wrapper for event loop registration.
 *
 * This function is called by the ESP event loop and forwards MQTT event data
 * to the internal handler.
 *
 * @param handler_args Not used.
 * @param base Event base.
 * @param event_id Event ID.
 * @param event_data Event-specific data.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    mqtt_event_handler_cb(event_data);
}

/**
 * @brief Initializes and starts the MQTT client with TLS credentials.
 *
 * Sets up the MQTT broker address, certificates, private key, and event handler.
 * Starts the client connection process.
 */
void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = "mqtts://acqql8v2q7crf-ats.iot.us-east-1.amazonaws.com",
            .verification.certificate = (const char *)root_ca_pem_start         // Root CA
        },
        .credentials = {
            .authentication = {
                .certificate = (const char*)device_cert_pem_start,              // Device certificate
                .key = (const char *)private_key_pem_start,                     // Private Key
            },
        },
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

/**
 * @brief Returns the MQTT client handle for use elsewhere in the application.
 *
 * @return esp_mqtt_client_handle_t The MQTT client instance.
 */
esp_mqtt_client_handle_t get_mqtt_client(void)
{
    return client;
}









// static const char *TAG = "MQTT_TCP";

// extern const uint8_t device_cert_pem_start[] asm("_binary_device_cert_pem_start");
// extern const uint8_t private_key_pem_start[] asm("_binary_private_key_pem_start");
// extern const uint8_t root_ca_pem_start[]     asm("_binary_root_ca_pem_start");

// esp_mqtt_client_handle_t client = NULL;

// static esp_err_t mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
// {
//     ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
//     esp_mqtt_event_handle_t event = event_data;
//     esp_mqtt_client_handle_t client = event->client;
//     switch (event->event_id)
//     {
//     case MQTT_EVENT_CONNECTED:
//         ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
//         esp_mqtt_client_subscribe(client, "my_topic", 0);
//         esp_mqtt_client_publish(client, "my_topic", "Node 1 alive", 0, 1, 0);
//         break;
//     case MQTT_EVENT_DISCONNECTED:
//         ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
//         break;
//     case MQTT_EVENT_SUBSCRIBED:
//         ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", (int)event->msg_id);
//         break;
//     case MQTT_EVENT_UNSUBSCRIBED:
//         ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", (int)event->msg_id);
//         esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
//         ESP_LOGI(TAG, "sent publish successful, msg_id=%d", (int)event->msg_id);
//         break;
//     case MQTT_EVENT_PUBLISHED:
//         ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", (int)event->msg_id);
//         break;
//     case MQTT_EVENT_DATA:
//         ESP_LOGI(TAG, "MQTT_EVENT_DATA");
//         printf("\nTOPIC=%.*s\r\n", event->topic_len, event->topic);
//         printf("DATA=%.*s\r\n", event->data_len, event->data);
//         break;
//     case MQTT_EVENT_ERROR:
//         ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
//         break;
//     default:
//         ESP_LOGI(TAG, "Other event id:%d", event->event_id);
//         break;
//     }
//     return ESP_OK;
// }

// void mqtt_app_start(void)
// {
//     const esp_mqtt_client_config_t mqtt_cfg = {
//         .broker = {
//             .address.uri = "mqtts://acqql8v2q7crf-ats.iot.us-east-1.amazonaws.com",
//             .verification.certificate = (const char *)root_ca_pem_start         // Root CA
//         },
//         .credentials = {
//             .authentication = {
//                 .certificate = (const char*)device_cert_pem_start,      // Device certificate
//                 .key = (const char *)private_key_pem_start,             // Private Key
//             },
//         },
//     };
//     esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
//     esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
//     esp_mqtt_client_start(client);
// }