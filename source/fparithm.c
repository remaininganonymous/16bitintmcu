#include "../include/fparithm.h"

uint16_t power2(uint16_t x)
{
    uint16_t result = 1;
    for (int i = 0; i < x; i++)
    {
        result *= 2;
    }

    return result;
}

