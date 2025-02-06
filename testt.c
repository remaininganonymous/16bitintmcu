#include <stdio.h>

typedef  unsigned short       uint16_t;

uint16_t pow2(uint16_t x)
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
    printf("%d", pow2(10));
}