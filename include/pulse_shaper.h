#ifndef _PULSE_SHAPER_H_
#define _PULSE_SHAPER_H_

#include <stdint.h>

typedef struct pulse_shaper_params
{
   unsigned int sps;
   unsigned int delay;
   float beta;
   float gain;
} pulse_shaper_params_t;

typedef struct pulse_shaper
{
   pulse_shaper_params_t params;
   int8_t* buffer;
   int8_t* pulse;
   unsigned int buffer_pos;
} pulse_shaper_t;

void pulse_shaper_init(pulse_shaper_t* shaper, pulse_shaper_params_t params);
void pulse_shaper_clean(pulse_shaper_t* shaper);

// Advance will produce sps+1 samples of output with every call
// This output will not correspond with the current next_symbol,
// But instead the next_symbol from delay calls prior
void pulse_shaper_advance(pulse_shaper_t* shaper, int next_symbol, int8_t* output);

#endif
