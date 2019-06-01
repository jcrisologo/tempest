#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <pulse_shaper.h>
#include <mixer.h>
#include <monitor_modulator.h>
#include <libconfig.h>

int stringtomodscheme(const char* str)
{
   if (strcmp(str, "AM") == 0)
      return MOD_MODE_AM;
   if (strcmp(str, "OOK") == 0)
      return MOD_MODE_OOK;
   if (strcmp(str, "BPSK") == 0)
      return MOD_MODE_BPSK;
   if (strcmp(str, "QPSK") == 0)
      return MOD_MODE_QPSK;
   if (strcmp(str, "DQPSK") == 0)
      return MOD_MODE_DQPSK;

   return MOD_MODE_ERR;
}

void load_conf(char* filename, monitor_modulator_params_t* params)
{
   config_t config;
   config_init(&config);
   config_set_auto_convert(&config, 1);
   config_read_file(&config, filename);

   config_lookup_float(&config, "display.pixelclock", &params->fs);
   config_lookup_int(&config, "display.xres", &params->resx);
   config_lookup_int(&config, "display.yres", &params->resy);
   config_lookup_int(&config, "display.xtot", &params->spanx);
   config_lookup_int(&config, "display.ytot", &params->spany);
   config_lookup_float(&config, "modulation.fc", &params->fc);

   const char* mod_mode_str;
   config_lookup_string(&config, "modulation.scheme", &mod_mode_str);
   params->mod_mode = stringtomodscheme(mod_mode_str);

   config_destroy(&config);
}

size_t read_data(const char* data_filename, uint8_t** dst)
{
   FILE* data_fp;
   size_t data_len;
   int ret;

   data_fp = fopen(data_filename, "rb");

   if (!data_fp)
   {
      printf("Couldn't open file?  Check your spelling!\n");
      exit(0);
   }
   
   fseek(data_fp, 0, SEEK_END);
   data_len = ftell(data_fp);
   fseek(data_fp, 0, SEEK_SET);
   *dst = malloc(data_len);
   ret = fread(*dst, data_len, 1, data_fp);

   if (!ret)
   {
      printf("Couldn't read\n");
      exit(0);
   }

   fclose(data_fp);

   return data_len;
}

void usage(const char* myname)
{
   printf("usage: %s config_file data_file\n"
          "\tsee the README\n", myname);
}

int main(int argc, char *argv[])
{
   char* conf_filename;
   char* data_filename;
   uint8_t* data_buf;
   size_t data_len;

   SDL_Event event;
   monitor_modulator_t mm;
   monitor_modulator_params_t mm_params;

   if (argc != 3) 
   {
      usage(argv[0]);
      exit(0);
   }

   atexit(SDL_Quit);

   conf_filename = argv[1];
   data_filename = argv[2]; 

   data_len = read_data(data_filename, &data_buf);
   load_conf(conf_filename, &mm_params);
   monitor_modulator_init(&mm, mm_params);

   while (1)
   {
      for (int i = 0; i < data_len; i++)
      {
         while (SDL_PollEvent(&event)) 
         {
            if (event.type == SDL_MOUSEBUTTONDOWN) 
            {
               goto exit;
            }
         }

         monitor_modulator_transmit_byte(&mm, data_buf[i]);
      }
   }

exit:
   monitor_modulator_destroy(&mm);
   free(data_buf);
   return 0;
}
