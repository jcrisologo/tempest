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
double pixelclock;

SDL_Window* sdlWindow;
SDL_Renderer* sdlRenderer;
SDL_Texture* send;
SDL_Surface* screen;
uint32_t* pixbuff;
uint8_t* audioread;
int* audiobuff;
unsigned int* carrier;
int audiolength;
uint32_t* mod_lut[256];
int* rrcos;
size_t pulse_len;

void play ()
{
  int pos=0;
  unsigned int ticksCurr;
  unsigned int ticksLast;
  while (1) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) if (event.type == SDL_MOUSEBUTTONDOWN) {printf("%d\n", ticksCurr - ticksLast); exit(0);}
    ticksLast = ticksCurr;

       if (pos >= pulse_len) pos -= pulse_len;

       for (int i = 0; i < resy; i++)
       {
          int val = rrcos[(pos + i) % pulse_len];
          for (int j = 0; j < resx; j++)
	  {
             ((uint32_t*)screen->pixels)[i * resx + j] = mod_lut[val][j];
	  }
       }
       
       SDL_UpdateTexture(send, NULL, screen->pixels, resx*sizeof(uint32_t));
       SDL_RenderCopy(sdlRenderer, send, NULL, NULL);
       SDL_RenderPresent(sdlRenderer);
       pos += verticalspan;
ticksCurr = SDL_GetTicks();
SDL_Delay(1000);
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

  printf(
	 "\n"
	 "Tempest for Eliza - by erikyyy !\n"
	 "--------------------------------\n"
	 "\n"
	 "Read the README file to understand what's happening\n"
	 "if you do not read it, you will NOT know what to do\n"
	 );

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

  sdlWindow = SDL_CreateWindow("waha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_CreateRGBSurface(0, resx, resy, 32, 0, 0, 0, 0);
  send = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, resx, resy);
  SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
  SDL_RenderClear(sdlRenderer);
  SDL_RenderPresent(sdlRenderer);

  carrier = (unsigned int*)malloc(sizeof(int) * resx * 256);
  for (int j = 0; j < 255; j++)
  {
    mod_lut[j] = carrier + j*resx;
  for (int i = 0; i < resx; i++)
  {
     unsigned int val = (unsigned int)(127 + cos(2.0*M_PI*fc/pixelclock*i)*(j - 127));
     mod_lut[j][i] = SDL_MapRGB(screen->format, val, val, val);
  }
  }
  
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
