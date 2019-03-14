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
int* carrier;
int audiolength;

void play ()
{
  int pos=0;
  while (1) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) if (event.type == SDL_MOUSEBUTTONDOWN) exit(0);

       if (pos >= audiolength) pos -= audiolength;

       for (int i = 0; i < resy; i++)
       {
          int val = audioread[(pos + i) % audiolength];
          for (int j = 0; j < resx; j++)
	  {
             int modulated = carrier[j] * val;
	     modulated >>= 9;
	     modulated += 128;
             uint32_t rgb = SDL_MapRGB(screen->format, modulated, modulated, modulated);
             ((uint32_t*)screen->pixels)[i * resx + j] = rgb;
	  }
       }
       
       SDL_UpdateTexture(send, NULL, screen->pixels, resx*sizeof(uint32_t));
       SDL_RenderCopy(sdlRenderer, send, NULL, NULL);
       SDL_RenderPresent(sdlRenderer);
       pos += verticalspan;
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

  carrier = (int*)malloc(sizeof(int) * resx);
  for (int i = 0; i < resx; i++)
  {
     carrier[i] = int(sin(2*M_PI*fc/pixelclock*i)*256.0);
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
     audiobuff[i] = (int(audioread[i]) - 128);
  }

  play();

  return 0;
};
