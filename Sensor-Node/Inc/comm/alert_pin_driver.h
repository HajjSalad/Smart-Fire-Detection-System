#ifndef ALERT_PIN_DRIVER_H_
#define ALERT_PIN_DRIVER_H_

/**
 * @file  alert_pin_driver.h
 * @brief Public API for GPIO output peripheral.
*/

#include <stdio.h>
#include <stdint.h>

// Function Prototyoes
void alert_pin_init(void);
void alert_pin_set(void);
void alert_pin_clear(void);

#endif  // ALERT_PIN_DRIVER_H_