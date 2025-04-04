#ifndef __CRC_H__
#define __CRC_H__

#include <stdint.h>
#include <stdio.h>

#define CRC_POLYNOMIAL 0x07  // CCITT crc8 

void crc_init(void);
uint8_t crc_calc(uint8_t *input, ssize_t length);
uint8_t crc_calc_comm(uint8_t *input, ssize_t length);
#endif // __CRC_H__
