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

    uint16_t p1 = a_L * b_L;  // младшие 16 бит
    uint16_t p2 = a_H * b_L;  // перенос в старшие биты
    uint16_t p3 = a_L * b_H;  // перенос в старшие биты
    uint16_t p4 = a_H * b_H;  // самые старшие 16 бит (переполнение)

    uint16_t mid = (p1 >> 8) + (p2 & 0xFF) + (p3 & 0xFF);
    *low = (p1 & 0xFF) | ((mid & 0xFF) << 8);
    *high = (mid >> 8) + (p2 >> 8) + (p3 >> 8) + p4;
}

void add16(uint16_t a, uint16_t b, uint16_t *sum, uint16_t *carry) {
    *sum = a + b;                  // Складываем числа
    *carry = (*sum < a) ? 1 : 0;    // Если произошло переполнение, устанавливаем carry
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


// MREM:
//                     a260 64c0 1bc0 cba0
//                a554 06a8 d8c8 deec
//           69e7 98ce aca6 e5c9
//      802d 375a 63a2 9063
//       A142 A1C5 E613 2351 faac cba0
// 802d a142 a1c5 e613 2351 faac cba0
int main ()
{
    uint16_t a[SIZE_ARR] = {0x86e0, 0x5444, 0x21fb, 0x4009}; // 4009 21fb 5444 86e0
    uint16_t b[SIZE_ARR] = {0x500b, 0x8b12, 0xbf0a, 0x4005}; // 4005 bf0a 8b12 500b
    
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

    
    
    
    // unsigned long long res = 0;
    // for (int i = 0; i < SIZE_ARR*2; i++)
    // {
    //     res += sm_rem[i] << (i*);
    // }
    // printf("\n%llx\n", res);

    
}