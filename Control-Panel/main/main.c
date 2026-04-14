/**
 * @file main.c
 * @brief
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_log.h"
#include "string.h"
#include "stdio.h"

#include "modbus_master.h"

static const char *TAG = "MAIN";

void app_main()
{
    ESP_LOGI(TAG, "Startup...");
    ESP_LOGI(TAG, "Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    /** 
     *  System-wide Initialization
     *    nvs_flash_init() -> Non-Volatile storage to persist data across reboots
     *    esp_netif_init() -> Create an LwIP core task and initialize LwIP-related work
     *    esp_event_loop_create_default() -> Create a system Event task and 
     *               initialize an application event's callback function
     * 
     *   These are used by: Wi-Fi driver, MQTT driver, 
    */
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /**
     * Priorities  (in FreeRTOS, 0 = lowest priority)
     * 
     * Priority 4  → Modbus Master
     * Priority 0  → Idle task
    */

    // Create tasks
    modbus_master_task_init();             // Priority: 4
}
