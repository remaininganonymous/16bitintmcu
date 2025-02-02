#include <stdio.h>

#define SIZE_ARR      4
#define SIGN_BIT_SIZE 1
#define EXPONENT_SIZE 11
#define MANTISSA_SIZE 52

void convert_double_to_arr(double in, short* out)
{
    unsigned long long input;
    union
    {
        double d;
        unsigned long long ull;
    }
    data;

    data.d = in;
    input = data.ull;

    out = (short*)malloc(SIZE_ARR * sizeof(short));
    for (short i = 0; i < SIZE_ARR; i++)
    {
        out[i] = (short)((input >> i*16) & 0xFFFF);
    }
}

double convert_arr_to_double(short* in)
{
    double result;
    unsigned long long 
    union
    {
        double d;
        unsigned long long ull;
    }
    data;

    // data.ull = (unsigned long long) in & 0xFFFF;

    

    result = data.d;

    return result;
}

unsigned short* add(short* a, short* b)
{

}

short* multiply(short* a, short* b)
{
    
}

short* divide(short* a, short* b)
{
    return 0x0;
}

int main()
{
    double a = 3.14;
    double b = 2.71;
    
    short* a_converted;
    convert_double_to_arr(a, a_converted);
    short* b_converted;
    convert_double_to_arr(b, b_converted);

    short* addition_result = add(a_converted, b_converted);
    short* multiplication_result = multiply(a_converted, b_converted);
    short* division_result = divide(a_converted, b_converted);

    double addition_result_double = convert_arr_to_double(addition_result);
    double multiplication_result_double = convert_arr_to_double(multiplication_result);
    double division_result_double = convert_arr_to_double(division_result);

    printf("%f", addition_result_double);
    printf("%f", multiplication_result_double);
    printf("%f", division_result_double);

    return 0;
}