#ifndef _PULSE_SHAPER_H_
#define _PULSE_SHAPER_H_

#include <stdint.h>

typedef struct pulse_shaper_params
{
   unsigned int sps;
   unsigned int delay;
   float beta;
} pulse_shaper_params;

typedef struct pulse_shaper
{
   pulse_shaper_params params;
   uint8_t* buffer;
   uint8_t* pulse;
   unsigned int buffer_pos;
} pulse_shaper;

void pulse_shaper_init(pulse_shaper* shaper, pulse_shaper_params params);
void pulse_shaper_clean(pulse_shaper* shaper);

// Advance will produce sps+1 samples of output with every call
// This output will not correspond with the current next_symbol,
// But instead the next_symbol from delay calls prior
void pulse_shaper_advance(pulse_shaper* shaper, int next_symbol, int8_t* output);

#endif
