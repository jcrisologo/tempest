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

double fc;
int resx;
int resy;
int horizontalspan;
int verticalspan;
double pixelclock;

uint8_t* audioread;
int* audiobuff;
int audiolength;

int stringtomodscheme(char* str)
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

int main(int argc, char *argv[])
{
  char* conf_file;
  char* data_file;

  atexit(SDL_Quit);

  conf_file = argv[1];
  data_file = argv[2]; 

  FILE* audio = fopen(data_file, "rb");
  fseek(audio, 0, SEEK_END);
  audiolength = ftell(audio);
  fseek(audio, 0, SEEK_SET);
  audioread = (uint8_t*)malloc(audiolength);
  audiobuff = (int*)malloc(sizeof(int) * audiolength);
  fread(audioread, audiolength, 1, audio);

  for (int i = 0; i < audiolength; i++)
  {
     audiobuff[i] = audioread[i] / 2;
  }

  monitor_modulator_t mm;
  monitor_modulator_params_t mm_params;
  load_conf(conf_file, &mm_params);

  monitor_modulator_init(&mm, mm_params);

    SDL_Event event;
    unsigned int data[] = {0, 0, 0, 0, 0, 0, 1};
//  unsigned int data[] = {0, 0, 0, 0,
//	                 0, 0, 0, 1,
//	                 0, 0, 1, 0,
//			 0, 0, 1, 1,
//			 0, 1, 0, 0,
//			 0, 1, 0, 1,
//			 0, 1, 1, 0,
//			 0, 1, 1, 1,
//			 1, 0, 0, 0,
//			 1, 0, 0, 1,
//			 1, 0, 1, 0,
//			 1, 0, 1, 1,
//			 1, 1, 0, 0,
//			 1, 1, 0, 1,
//			 1, 1, 1, 0,
//			 1, 1, 1, 1,};
  int pos = 0;
    while(1)
  for (int i = 0; i < 600; i++)
  {
    while(SDL_PollEvent(&event)) 
       if (event.type == SDL_MOUSEBUTTONDOWN) 
          exit(0);
     monitor_modulator_transmit(&mm, data[pos++]);
     pos %= 7;
     //SDL_Delay(150);
  }

  return 0;
};
