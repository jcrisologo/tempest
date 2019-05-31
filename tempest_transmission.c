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

void usage()
{
   printf("FUCK\n");
}

int main(int argc, char *argv[])
{
   char* conf_filename;
   char* data_filename;
   FILE* data_fp;
   unsigned char* data_buf;
   int data_len;

   if (argc != 3) 
   {
      usage();
      exit(0);
   }

   atexit(SDL_Quit);

   conf_filename = argv[1];
   data_filename = argv[2]; 

   data_fp = fopen(data_filename, "rb");
   fseek(data_fp, 0, SEEK_END);
   data_len = ftell(data_fp);
   fseek(data_fp, 0, SEEK_SET);
   data_buf = (uint8_t*)malloc(data_len);
   fread(data_buf, data_len, 1, data_fp);
   fclose(data_fp);

   monitor_modulator_t mm;
   monitor_modulator_params_t mm_params;
   load_conf(conf_filename, &mm_params);
   monitor_modulator_init(&mm, mm_params);

   // AM modulation needs an extra bit so we reduce dynamic range here
   // (last-minute kludge)
   if (mm_params.mod_mode == MOD_MODE_AM)
   {
      for (int i = 0; i < data_len; i++)
      {
         data_buf[i] = data_buf[i] / 2;
      }
   }


   SDL_Event event;
   
   while (1)
   {
      for (int i = 0; i < data_len; i++)
      {
         while (SDL_PollEvent(&event)) 
         {
            if (event.type == SDL_MOUSEBUTTONDOWN) 
            {
               exit(0);
            }
         }

         monitor_modulator_transmit_byte(&mm, data_buf[i]);
      }
   }

   return 0;
}
