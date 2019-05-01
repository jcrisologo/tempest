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

double fc;
int resx;
int resy;
int horizontalspan;
int verticalspan;
double pixelclock;

SDL_Window* sdlWindow;
SDL_Renderer* sdlRenderer;
SDL_Texture* send;
SDL_Surface* screen;
uint8_t* audioread;
int* audiobuff;
int audiolength;

void play ()
{
  int pos=0;
  unsigned int ticksCurr = SDL_GetTicks();
  unsigned int ticksLast = ticksCurr;
  unsigned int missed = 0;

  pulse_shaper_t ps;
  pulse_shaper_params_t ps_params;

  ps_params.sps = verticalspan;
  ps_params.delay = 2;
  ps_params.beta = 0.5;
  ps_params.gain = 84;
  pulse_shaper_init(&ps, ps_params);

  mixer_t m;
  mixer_params_t m_params;
  m_params.fc = fc;
  m_params.fs = pixelclock;
  m_params.upsample = horizontalspan;
  mixer_init(&m, m_params);

  int8_t buff[verticalspan];
  int8_t horbuff[horizontalspan];

//  unsigned int data[] = {-1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1};
  unsigned int data[] = {-1, -1, -1, 1,
	                 -1, -1, 1, -1,
			 -1, -1, 1, 1,
			 -1, 1, -1, -1,
			 -1, 1, -1, 1,
			 -1, 1, 1, -1,
			 -1, 1, 1, 1,
			 1, -1, -1, -1,
			 1, -1, -1, 1,
			 1, -1, 1, -1,
			 1, -1, 1, 1,
			 1, 1, -1, -1,
			 1, 1, -1, 1,
			 1, 1, 1, -1,
			 1, 1, 1, 1,};

  size_t data_len = sizeof(data)/sizeof(data[0]);

  while (1) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) if (event.type == SDL_MOUSEBUTTONDOWN) {printf("%d\n", missed); exit(0);}
    ticksLast = ticksCurr;

       pulse_shaper_advance(&ps, data[pos++], buff);

       for (int i = 0; i < resy; i++)
       {
          mixer_mix(&m, buff[i], 50, (uint8_t*)&screen->pixels[i*resx], NULL);
       }

       send = SDL_CreateTextureFromSurface(sdlRenderer, screen);
       SDL_RenderCopy(sdlRenderer, send, NULL, NULL);
       SDL_RenderPresent(sdlRenderer);
       SDL_DestroyTexture(send);
       pos %= data_len;
ticksCurr = SDL_GetTicks();
if (ticksCurr - ticksLast > 20) missed++;
//SDL_Delay(150);
  };

};

void usage()
{
  printf("\nwrong parameters ! read readme file!\n\n");
  exit(1);
};

int main(int argc, char *argv[])
{
  fc=10000000.0;
  resx=1024;
  resy=768;
  horizontalspan=1400;
  verticalspan=1600;
  pixelclock=105.0 * 1e6;
  char *filename;

  atexit(SDL_Quit);

  if (argc!=8) usage();
  pixelclock=atof(argv[1]);
  resx=atol(argv[2]);
  resy=atol(argv[3]);
  horizontalspan=atol(argv[4]);
  verticalspan=atol(argv[5]);
  fc=atof(argv[6]);
  filename=argv[7]; 
  printf("\n"
	 "Pixel Clock %.0f Hz\n"
	 "X Resolution %d Pixels\n"
	 "Y Resolution %d Pixels\n"
	 "Horizontal Total %d Pixels\n"
	 "Vertical Total %d Pixels\n"
	 "AM Carrier Frequency %.0f Hz\n"
	 "\n\n",
	 pixelclock,resx,resy,horizontalspan,verticalspan,fc);

  FILE* audio = fopen(filename, "rb");
  fseek(audio, 0, SEEK_END);
  audiolength = ftell(audio);
  fseek(audio, 0, SEEK_SET);
  audioread = (uint8_t*)malloc(audiolength);
  audiobuff = (int*)malloc(sizeof(int) * audiolength);
  fread(audioread, audiolength, 1, audio);

  for (int i = 0; i < audiolength; i++)
  {
     audiobuff[i] = audioread[i]/2;
  }

  monitor_modulator_t mm;
  monitor_modulator_params_t mm_params;
  mm_params.resx = resx;
  mm_params.resy = resy;
  mm_params.spanx = horizontalspan;
  mm_params.spany = verticalspan;
  mm_params.mod_mode = MOD_MODE_QPSK;
  mm_params.fc = fc;
  mm_params.fs = pixelclock;

  monitor_modulator_init(&mm, mm_params);

    SDL_Event event;
  for (int i = 0; i < 600; i++)
  {
    while(SDL_PollEvent(&event)) 
       if (event.type == SDL_MOUSEBUTTONDOWN) 
          exit(0);
     monitor_modulator_transmit(&mm, i%4);
     SDL_Delay(250);
  }

  return 0;
};
