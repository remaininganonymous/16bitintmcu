#include <stdio.h>

typedef  unsigned short       uint16_t;
typedef short int16_t;
    
uint16_t power2(uint16_t x)
{
    uint16_t result = 1;
    for (int i = 0; i < x; i++)
    {
        result *= 2;
    }
    return result;
}

int main ()
{
    int16_t mask = 1;
    // uint16_t pow = -1;

    while (mask - 0xD < 0)
    {
        // pow++;
        mask *=2;
    }

    //mask = power2(pow);
    printf("%d", mask);
}