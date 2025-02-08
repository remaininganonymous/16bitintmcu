#define  PROCESSOR_BIT_DEPTH  16
#define  SIZE_ARR             4
#define  SIGN_BIT_SIZE        1
#define  EXPONENT_SIZE        11
#define  MANTISSA_SIZE        52

typedef unsigned short        uint16_t;
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
    uint16_t  mantissa[4];
}
double_repr;