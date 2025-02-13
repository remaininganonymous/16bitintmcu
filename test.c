#include <stdio.h>

#define  PROCESSOR_BIT_DEPTH  16
#define  SIZE_ARR             4
#define  SIGN_BIT_SIZE        1
#define  EXPONENT_SIZE        11
#define  MANTISSA_SIZE        52

typedef  unsigned long long   uint64_t;
typedef  unsigned short       uint16_t;
typedef  union
{
    double    d;
    uint64_t  ull; 
}
double_ull_t; //для разбора дробного числа по частям
typedef  struct
{
    uint16_t  sign;
    uint16_t  exponent;
    uint16_t  mantissa[SIZE_ARR]; 
}
double_repr; //для переноса составляющих дробного числа по полям

void print_array(uint16_t* in)
{
    for (int i = SIZE_ARR-1; i >= 0; i--)
    {
        printf("%hx ", in[i]);
    }
    printf("\n");
}

//делает из числа массив
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

//обратно
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

//перевод массива числа в структуру
double_repr arr_to_double_repr(uint16_t* in)
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

//структура в массив
void double_repr_to_arr(double_repr* in, uint16_t* out)
{
    out[SIZE_ARR-1] = in->sign << 15;
    out[SIZE_ARR-1] |= in->exponent << 4;
    out[SIZE_ARR-1] |= in->mantissa[SIZE_ARR-1];
    for (int i = SIZE_ARR-2; i >= 0; i--)
    {
        out[i] = in->mantissa[i];
    }
}

//побитовый сдвиг мантиссы
void shift_mantissa_bits(double_repr* in, uint16_t exp_diff)
{
    /* разность экспонент может быть большой, но благо мы можем
       позволить себе сдвинуть максимум на 52 бит влево */
    uint16_t shift = (exp_diff <= 52) ? exp_diff : 52;
    // увеличение экспоненты
    in->exponent += shift; //?
    /* два случая: сдвиг на менее чем 16 бит и сдвиг на более чем 16 бит.
       в первом случае двигаем только побитово, а во втором сдвигаем
       и целые блоки по 16 бит */

    uint16_t shift_int = shift / PROCESSOR_BIT_DEPTH;
    uint16_t shift_fract = shift % PROCESSOR_BIT_DEPTH;

    // сдвиг внутри ячейки массива
    if (shift_fract != 0)
    {
        uint16_t buff = 0;
        for (int i = 0; i < SIZE_ARR; i++)
        {
            if (i+1 < SIZE_ARR) buff = in->mantissa[i+1] << (PROCESSOR_BIT_DEPTH-shift_fract);
            else buff = 0x0002; //тут может быть подвох, наблюдаю
            in->mantissa[i] = (in->mantissa[i] >> shift_fract) | buff;
        }
    }
    // сдвиг ячеек внутри массива
    if (shift_int != 0)
    { 
        in->mantissa[0] = in->mantissa[0+shift_int];
        in->mantissa[1] = (1+shift_int < SIZE_ARR) ? in->mantissa[1+shift_int] : 0x0000;
        in->mantissa[2] = (2+shift_int < SIZE_ARR) ? in->mantissa[2+shift_int] : 0x0000;
        in->mantissa[3] = 0x0000;
    }
}

//нормализация полученного числа
int normalize_double(double_repr* in)
{
    // сдвиг, если мантисса занимает 53 бит считая справа налево
    if ((in->mantissa[SIZE_ARR-1] >> 3) > 0) //кажется небольшое расхождение с IEEE754
    {
        shift_mantissa_bits(in, 1);
        return 1;
    }
    return 0;
}

void add(uint16_t* a, uint16_t* b, uint16_t* out) //нужна поддержка вычитания
{
    double_repr a_repr = arr_to_double_repr(a);
    double_repr b_repr = arr_to_double_repr(b);
    double_repr sum = {0, 0, 0};

    //нам нужно определиться, какое число нужно выравнивать относительно какого
    // (ищем число с меньшей экспонентой, чтобы выровнять его относительно большей)
    if (a_repr.exponent < b_repr.exponent)
    {
        uint16_t exponent_diff = b_repr.exponent - a_repr.exponent;
        if (exponent_diff > 0) shift_mantissa_bits(&a_repr, exponent_diff);
        sum.exponent = b_repr.exponent;
    }
    else
    {
        uint16_t exponent_diff = a_repr.exponent - b_repr.exponent;
        if (exponent_diff > 0) shift_mantissa_bits(&b_repr, exponent_diff);
        sum.exponent = a_repr.exponent;
    }

    // по сути сложение в столбик с переносом единицы на высший разряд
    uint16_t least;
    uint16_t excess = 0;
    for (int i = 0; i < SIZE_ARR; i++)
    {
        sum.mantissa[i] = a_repr.mantissa[i] + b_repr.mantissa[i] + excess;
        least = (a_repr.mantissa[i] < b_repr.mantissa[i]) ? a_repr.mantissa[i] : b_repr.mantissa[i];
        excess = (sum.mantissa[i] < least) ? 1 : 0;
    }
    
    normalize_double(&sum);
    double_repr_to_arr(&sum, out);
}

//неизбежно переполнение, так как мы ограничены 16-ю битами, поэтому результат произведения
//делится здесь на "старшие" и "младшие" биты
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

//пока что функция работает неопредсказуемо. работа над ней продолжается
void multiply(uint16_t* a, uint16_t* b, uint16_t* out)
{
    double_repr a_repr = arr_to_double_repr(a);
    double_repr b_repr = arr_to_double_repr(b);
    double_repr mult = {0, 0, 0};

    //выравнивание чисел
    if (a_repr.exponent < b_repr.exponent)
    {
        uint16_t exponent_diff = b_repr.exponent - a_repr.exponent;
        if (exponent_diff > 0) shift_mantissa_bits(&a_repr, exponent_diff);
        mult.exponent = b_repr.exponent;
    }
    else
    {
        uint16_t exponent_diff = a_repr.exponent - b_repr.exponent;
        if (exponent_diff > 0) shift_mantissa_bits(&b_repr, exponent_diff);
        mult.exponent = a_repr.exponent;
    }

    //определение знака результата
    mult.sign = a_repr.sign ^ b_repr.sign;

    //сложение экспонент
    uint16_t exp = (a_repr.exponent - 1023) + (b_repr.exponent - 1023);
    
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
    shift_mantissa_bits(mult.mantissa, 1);

    //вычисление итоговой экспоненты
    exp += (status == 1) ? 1 : 0;
    mult.exponent = exp + 1023;

    //перевод представления числа в массив
    double_repr_to_arr(&mult, out);
}

int main()
{
    double a = 3.1415926536;
    double b = 2.7182818284;
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

    double _a = convert_arr_to_double(a_converted);
    double _b = convert_arr_to_double(b_converted);
    double _c = convert_arr_to_double(c_array_add);
    double _cm = convert_arr_to_double(c_array_mult);

    printf("back to double:\n");
    printf("a = %.10f, b = %.10f\n", _a, _b);
    printf("a + b = %.10f\n", _c);

    return 0;
}