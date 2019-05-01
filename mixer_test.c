#include <stdio.h>
#include <mixer.h>

int main()
{
   mixer_t m;
   mixer_params_t mp;
   int8_t buffer[1024];
   int samples_clipped;

   mp.fs = 64;
   mp.fc = 2;
   mp.upsample = 1024;

   mixer_init(&m, mp);

   mixer_mix(&m, 0, 127, buffer, &samples_clipped);

   for (int i = 0; i < 1024; i++)
   {
      printf("%d ", buffer[i]);
   }
   printf("\n%d clipped\n", samples_clipped);
}
