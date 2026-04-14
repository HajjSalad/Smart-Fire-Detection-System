/**
 * @file  gas_sensor.c
 * @brief CO2 gas sensor driver
 * 
 * Current implementatio - simulated
 * 
 * Typical CO2 levels:
 *  400  ppm - clean outdoor air
 *  1000 ppm - indoor ventilation threshold
 *  5000 ppm - safety limit (OSHA)
 *  Fire: can exceed 10000+ ppm
 * 
 * Source: CO2METER:
 *  https://www.co2meter.com/en-ca/blogs/news/carbon-dioxide-indoor-levels-chart?srsltid=AfmBOooCOI0mX7qUUcS6hQupJursqoDPJ0UAVkBZ9bVmj4py7abodbUg
*/

#include <stdlib.h>
#include <stddef.h>
#include "sensor_drivers.h"

#define GAS_SENSOR_MIN      (400U)
#define GAS_SENSOR_MAX      (5000U)

Gas_Status_t gas_sensor_init(void) 
{
    // In production: wake sensor, trigger continous measurement mode

    return GAS_OK;
}

Gas_Status_t gas_sensor_read(float *co2)
{
    if (co2 == NULL) return GAS_ERROR;

    // Simulate sensor reading 400-5000 ppm
    *co2 = GAS_SENSOR_MIN + (float)(rand() % (GAS_SENSOR_MAX - GAS_SENSOR_MIN));

    return GAS_OK;
}