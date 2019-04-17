#include <stdint.h>
#include <stdlib.h>
#include "rrcos.h"
#include "pulse_shaper.h"

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
      shaper->pulse[i] = (int)(pulse_float[i] / 1.2 * 127) + 127;
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
      shaper->buffer[sum_pos] += next_symbol * shaper->pulse[i];
   }

   shaper->buffer_pos += p.sps;
}
