/*
  tempest for eliza code
  Copyright (C) 2001  Erik Thiele

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <SDL.h>
#include <string.h>
#include "rrcos.h"

double fc;
int resx;
int resy;
int horizontalspan;
int verticalspan;
int cyclecount;
double pixelclock;

SDL_Window* sdlWindow;
SDL_Renderer* sdlRenderer;
SDL_Texture* send;
SDL_Surface* screen;
uint32_t* pixbuff;
uint8_t* audioread;
int* audiobuff;
float* carrier;
int audiolength;
SDL_Texture* mod_lut[256];
int* rrcos;
size_t pulse_len;

void play ()
{
  int pos=0;
  unsigned int ticksCurr;
  unsigned int ticksLast;
  SDL_Rect dstrect;
  dstrect.x = 0;
  dstrect.y = 0;
  dstrect.w = pixelclock / fc * cyclecount;
  dstrect.h = 1;

  while (1) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) if (event.type == SDL_MOUSEBUTTONDOWN) {printf("%d\n", ticksCurr - ticksLast); exit(0);}
    ticksLast = ticksCurr;

       if (pos >= pulse_len) pos -= pulse_len;

       for (int i = 0; i < resy; i++)
       {
          int val = rrcos[(pos + i) % pulse_len];
          dstrect.y = i;
          for (int j = 0; j < resx * 2; j+=pixelclock/fc * cyclecount)
	  {
             dstrect.x = j;
             SDL_RenderCopy(sdlRenderer, mod_lut[val], NULL, &dstrect);
	  }
       }
       
       SDL_RenderPresent(sdlRenderer);
       pos += verticalspan;
ticksCurr = SDL_GetTicks();
//SDL_Delay(5000);
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
  cyclecount = 1;
  char *filename;

  atexit(SDL_Quit);

  printf(
	 "\n"
	 "Tempest for Eliza - by erikyyy !\n"
	 "--------------------------------\n"
	 "\n"
	 "Read the README file to understand what's happening\n"
	 "if you do not read it, you will NOT know what to do\n"
	 );

  if (argc!=9) usage();
  pixelclock=atof(argv[1]);
  cyclecount=atol(argv[2]);
  resx=atol(argv[3]);
  resy=atol(argv[4]);
  horizontalspan=atol(argv[5]);
  verticalspan=atol(argv[6]);
  fc=atof(argv[7]);
  filename=argv[8];

  printf("\n"
	 "Pixel Clock %.0f Hz\n"
	 "X Resolution %d Pixels\n"
	 "Y Resolution %d Pixels\n"
	 "Horizontal Total %d Pixels\n"
	 "Vertical Total %d Pixels\n"
	 "AM Carrier Frequency %.0f Hz\n"
	 "\n\n",
	 pixelclock,resx,resy,horizontalspan,verticalspan,fc);

  sdlWindow = SDL_CreateWindow("waha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_CreateRGBSurface(0, resx, resy, 32, 0, 0, 0, 0);
  send = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, resx, resy);
  SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
  SDL_RenderClear(sdlRenderer);
  SDL_RenderPresent(sdlRenderer);

  size_t ppc = pixelclock / fc * cyclecount;
  carrier = (float*)malloc(sizeof(float) * ppc);
  for (int i = 0; i < ppc; i++)
  {
    carrier[i] = cos(2.0*M_PI*fc/pixelclock*i);
  }

  for (int j = 0; j < 255; j++)
  {
    unsigned int mod_cycle[ppc];

    mod_lut[j] = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ppc, 1);
    for (int i = 0; i < ppc; i++)
    {
      unsigned int val = (unsigned int)(127 + carrier[i]*(j - 127));
      mod_cycle[i] = SDL_MapRGB(screen->format, val, val, val);
    }
    SDL_UpdateTexture(mod_lut[j], NULL, &mod_cycle, ppc*sizeof(int));
  }

  free(carrier);
  
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

  pulse_len = 2*verticalspan*2+1;
  float* rrcos_float = (float*)malloc(pulse_len * sizeof(float));
  rrcos = (int*)malloc(pulse_len * sizeof(int));
  liquid_firdes_rrcos(verticalspan, 2, 0.5, 0, rrcos_float);

  for (int i = 0; i < pulse_len; i++)
  {
    rrcos[i] = (int)(rrcos_float[i] / 1.2 * 127) + 127;
  }
  free(rrcos_float);

  play();

  return 0;
};
