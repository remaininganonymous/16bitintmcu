#include <stdio.h>
#include "types.h"

void print_array(uint16_t* in);

void print_array_reversed(uint16_t* in);

void convert_double_to_arr(double in, uint16_t* out);

double convert_arr_to_double(uint16_t* in);

double_repr arr_to_double_repr(uint16_t* in);

void double_repr_to_arr(double_repr* in, uint16_t* out);