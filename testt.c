#include <stdio.h>

typedef  unsigned short       uint16_t;
typedef short int16_t;
#define SIZE_ARR 4

typedef  struct
{
    uint16_t  sign;
    uint16_t  exponent;
    uint16_t  mantissa[SIZE_ARR]; 
}
double_repr;

void multiply16(uint16_t a, uint16_t b, uint16_t *low, uint16_t *high)
{
    uint16_t a_L = a & 0xFF;
    uint16_t a_H = (a >> 8) & 0xFF;
    uint16_t b_L = b & 0xFF;
    uint16_t b_H = (b >> 8) & 0xFF;

    uint16_t p1 = a_L * b_L;
    uint16_t p2 = a_H * b_L;
    uint16_t p3 = a_L * b_H;
    uint16_t p4 = a_H * b_H;

    uint16_t mid = (p1 >> 8) + (p2 & 0xFF) + (p3 & 0xFF);
    *low = (p1 & 0xFF) | ((mid & 0xFF) << 8);
    *high = (mid >> 8) + (p2 >> 8) + (p3 >> 8) + p4;
}

void add16(uint16_t a, uint16_t b, uint16_t *sum, uint16_t *carry) {
    *sum = a + b;   
    uint16_t least = (a < b) ? a : b;
    *carry = (*sum < least) ? 1 : 0;
}

void add_normalised_mantissas(uint16_t* a, uint16_t* b, uint16_t* out)
{
    for (int i = 0; i < SIZE_ARR; i++)
    {
        uint16_t excess = 0;
        uint16_t sum = excess;
        add16(a[i], b[i], &sum, &excess);
        out[i] = sum;
        out[i+1] += excess;
    }
}

void add_normalised_mantissas_sm(uint16_t* a, uint16_t* b, uint16_t* out)
{
    uint16_t excess = 0;
    uint16_t sum;
    for (int i = 0; i < SIZE_ARR*2; i++)
    {
        out[i] += excess;
        add16(a[i], b[i], &sum, &excess);
        out[i] += sum; 
    }
}

int main ()
{
    uint16_t a[SIZE_ARR] = {0x86e0, 0x5444, 0x21fb, 0x19}; // 9 21fb 5444 86e0
    uint16_t b[SIZE_ARR] = {0x500b, 0x8b12, 0xbf0a, 0x15}; // 5 bf0a 8b12 500b
    
    uint16_t m_rem[SIZE_ARR][SIZE_ARR];
    uint16_t m_exc[SIZE_ARR][SIZE_ARR];

    for (int i = 0; i < SIZE_ARR; i++)
    {
        for (int j = 0; j < SIZE_ARR; j++)
        {
            multiply16(a[i], b[j], &m_rem[j][i], &m_exc[j][i]);
            printf("%04hx * %04hx = %04hx %04hx (%x)\n", a[i], b[j], m_exc[j][i], m_rem[j][i], a[i]*b[j]);
        }
    }

    uint16_t sm_rem[SIZE_ARR*2] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t sm_exc[SIZE_ARR*2] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t sm[SIZE_ARR*2] = {0, 0, 0, 0, 0, 0, 0, 0};
    printf("\nMREM:\n");
    for (int i = 0; i < SIZE_ARR; i++)
    {
        for (int j = i; j < SIZE_ARR; j++)
        {
            printf("     ");
        }
        for (int j = SIZE_ARR-1; j >= 0; j--)
        {
            printf("%04x ", m_rem[j][i]);
        }
        printf("\n");
    }

    for (int i = 0; i < SIZE_ARR; i++)
    {
        add_normalised_mantissas(m_rem[i], sm_rem+i, sm_rem+i);
    }

    printf("\n");
    for (int i = SIZE_ARR*2-1; i >= 0; i--)
    {
        printf("%04x ", sm_rem[i]);
    }

    printf("\n\nMEXC:\n");
    for (int i = 0; i < SIZE_ARR; i++)
    {
        for (int j = i; j < SIZE_ARR; j++)
        {
            printf("     ");
        }
        for (int j = SIZE_ARR-1; j >= 0; j--)
        {
            printf("%04x ", m_exc[i][j]);
        }
        printf("\n");
    }


    for (int i = 0; i < SIZE_ARR; i++)
    {
        add_normalised_mantissas(m_exc[i], sm_exc+1+i, sm_exc+1+i);
    }

    printf("\n");
    for (int i = SIZE_ARR*2-1; i >= 0; i--)
    {
        printf("%04x ", sm_exc[i]);
    }

    add_normalised_mantissas_sm(sm_exc, sm_rem, sm);
    printf("\n");
    printf("\n1003 b875 f279 28dc 831e 86ef 24d7 cba0\n");
    for (int i = SIZE_ARR*2-1; i >= 0; i--)
    {
        printf("%04x ", sm[i]);
    }
    
    
}