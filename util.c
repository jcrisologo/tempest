#include <stdint.h>
#include <stddef.h>
#include <util.h>

float max(float* f_arr, size_t size)
{
   float max = f_arr[0];

   for (int i = 1; i < size; i++)
   {
      max = max > f_arr[i] ? max : f_arr[i];
   }

   return max;
}

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

