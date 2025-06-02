/**
 * @file wifi.h
 * @brief Header file for Wi-Fi connection setup.
 *
 * This file contains the definitions and function declarations for
 * setting up Wi-Fi in station mode.
 */

#ifndef WIFI_H
#define WIFI_H

/// Wi-Fi SSID to connect to
#define SSID "RT-AC1200_68_2G"
/// Wi-Fi password for the defined SSID
#define PASS "museum_5669"

/**
 * @brief Establish a Wi-Fi connection using STA mode.
 *
 * Connects the ESP32 to a Wi-Fi network using credentials defined in macros.
 */
void wifi_connection();

#endif // WIFI_H