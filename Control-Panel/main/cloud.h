#ifndef CLOUD_H
#define CLOUD_H

#include "mqtt_client.h"

void mqtt_app_start(void);
esp_mqtt_client_handle_t get_mqtt_client(void);

#endif // CLOUD_H