#include <stdio.h>

#define  PROCESSOR_BIT_DEPTH  16
#define  SIZE_ARR             4
#define  SIGN_BIT_SIZE        1
#define  EXPONENT_SIZE        11
#define  MANTISSA_SIZE        52

typedef  unsigned long long   uint64_t;
typedef  unsigned long        uint32_t;
typedef  unsigned short       uint16_t;
typedef  union
{
    double    d;
    uint64_t  ull; 
}
double_ull_t;
typedef  struct
{
    uint16_t  sign;
    uint16_t  exponent;
    uint16_t  mantissa[SIZE_ARR]; 
}
double_repr;

void print_array(uint16_t* in)
{
    //for (int i = 0; i < SIZE_ARR; i++)
    for (int i = SIZE_ARR-1; i >= 0; i--)
    {
        printf("%llx ", in[i]);
    }
    printf("\n");
}

void convert_double_to_arr(double in, uint16_t* out)
{
    double_ull_t data;
    data.d = in;
    uint64_t input = data.ull;

    for (int i = 0; i < SIZE_ARR; i++)
    {
        out[i] = (uint16_t)((input >> (i*16)) & 0xFFFF);
    }
}

double convert_arr_to_double(uint16_t* in)
{
    double result;
    double_ull_t data;
    uint64_t input = 0;

    for (int i = 0; i < SIZE_ARR; i++)
    {
        input |= ((uint64_t)(in[i]) << (i * 16));
    }

    data.ull = input;
    result = data.d;
    
    return result;
}

double_repr arr_to_double_repr(uint16_t* in) //можно ускорить через union?
{
    double_repr num;

    num.sign = (in[SIZE_ARR-1] >> 15) & 0x0001;
    num.exponent = (in[SIZE_ARR-1] >> 4) & 0x7FF;
    num.mantissa[SIZE_ARR-1] = in[SIZE_ARR-1] & 0x000F;
    for (int i = SIZE_ARR-2; i >= 0; i--)
    {
        num.mantissa[i] = in[i];
    }

    return num;
}

void double_repr_to_arr(double_repr* in, uint16_t* out)
{
    out[SIZE_ARR-1] = in->sign << 15;
    out[SIZE_ARR-1] |= in->exponent << 4;
    out[SIZE_ARR-1] |= in->mantissa[SIZE_ARR-1];
    for (int i = SIZE_ARR-2; i >= 0; i--)
    {
        out[i] = in->mantissa[i];
    }
    printf("drta_out: ");
    print_array(out);
}

void shift_mantissa_bits(double_repr* in, uint16_t exp_diff)
{
    printf("before shift: ");
    print_array(in->mantissa);
    /* разность экспонент может быть большой, но благо мы можем
       позволить себе сдвинуть максимум на 52 бит влево */
    uint16_t shift = (exp_diff <= 52) ? exp_diff : 52;
    // увеличение экспоненты
    in->exponent += shift; //?
    /* два случая: сдвиг на менее чем 16 бит и сдвиг на более чем 16 бит.
       в первом случае двигаем только побитово, а во втором сдвигаем
       и целые блоки по 16 бит */

    // 402a 8000 0022 1111
    uint16_t shift_int = shift / PROCESSOR_BIT_DEPTH;
    uint16_t shift_fract = shift % PROCESSOR_BIT_DEPTH;

    if (shift_fract != 0)
    {
        uint16_t buff = 0;
        for (int i = 0; i < SIZE_ARR; i++)
        {
            if (i+1 < SIZE_ARR) buff = in->mantissa[i+1] << (PROCESSOR_BIT_DEPTH-shift_fract);
            else buff = 0x002;
            in->mantissa[i] = (in->mantissa[i] >> shift_fract) | buff;
        }
    }
    if (shift_int != 0)
    { 
        in->mantissa[0] = in->mantissa[0+shift_int];
        in->mantissa[1] = (1+shift_int < SIZE_ARR) ? in->mantissa[1+shift_int] : 0x0000;
        in->mantissa[2] = (2+shift_int < SIZE_ARR) ? in->mantissa[2+shift_int] : 0x0000;
        in->mantissa[3] = 0x0000;
    }
    

    //после сдвига ищем ячейку, в которую "допишем" неявную единицу
    // int target_index = -1;
    // for (int i = SIZE_ARR-2; i >= 0 && target_index == -1; i--)
    // {
    //     if (in->mantissa[i+1] == 0 && in->mantissa[i] != 0)
    //     {
    //         if (in->mantissa[i] < 0x8000) target_index = i;
    //         else target_index = i+1;
    //     }
    // }
    // if (target_index == -1) target_index = SIZE_ARR-1;
    // printf("TARGET INDEX: %d\n", target_index);
    
    // uint16_t implicit_bit = 1 << ;
    // in->mantissa[target_index] |= implicit_bit;
    // 1000000000000000
    // 0000000011001000
    // 0000 0000 A000 0022
    // 000F a402 8000 0022
    
    printf("after shift: ");
    print_array(in->mantissa);
}

void normalize_double(double_repr* in)
{
    in->sign = 0;
    in->exponent = 0b10000000010; ////
}

void add(uint16_t* a, uint16_t* b, uint16_t* out) //
{
    double_repr a_repr = arr_to_double_repr(a);
    double_repr b_repr = arr_to_double_repr(b);
    double_repr sum = {0, 0, 0};

    printf("exp_a = %d\n", a_repr.exponent);
    printf("exp_b = %d\n", b_repr.exponent);

    uint16_t least;
    if (a_repr.exponent < b_repr.exponent)
    {
        //least = a_repr.exponent;
        uint16_t exponent_diff = b_repr.exponent - a_repr.exponent;
        if (exponent_diff > 0) shift_mantissa_bits(&a_repr, exponent_diff);
    }
    else
    {
        //least = b_repr.exponent;
        uint16_t exponent_diff = a_repr.exponent - b_repr.exponent;
        if (exponent_diff > 0) shift_mantissa_bits(&b_repr, exponent_diff);
    }

    uint16_t excess = 0;
    for (int i = 0; i < SIZE_ARR; i++)
    {
        least = (a_repr.mantissa[i] < b_repr.mantissa[i]) ? a_repr.mantissa[i] : b_repr.mantissa[i];
        sum.mantissa[i] = a_repr.mantissa[i] + b_repr.mantissa[i] + excess;
        excess = (sum.mantissa[i] < least) ? 1 : 0;
        printf ("%llx < %llx\n", sum.mantissa[i], least);
    }

    normalize_double(&sum);
    double_repr_to_arr(&sum, out);
}

// 0 10000000010 1000000000000000000000000000000000000000000000000000 - 4028 0    0 0
//               1000000000000000000000000000000000000000000000000000 - 0008 0    0 0
// 0 01111111111 0100000000000000000000000000000000000000000000000000 - 3ff4 0    0 0 +
//               0010100000000000000000000000000000000000000000000000 - 0000 8    0 0
// 0 10000000010 1010100000000000000000000000000000000000000000000000 - 402a 8000 0 0

int main()
{
    // double a = 3.141592653589793;
    // double b = 2.718281828459045;
    // double a = 23.235257;
    // double b = 33.235257;
    double a = 12;
    double b = 1.25;
    double c = a + b;
    // double_ull_t c;
    // c.d = a + b;
    

    printf("======= EXPECTED: =======\n");
    printf("A + B = %.15f\n", a + b);
    printf("A * B = %.15f\n", a * b);
    // printf("A / B = %.15f\n", a / b);
    printf("=========================\n");
    // printf("A = %llx\n", a);
    // printf("B = %llx\n", b);
    // printf("C = %llx\n", c);


    uint16_t a_converted[SIZE_ARR];
    convert_double_to_arr(a, a_converted);
    uint16_t b_converted[SIZE_ARR];
    convert_double_to_arr(b, b_converted);
    uint16_t c_converted[SIZE_ARR];
    convert_double_to_arr(c, c_converted);

    uint16_t c_array[SIZE_ARR];
    add(a_converted, b_converted, c_array);

    printf("short arrays:\n");
    print_array(a_converted);
    print_array(b_converted);
    print_array(c_converted);
    print_array(c_array);

    double _a = convert_arr_to_double(a_converted);
    double _b = convert_arr_to_double(b_converted);
    double _c = convert_arr_to_double(c_array);

    printf("back to double:\n");
    printf("%.15f %.15f %.15f", _a, _b, _c);

    return 0;
}