/**
 * @file  crc_16.c
 * @brief CRC-16 computation for UART packet integrity validation.
*/

#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Compute CRC-16 checksum over a byte buffer.
 * 
 * Polynomial : 0x8005
 * 
 * @param  pData     Pointer to the data buffer.
 * @param  numBytes  Number of bytes to process.
 * @return           16-bit CRC checksum.
 * 
 * Adapted from Reddit: 
 * https://www.reddit.com/r/embedded/comments/1acoobg/crc16_again_with_a_little_gift_for_you_all/ 
*/
uint16_t compute_crc(const uint8_t * pData, size_t numBytes) 
{
	uint32_t crc = 0;

	for (size_t i=0; i<numBytes; i++)
	{
		uint8_t  d = *(pData++);
		uint32_t x = ((crc ^ d) & 0xff) << 8;
		uint32_t y = x;

		x ^= x << 1;
		x ^= x << 2;
		x ^= x << 4;
		
		x  = (x & 0x8000) | (y >> 1);

		crc = (crc >> 8) ^ (x >> 15) ^ (x >> 1) ^ x;
	}
	return crc;
}