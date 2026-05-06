/**
 * @file  sensor_drivers.h
 * @brief 
*/

#ifndef SENSOR_DRIVERS_H
#define SENSOR_DRIVERS_H

#include <stdint.h>

/* ---  BME680 Environment Sensor --- */
typedef enum {
    BME680_OK    = 0,
    BME680_ERROR = 1
} BME680_Status_t;

BME680_Status_t bme680_init(void);
BME680_Status_t bme680_read(float *temp, float *humi, float *pres, float *voc);

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

#endif      // SENSOR_DRIVERS_H