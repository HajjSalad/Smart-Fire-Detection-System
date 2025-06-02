/**
 * @file wifi.c
 * @brief Wi-Fi initialization and connection for ESP32.
 *
 * This file handles the initialization, configuration, and connection 
 * of the ESP32 device to a Wi-Fi network in station (STA) mode.
 */

#include "wifi.h"

#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "esp_eth.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_check.h"
#include "esp_netif.h"

#include "esp_event.h"
#include "sdkconfig.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

/**
 * @brief Event handler for Wi-Fi and IP events.
 *
 * Logs Wi-Fi connection status and IP acquisition.
 *
 * @param event_handler_arg Unused parameter.
 * @param event_base Event base type (Wi-Fi or IP).
 * @param event_id Specific event ID.
 * @param event_data Event-specific data.
 */
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

/**
 * @brief Connect to a Wi-Fi network using station (STA) mode.
 *
 * Initializes network interfaces, registers event handlers, configures Wi-Fi settings,
 * and connects to the Wi-Fi network using predefined SSID and password.
 */
void wifi_connection() {
    // 1 - Init Phase
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 2 - Configuration Phase
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASS,
        },
    };
    
    // 3 - Start Phase
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_ps(WIFI_PS_NONE); // Disable power save for better connectivity

    // 4 - Connect Phase
    ESP_ERROR_CHECK(esp_wifi_connect());
}
