#ifndef _MIXER_H_
#define _MIXER_H_

#include <stdint.h>

#define _MIXER_LUT_SIZE 256

typedef struct
{
   unsigned int fc;
   unsigned int fs;
   unsigned int upsample;
} mixer_params;

typedef struct
{
   mixer_params params;
   int8_t* sin_lut[_MIXER_LUT_SIZE];
   int8_t* cos_lut[_MIXER_LUT_SIZE];
} mixer;

void mixer_init(mixer* m, mixer_params mp);
void mixer_mix(mixer* m, int i_samp, int q_samp, int8_t* buffer_out, int* samples_clipped);

#endif
