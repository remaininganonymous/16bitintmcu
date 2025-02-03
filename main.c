#include <stdio.h>

#define SIZE_ARR      4
#define SIGN_BIT_SIZE 1
#define EXPONENT_SIZE 11
#define MANTISSA_SIZE 52

typedef unsigned long long uint64_t;
typedef union
{
    double d;
    uint64_t ull; 
}
union_t;

void convert_double_to_arr(double in, unsigned short* out)
{
    union_t data;
    data.d = in;
    uint64_t input = data.ull;

    printf("***\n");
    for (int i = 0; i < SIZE_ARR; i++)
    {
        out[i] = (unsigned short)((input >> (i*16)) & 0xFFFF);
        printf("%x ", out[i]);
    }
    printf("input = %llx\n", input);
    printf("\n***\n");
}

double convert_arr_to_double(unsigned short* in)
{
    double result;
    union_t data;
    uint64_t input = 0;

    printf("---\n");
    for (int i = 0; i < SIZE_ARR; i++)
    {
        input |= ((uint64_t)(in[i]) << (i * 16));
    }
    printf("%llx ", input);
    printf("\n---\n");
    data.ull = input;
    printf("\n%llu %.15f\n", data.ull, data.d);
    result = data.d;
    
    return result;
}

unsigned short* add(short* a, short* b)
{

}

// short* multiply(short* a, short* b)
// {
    
// }

// short* divide(short* a, short* b)
// {
//     return 0x0;
// }

void print_array(unsigned short* in)
{
    for (int i = 0; i < SIZE_ARR; i++)
    {
        printf("%llx ", in[i]);
    }
    printf("\n");
}

int main()
{
    double a = 3.141592653589793;
    double b = 2.718281828459045;
    
    unsigned short a_converted[SIZE_ARR];
    convert_double_to_arr(a, a_converted);
    short b_converted[SIZE_ARR];
    convert_double_to_arr(b, b_converted);

    printf("short arrays:\n");
    print_array(a_converted);
    print_array(b_converted);

    double _a = convert_arr_to_double(a_converted);
    double _b = convert_arr_to_double(b_converted);

    printf("back to double:\n");
    printf("%.15f %.15f", _a, _b);
    // short* addition_result = add(a_converted, b_converted);
    // short* multiplication_result = multiply(a_converted, b_converted);
    // short* division_result = divide(a_converted,s b_converted);

    // double addition_result_double = convert_arr_to_double(addition_result);
    // double multiplication_result_double = convert_arr_to_double(multiplication_result);
    // double division_result_double = convert_arr_to_double(division_result);

    // printf("%f\n", addition_result_double);
    // printf("%f\n", multiplication_result_double);
    // printf("%f\n", division_result_double);

    return 0;
}