#include "types.h"

void shift_mantissa_bits(double_repr* in, uint16_t exp_diff);

void shift_mantissa_bits_final(double_repr* in);

int normalize_double(double_repr* in);

void add(uint16_t* a, uint16_t* b, uint16_t* out);

void multiply16(uint16_t a, uint16_t b, uint16_t *low, uint16_t *high);

void multiply(uint16_t* a, uint16_t* b, uint16_t* out);

void divide(uint16_t* a, uint16_t* b, uint16_t* out);