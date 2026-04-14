/**
 * @file  modbus_register.h
 * @brief MODBUS RTU register map for ESP32 sensor node
 * 
 * All sensor values are stored as uint16_t holding registers.
 * Floating point values are scaled to preserve decimal precision.
 * 
 * Register Map:
 * ┌──────┬─────────────┬────────┬────────────────────────────┐
 * │ Addr │ Sensor      │ Unit   │ Scale                      │
 * ├──────┼─────────────┼────────┼────────────────────────────┤
 * │ 0x00 │ Temperature │ °C     │ × 100  (2534 = 25.34°C)    │
 * │ 0x01 │ Humidity    │ %RH    │ × 100  (5210 = 52.10%)     │
 * │ 0x02 │ Pressure    │ hPa    │ × 10   (10132 = 1013.2hPa) │
 * │ 0x03 │ VOC         │ Ω      │ × 1    (raw resistance)    │
 * │ 0x04 │ CO2         │ ppm    │ × 1    (412 = 412ppm)      │
 * │ 0x05 │ PM2.5       │ µg/m³  │ × 10   (85 = 8.5µg/m³)     │
 * │ 0x06 │ Flame       │ 0/1    │ × 1    (0=none, 1=detect)  │
 * └──────┴─────────────┴────────┴────────────────────────────┘
*/

#ifndef MODBUS_REGISTERS_H
#define MODBUS_REGISTERS_H

#include <stdint.h>

/* ── MODBUS node identity ── */
#define MODBUS_SLAVE_ADDR           0x01U   // this node address on bus
#define MODBUS_BROADCAST_ADDR       0x00U   // broadcast — all slaves

/* ── Supported function codes ── */
#define MODBUS_FC_READ_HOLDING      0x03U   // read holding registers    
#define MODBUS_FC_WRITE_SINGLE      0x06U   // write single register 

/* ── Exception codes ── */
#define MODBUS_EX_ILLEGAL_FUNCTION  0x01U   // unsupported function code 
#define MODBUS_EX_ILLEGAL_ADDRESS   0x02U   // register addr out of range
#define MODBUS_EX_ILLEGAL_VALUE     0x03U   // invalid register value    
#define MODBUS_EX_SERVER_FAILURE    0x04U   // internal slave error      

/* ── Register addresses ── */
#define REG_TEMPERATURE             0x0000U
#define REG_HUMIDITY                0x0001U
#define REG_PRESSURE                0x0002U
#define REG_VOC                     0x0003U
#define REG_CO2                     0x0004U
#define REG_PM25                    0x0005U
#define REG_FLAME                   0x0006U
#define MODBUS_REG_COUNT            7U       // total number of registers
#define MODBUS_START_ADDR           0x0000U

/* Scaling factors */
#define SCALE_TEMP              100.0f
#define SCALE_HUMI              100.0f
#define SCALE_PRES              10.0f
#define SCALE_VOC               1.0f
#define SCALE_CO2               1.0f
#define SCALE_PM25              10.0f

/* ── Helper macro: float to scaled uint16_t ── */
#define FLOAT_TO_REG(val, scale)    ((uint16_t)((val) * (scale)))

#define SLAVE_ADDR_POS              0
#define FC_ADDR_POS                 1

#endif