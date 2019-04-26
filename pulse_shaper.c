#include <stdint.h>
#include <stdlib.h>
#include "rrcos.h"
#include "pulse_shaper.h"

float max(float* f_arr, size_t size)
{
   float max = f_arr[0];

   for (int i = 1; i < size; i++)
   {
      max = max > f_arr[i] ? max : f_arr[i];
   }

   return max;
}

// 8-bit signed saturating addition
// This function will turn arithmetic overflow into clipping
// Ideally, tune the gain parameter so clipping doesn't occur
// Branch prediction should kick in if gain is tuned right
int8_t ssadd8(int8_t a, int8_t b, int* flag)
{
   if (flag) *flag = 0;

   if (a > 0)
   {
      if (b > INT8_MAX - a)
      {
         if (flag) *flag = 1;
         return INT8_MAX;
      }
   }
   else if (b < INT8_MIN - a)
   {
      if (flag) *flag = 1;
      return INT8_MIN;
   }
   return a + b;
}

void pulse_shaper_init(pulse_shaper* shaper, pulse_shaper_params params)
{
   unsigned int pulse_len = 2 * params.sps * params.delay;
   float pulse_float[pulse_len + 1];

   shaper->params = params;
   shaper->buffer = malloc(sizeof(*shaper->buffer) * pulse_len);
   shaper->pulse = malloc(sizeof(*shaper->pulse) * pulse_len);
   shaper->buffer_pos = 0;

   liquid_firdes_rrcos(params.sps, params.delay, params.beta, 0, pulse_float);

   for (int i = 0; i < pulse_len; i++)
   {
      shaper->pulse[i] = 
         (int)(pulse_float[i] / max(pulse_float, pulse_len + 1) * params.gain);

      shaper->buffer[i] = 0;
   }
}

void pulse_shaper_clean(pulse_shaper* shaper)
{
   free(shaper->buffer);
   free(shaper->pulse);
}

void pulse_shaper_advance(pulse_shaper* shaper, int next_symbol, int8_t* output)
{
   pulse_shaper_params p = shaper->params;
   unsigned int pulse_len = 2 * p.sps * p.delay;
   int output_start = shaper->buffer_pos - p.sps;
   output_start = (output_start + pulse_len) % pulse_len;

   for (int i = 0; i < p.sps; i++)
   {
      output[i] = shaper->buffer[output_start + i];
   }

   for (int i = 0; i < p.sps; i++)
   {
      shaper->buffer[output_start + i] = next_symbol * shaper->pulse[pulse_len - p.sps + i];
   }

   for (int i = 0; i < pulse_len - p.sps; i++)
   {
      int sum_pos = (shaper->buffer_pos + i) % pulse_len;
      shaper->buffer[sum_pos] = ssadd8(shaper->buffer[sum_pos], next_symbol * shaper->pulse[i], NULL);
   }

   shaper->buffer_pos += p.sps;
}
