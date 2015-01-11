#include "analog_companding.h"
static float value,a,b;
static int8_t sign;


uint8_t u_compress(int16_t sample, uint8_t bitLen, float uValue)
{
    /* convert data to -1 - 1 range */
    value = ((float)sample) / (( 1 << (bitLen - 1) )-1) ;
    if(value < - 1) value = -1;
    sign = 1;
    if(value < 0)
    {
        value = -value;
        sign = -1;
    }

    value = log(1.0+(uValue*value)) / log(1.0+uValue);

    if(sign == -1) return ((uint8_t)(value*127))+128;
    return ((uint8_t)(value*127));
}

int16_t u_expand(uint8_t data, uint8_t bitLen, float uValue)
{
    sign = 1;
    if(data & 128) sign = -1;
    value = data & 127;

    value /= 127.0;

    value = ( pow(uValue+1.0,value) - 1.0)/uValue;

    /* return signed value of desired bit length */
    return ((int16_t) ( value*sign*(( 1 << (bitLen - 1) )-1 ) ) );
}

uint8_t A_compress(int16_t sample, uint8_t bitLen, float AValue)
{
    /* convert data to -1 - 1 range */
    value = ((float)sample) / (( 1 << (bitLen - 1) )-1) ;
    if(value < - 1) value = -1;
    sign = 1;
    if(value < 0)
    {
        value = -value;
        sign = -1;
    }

    if(value <= 1.0/AValue)
    {
        value = (AValue*value) / (1.0 + log(AValue) );
    }
    else
    {
        value = (1.0 + log(AValue*value) ) / (1.0 + log(AValue) );
    }

    if(sign == -1) return ((uint8_t)(value*127))+128;
    return ((uint8_t)(value*127));
}

int16_t A_expand(uint8_t data, uint8_t bitLen, float AValue)
{
    sign = 1;
    if(data & 128) sign = -1;
    value = data & 127;

    value /= 127.0;

    if(value < (1.0 / (1.0 + log(AValue) )) )
    {
        value = value * (1.0 + log(AValue) ) / AValue;
    }
    else
    {
        value = exp( (value * (1.0 + log(AValue) )) - 1) / AValue;
    }

    /* return signed value of desired bit length */
    return ((int16_t) ( value*sign*(( 1 << (bitLen - 1) )-1 ) ) );
}

uint8_t seg13_compress(int16_t sample, uint8_t bitLen)
{
    /* convert data to -1 - 1 range */
    value = ((float)sample) / (( 1 << (bitLen - 1) )-1) ;
    if(value < - 1) value = -1;
    sign = 1;
    if(value < 0)
    {
        value = -value;
        sign = -1;
    }

    if(value <= 1.0/64)
    {
       a = 16.0; b=0.0;
    }
    else if(value <= 1.0/32)
    {
       a = 8.0; b = 0.125;
    }
    else if(value <= 1.0/16)
    {
        a = 4.0; b = 0.25;
    }
    else if(value < 1.0/8)
    {
        a = 2.0; b = 0.375;
    }
    else if(value < 1.0/4)
    {
        a = 1.0; b = 0.5;
    }
    else if(value < 1.0/2)
    {
        a = 0.5; b = 0.625;
    }
    else
    {
        a = 0.25; b = 0.75;
    }

    value = a*value+b;
    if(sign == -1) return ((uint8_t)(value*127))+128;
    return ((uint8_t)(value*127));

}

int16_t seg13_expand(uint8_t data, uint8_t bitLen)
{
    sign = 1;
    if(data & 128) sign = -1;
    value = data & 127;

    value /= 127.0;

    if(value <= 2.0/8)
    {
       a = 16.0; b=0.0;
    }
    else if(value <= 3.0/8)
    {
       a = 8.0; b = 0.125;
    }
    else if(value <= 4.0/8)
    {
        a = 4.0; b = 0.25;
    }
    else if(value < 5.0/8)
    {
        a = 2.0; b = 0.375;
    }
    else if(value < 6.0/8)
    {
        a = 1.0; b = 0.5;
    }
    else if(value < 7.0/8)
    {
        a = 0.5; b = 0.625;
    }
    else
    {
        a = 0.25; b = 0.75;
    }


    value = (value - b)/a;

    /* return signed value of desired bit length */
    return ((int16_t) ( value*sign*(( 1 << (bitLen - 1) )-1 ) ) );
}
