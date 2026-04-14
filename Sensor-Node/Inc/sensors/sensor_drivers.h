/**
 * @file  sensor_drivers.h
 * @brief 
*/

#ifndef SENSOR_DRIVERS_H
#define SENSOR_DRIVERS_H

#include <stdint.h>

/* ---  Gas sensor - Simulated (No sensor) --- */
typedef enum { 
    GAS_OK = 0, 
    GAS_ERROR = 1 
} Gas_Status_t;

Gas_Status_t gas_sensor_init(void);
Gas_Status_t gas_sensor_read(float *co2);

/* ---  Smoke sensor - Simulated (NO sensor) --- */
typedef enum { 
    SMOKE_OK = 0, 
    SMOKE_ERROR = 1 
} Smoke_Status_t;

Smoke_Status_t smoke_sensor_init(void);
Smoke_Status_t smoke_sensor_read(float *smoke);

/* ---  Flame sensor - Digital GPIO input --- */
typedef enum {
    FLAME_OK    = 1,
    FLAME_ERROR = 0
} Flame_Status_t;

typedef enum {
    FLAME_NOT_DETECTED = 0,
    FLAME_DETECTED     = 1
} Flame_State_t;

/* ---  TMP102 Temp Sensor --- */
typedef enum {
    TMP102_OK    = 0,
    TMP102_ERROR = 1
} TMP102_Status_t;

TMP102_Status_t tmp102_init(void);
TMP102_Status_t tmp102_read(float *temp);

/* ---  BME680 Environment Sensor --- */




#endif      // SENSOR_DRIVERS_H