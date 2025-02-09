#include <stdio.h>

#define  PROCESSOR_BIT_DEPTH     16
#define  SIZE_ARR                4
#define  SIGN_BIT_SIZE           1
#define  EXPONENT_SIZE           11
#define  MANTISSA_SIZE           52
#define  IMPLICIT_1_DIFF_E_MASK  0x10
#define  IMPLICIT_1_SAME_E_MASK  0x20


typedef  unsigned long long   uint64_t;
typedef  unsigned short       uint16_t;
typedef  short                int16_t;
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
    for (int i = SIZE_ARR-1; i >= 0; i--)
    {
        printf("%hx ", in[i]);
    }
    printf("\n");
}

uint16_t power2(uint16_t x)
{
    uint16_t result = 1;
    for (int i = 0; i < x; i++)
    {
        result *= 2;
    }

    return result;
}

void round_64bit_num_arr(uint16_t* in, uint16_t* out)
{
    //
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

double_repr arr_to_double_repr(uint16_t* in)
{
    double_repr num;

    int16_t mask = 0;
    uint16_t pow = -1;

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
    out[SIZE_ARR-1] |= in->mantissa[SIZE_ARR-1] & 0x000F;
    for (int i = SIZE_ARR-2; i >= 0; i--)
    {
        out[i] = in->mantissa[i];
    }
    printf("drta_out: ");
    print_array(out);
}

void perform_fract_shift(double_repr* in, uint16_t shift)
{
    for (int i = 0; i < SIZE_ARR; i++)
    {
        in->mantissa[i] >>= shift;
        if (i < SIZE_ARR-1)
            in->mantissa[i] |= in->mantissa[i+1] << (PROCESSOR_BIT_DEPTH-shift);
    }
}

void perform_int_shift(double_repr* in, uint16_t shift)
{
    in->mantissa[0] = in->mantissa[0+shift];
    in->mantissa[1] = (1+shift < SIZE_ARR) ? in->mantissa[1+shift] : 0x0000;
    in->mantissa[2] = (2+shift < SIZE_ARR) ? in->mantissa[2+shift] : 0x0000;
    in->mantissa[3] = 0x0000;
}

void shift_mantissa_bits_final(double_repr* in)
{
    in->exponent += 1; 

    printf("before final shift: ");
    print_array(in->mantissa);

    const uint16_t shift = 1;

    perform_fract_shift(in, shift);

    printf("after final shift: ");
    print_array(in->mantissa);
}

void shift_mantissa_bits(double_repr* in, const uint16_t exp_diff)
{
    printf("before shift: ");
    print_array(in->mantissa);
    /* разность экспонент может быть большой, но благо мы можем
       позволить себе сдвинуть максимум на 52 бит влево */
    uint16_t shift = (exp_diff <= 52) ? exp_diff : 52;
    // увеличение экспоненты
    //in->exponent += shift; //?
    /* два случая: сдвиг на менее чем 16 бит и сдвиг на более чем 16 бит.
       в первом случае двигаем только побитово, а во втором сдвигаем
       и целые блоки по 16 бит */

    const uint16_t shift_int = shift / PROCESSOR_BIT_DEPTH;
    const uint16_t shift_fract = shift % PROCESSOR_BIT_DEPTH;
    printf("SHIFT_FRACT = %d\n", shift_fract);

    if (exp_diff != 0) in->mantissa[SIZE_ARR-1] |= IMPLICIT_1_DIFF_E_MASK;
    else in->mantissa[SIZE_ARR-1] |= IMPLICIT_1_SAME_E_MASK;

    if (shift_fract != 0) perform_fract_shift(in, shift_fract);
    
    if (shift_int != 0) perform_int_shift(in, shift_int);

    printf("after shift: ");
    print_array(in->mantissa);
}

int normalize_double(double_repr* in) //TODO: убрать int после полной отладки
{
    int16_t mask = 0;
    uint16_t pow = -1;

    in->sign = 0;
    // printf("MANTISSA [4] = %llx\n", in->mantissa[SIZE_ARR-1]);
    print_array(in->mantissa);
    if ((in->mantissa[SIZE_ARR-1] >> 4) > 0)
    {
        shift_mantissa_bits_final(in);
        return 1;
    }
    return 0;
}

uint16_t calculate_diff(uint16_t a, uint16_t b)
{
    return (a > b) ? a - b : b - a;
}

uint16_t determine_max(uint16_t a, uint16_t b)
{
    return (a > b) ? a : b;
}

void add_normalised_mantissas(double_repr* a, double_repr* b, double_repr* out)
{
    uint16_t least;
    uint16_t excess = 0;
    for (int i = 0; i < SIZE_ARR; i++)
    {
        out->mantissa[i] = a->mantissa[i] + b->mantissa[i] + excess;
        least = (a->mantissa[i] < b->mantissa[i]) ? a->mantissa[i] : b->mantissa[i];
        excess = (out->mantissa[i] < least) ? 1 : 0;
    }
}

void add(uint16_t* a, uint16_t* b, uint16_t* out) //TODO: нужна поддержка вычитания и правильного округления
{
    double_repr a_repr = arr_to_double_repr(a);
    double_repr b_repr = arr_to_double_repr(b);
    double_repr sum = {0, 0, 0};

    printf("exp_a = %d\n", a_repr.exponent);
    print_array(a_repr.mantissa);
    printf("exp_b = %d\n", b_repr.exponent);
    print_array(b_repr.mantissa);
    
    const uint16_t exponent_diff = calculate_diff(a_repr.exponent, b_repr.exponent);
    double_repr* repr_to_shift = (a_repr.exponent < b_repr.exponent) ? &a_repr: &b_repr;
    shift_mantissa_bits(repr_to_shift, exponent_diff);
    sum.exponent = determine_max(a_repr.exponent, b_repr.exponent);

    printf("=+=+=+=\n");
    print_array(a_repr.mantissa);
    print_array(b_repr.mantissa);

    add_normalised_mantissas(&a_repr, &b_repr, &sum);

    printf("%hx\n", sum.mantissa[SIZE_ARR-1]);
    print_array(sum.mantissa);
    int status = normalize_double(&sum);
    
    if (status == 1) printf("STATUS");
    double_repr_to_arr(&sum, out);
}

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

void multiply(uint16_t* a, uint16_t* b, uint16_t* out)
{
    double_repr a_repr = arr_to_double_repr(a);
    double_repr b_repr = arr_to_double_repr(b);
    double_repr mult = {0, 0, 0};

    printf("exp_a = %d\n", a_repr.exponent);
    print_array(a_repr.mantissa);
    printf("exp_b = %d\n", b_repr.exponent);
    print_array(b_repr.mantissa);

    if (a_repr.exponent < b_repr.exponent) //TODO: упростить
    {
        const uint16_t exponent_diff = b_repr.exponent - a_repr.exponent;
        if (exponent_diff > 0) shift_mantissa_bits(&a_repr, exponent_diff);
    }
    else
    {
        const uint16_t exponent_diff = a_repr.exponent - b_repr.exponent;
        if (exponent_diff > 0) shift_mantissa_bits(&b_repr, exponent_diff);
    }

    //определение знака результата
    mult.sign = a_repr.sign ^ b_repr.sign;

    //умножение мантисс 
    uint16_t excess = 0;
    uint16_t rem = 0;
    for (int b = 0; b < SIZE_ARR; b++)
    {
        for (int a = 0; a < SIZE_ARR; a++)
        { 
            multiply16(a_repr.mantissa[a], b_repr.mantissa[b], &rem, &excess);
            if (a+b < SIZE_ARR) mult.mantissa[a+b] += rem;
            if (a+b+1 < SIZE_ARR) mult.mantissa[a+b+1] += excess;
        }
    }

    //нормализация
    int status = normalize_double(&mult);
    //shift_mantissa_bits(&mult, 1);

    //вычисление итоговой экспоненты
    uint16_t exp = a_repr.exponent + b_repr.exponent - 1023;
    exp += (status == 1) ? 1 : 0;

    //перевод представления числа в массив
    double_repr_to_arr(&mult, out);
}

int main()
{
    int choice;
    double a;
    double b;
    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
        a = 3.1415926536;
        b = 2.7182818284;
        break;
    case 2:
        a = 0.1415926536;
        b = 0.7182818284;
        break;
    case 3:
        a = 3.14;
        b = 9.72;
        break;
    case 4:
        a = 7.1415926536;
        b = 2122.7182818284;
        break;
    default:
        a = 3.1415926536;
        b = 2.7182818284;
        break;
    }

    double c = a + b;

    printf("======= EXPECTED: =======\n");
    printf("A + B = %.10f\n", a + b);
    printf("A * B = %.10f\n", a * b);
    printf("=========================\n");

    uint16_t a_converted[SIZE_ARR];
    convert_double_to_arr(a, a_converted);
    uint16_t b_converted[SIZE_ARR];
    convert_double_to_arr(b, b_converted);
    uint16_t c_converted[SIZE_ARR];
    convert_double_to_arr(c, c_converted);
    uint16_t c_converted_mult[SIZE_ARR];
    convert_double_to_arr(a*b, c_converted_mult);

    uint16_t c_array_add[SIZE_ARR];
    add(a_converted, b_converted, c_array_add);
    uint16_t c_array_mult[SIZE_ARR];
    multiply(a_converted, b_converted, c_array_mult);

    printf("short arrays:\n");
    print_array(a_converted);
    print_array(b_converted);
    print_array(c_converted);
    print_array(c_array_add);
    print_array(c_converted_mult);
    print_array(c_array_mult);

    double _a = convert_arr_to_double(a_converted);
    double _b = convert_arr_to_double(b_converted);
    double _c = convert_arr_to_double(c_array_add);
    double _cm = convert_arr_to_double(c_array_mult);

    printf("back to double:\n");
    printf("%.10f %.10f %.10f %.10f", _a, _b, _c, _cm);

    return 0;
}