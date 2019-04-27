#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pulse_shaper.h"

int main()
{
   pulse_shaper_params ps_params;
   pulse_shaper ps;
   int8_t output[6];

   ps_params.sps = 6;
   ps_params.delay = 2;
   ps_params.beta = 0.5;
   ps_params.gain = 127;

   pulse_shaper_init(&ps, ps_params);
   pulse_shaper_advance(&ps, 1, output);
   for (int i = 0; i < 24; i++)
   {
      pulse_shaper_advance(&ps, 0, output);

      for (int j = 0; j < 6; j++)
      {
         printf("%d ", output[j]);
      }
      printf("\n");
   }
	

   for (int k = 0; k < 4; k++)
   {
   for (int i = 0; i < 4; i++)
   {
      pulse_shaper_advance(&ps, 1, output);

      for (int j = 0; j < 6; j++)
      {
         printf("%d ", output[j]);
      }
      printf("\n");
   }

   for (int i = 0; i < 4; i++)
   {
      pulse_shaper_advance(&ps, -1, output);

      for (int j = 0; j < 6; j++)
      {
         printf("%d ", output[j]);
      }
      printf("\n");
   }
   }

   for (int i = 0; i < 24; i++)
   {
      pulse_shaper_advance(&ps, 1, output);

      for (int j = 0; j < 6; j++)
      {
         printf("%d ", output[j]);
      }
      printf("\n");
   }

   pulse_shaper_clean(&ps);

   return 0;
}
