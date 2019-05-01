#ifndef _MIXER_H_
#define _MIXER_H_

#include <stdint.h>

#define _MIXER_LUT_SIZE 256

typedef struct
{
   double fc;
   double fs;
   unsigned int upsample;
} mixer_params_t;

typedef struct
{
   mixer_params_t params;
   int8_t* sin_lut[_MIXER_LUT_SIZE];
   int8_t* cos_lut[_MIXER_LUT_SIZE];
} mixer_t;

void mixer_init(mixer_t* m, mixer_params_t mp);
void mixer_lut_init(int8_t* cos_lut[], int8_t* sin_lut[], mixer_params_t mp);
void mixer_mix(mixer_t* m, int i_samp, int q_samp, int8_t* buffer_out, int* samples_clipped);

#endif
