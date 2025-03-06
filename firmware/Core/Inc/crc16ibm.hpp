#include "main.h"

static uint8_t crc16lut[256] = { 0 };
static uint8_t crc16ref[256] = { 0 };

void initfastcrc16ibm()
{
    uint16_t remainder;

    for (size_t dividend = 0; dividend < 256; ++dividend)
    {
        remainder = dividend << (16 - 8);
        for (uint8_t bit = 8; bit > 0; --bit)
        {		
            if (remainder & 0x8000) remainder = (remainder << 1) ^ 0x8005;
            else remainder = (remainder << 1);
        }
        crc16lut[dividend] = remainder;
    }
    for(size_t i = 0; i < 256; i++)
    {
        uint8_t data = i;
        uint8_t size = 8;
        uint8_t reflection = 0x00;
        for (uint8_t bit = 0; bit < size; bit++) 
        {
            if (data & 0x01) reflection |= (1 << ((size - 1) - bit));
            data = (data >> 1);
        }
        crc16ref[i] = reflection;
    }
}

uint16_t fastcrc16ibm(uint8_t *data, size_t size)
{
    uint16_t crc = 0xFFFF; // Initial value
    for (size_t i = 0; i < size; ++i) 
    {
        uint8_t tableIndex = (crc >> 8) ^ crc16ref[data[i]];
        crc = (crc << 8) ^ crc16lut[tableIndex];
    }
    crc = crc16ref[(crc >> 8)] << 8 | crc16ref[(crc)];
    return crc;
}

