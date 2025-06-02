/**
 * @file cloud.h
 * @brief MQTT Cloud Connectivity Header
 *
 * Provides functions for initializing and interacting with an MQTT client over TLS
 * for cloud-based IoT applications.
 */

#ifndef CLOUD_H
#define CLOUD_H

#include "mqtt_client.h"

/**
 * @brief Initializes and starts the MQTT client.
 *
 * This function sets up the MQTT client using TLS with a specified broker,
 * client certificate, and private key. It registers the event handler and
 * initiates the connection to the broker.
 */
void mqtt_app_start(void);

/**
 * @brief Retrieves the current MQTT client handle.
 *
 * @return esp_mqtt_client_handle_t The MQTT client instance.
 */
esp_mqtt_client_handle_t get_mqtt_client(void);

#endif // CLOUD_H