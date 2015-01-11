#ifndef ANALOG_COMPANDING_H
#define ANALOG_COMPANDING_H
#include "math.h"
#include "stm32f4xx.h"

uint8_t A_compress(int16_t sample, uint8_t bitLen, float AValue);
uint8_t u_compress(int16_t sample, uint8_t bitLen, float uValue);
int16_t A_expand(uint8_t data, uint8_t bitLen, float AValue);
int16_t u_expand(uint8_t data, uint8_t bitLen, float uValue);
uint8_t seg13_compress(int16_t sample, uint8_t bitLen);
int16_t seg13_expand(uint8_t data, uint8_t bitLen);

#endif //ANALOG_COMPANDING_H
