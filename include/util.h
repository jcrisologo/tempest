#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>

float max(float* f_arr, size_t size);

// 8-bit signed saturating addition
// This function will turn arithmetic overflow into clipping
// If flag is not NULL, it will be set to 1 if clipping occurred
// 0 otherwise
int8_t ssadd8(int8_t a, int8_t b, int* flag);

#endif
