// /**
//  * @file  flame_sensor.c
//  * @brief Flame sensor driver
//  * 
//  * Digital GPIO input
//  * 
//  * Current implementation: Simulated values - no physical sensor
//  * In production: replace body of flame_sensor_read() with real
//  * GPIO_ReadPin() call on configured input pin.
// */

// #include <stddef.h>
// #include <stdint.h>
// #include "sensors.h"

// Flame_Status_t flame_sensor_init(void) 
// {
//     // In production: Configure GPIO pin as input with pull-up

//     return FLAME_OK;
// }

// Flame_Status_t flame_sensor_read(uint8_t *state) 
// {
//     if (state == NULL) return FLAME_ERROR;

//     // Simulated: Normal operation - no flame detected
//     *state = FLAME_NOT_DETECTED;

//     return FLAME_OK;
// }