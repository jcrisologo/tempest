#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <util.h>
#include <mixer.h>

void lut_init(int8_t* cos_lut[], int8_t* sin_lut[], mixer_params mp)
{
   cos_lut[0] = malloc(_MIXER_LUT_SIZE * mp.upsample);
   sin_lut[0] = malloc(_MIXER_LUT_SIZE * mp.upsample);

   // The LUTs are an array of pointers into the space we mallocced above
   for (int scale = 1; scale < _MIXER_LUT_SIZE; scale++)
   {
      cos_lut[scale] = cos_lut[0] + mp.upsample * scale;
      sin_lut[scale] = sin_lut[0] + mp.upsample * scale;
   }

   // Every row of the LUT contains a scaled copy of a sinusoidal
   for (int n = 0; n < mp.upsample; n++)
   {
      float cos_val = cos(2.0 * M_PI * mp.fc / mp.fs * n);
      float sin_val = sin(2.0 * M_PI * mp.fc / mp.fs * n);

      for (int scale = -(_MIXER_LUT_SIZE + 1) / 2; scale < _MIXER_LUT_SIZE / 2; scale++)
      {
         int scale_idx = scale + (_MIXER_LUT_SIZE + 1) / 2;
         cos_lut[scale_idx][n] = (int)(scale * cos_val);
         sin_lut[scale_idx][n] = (int)(scale * sin_val);
      }
   }
}

void mixer_init(mixer* m, mixer_params mp)
{
   m->params = mp;
   lut_init(m->cos_lut, m->sin_lut, mp);
}

void mixer_mix(mixer* m, int i_samp, int q_samp, int8_t* buffer_out, int* samples_clipped)
{
   int idx_offset = (_MIXER_LUT_SIZE + 0) / 2;
   int flag;
   *samples_clipped = 0;

   for (int i = 0; i < m->params.upsample; i++)
   {
      buffer_out[i] = ssadd8(m->cos_lut[i_samp + idx_offset][i], m->sin_lut[q_samp + idx_offset][i], &flag);
      if (flag) (*samples_clipped)++;
   }
}
