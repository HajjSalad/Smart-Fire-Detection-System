/**
 * @file  smoke_sensor.c
 * @brief Particulate matter sensor driver
 *
 * Current implementation: Simulated 
 *
 * PM2.5 reference levels (µg/m³):
 *   0   - 12  → Good
 *   12  - 35  → Moderate
 *   35  - 150 → Unhealthy
 *   150 - 500 → Hazardous / fire smoke
 * 
 * Resource - Air Resource Board:
 * https://ww2.arb.ca.gov/resources/inhalable-particulate-matter-and-health
*/

#include <stddef.h>
#include <stdlib.h>
#include "sensor_drivers.h"

#define SMOKE_SENSOR_MIN    (0U)
#define SMOKE_SENSOR_MAX    (500U)

Smoke_Status_t smoke_sensor_init(void)
{
    // In production: power on sensor

    return SMOKE_OK;
}

Smoke_Status_t smoke_sensor_read(float *pm25)
{
    if (pm25 == NULL) return SMOKE_ERROR;

    // Simulated: PM2.5 range 0-500 µg/m³
    *pm25 = SMOKE_SENSOR_MIN + (float)(rand() % (SMOKE_SENSOR_MAX - SMOKE_SENSOR_MIN));

    return SMOKE_OK;
}