#ifndef CRC_16_H_
#define CRC_16_H_

#include <stdint.h>
#include <stddef.h>

uint16_t compute_crc(const uint8_t *pData, size_t numBytes);

#endif  // CRC_16_H_