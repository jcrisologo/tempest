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

double carrier;
int resx;
int resy;
int horizontalspan;
double pixelclock;

SDL_Window* sdlWindow;
SDL_Renderer* sdlRenderer;
SDL_Texture* send0;
SDL_Texture* send1;
SDL_Surface* screen;
uint32_t* pixbuff;

void play ()
{
unsigned int data[] = {0, 0, 0, 1,
	                 0, 0, 1, 0,
			 0, 0, 1, 1,
			 0, 1, 0, 0,
			 0, 1, 0, 1,
			 0, 1, 1, 0,
			 0, 1, 1, 1,
			 1, 0, 0, 0,
			 1, 0, 0, 1,
			 1, 0, 1, 0,
			 1, 0, 1, 1,
			 1, 1, 0, 0,
			 1, 1, 0, 1,
			 1, 1, 1, 0,
			 1, 1, 1, 1,};

  size_t data_len = sizeof(data)/sizeof(data[0]);
  int pos=0;
  while (1) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) if (event.type == SDL_MOUSEBUTTONDOWN) exit(0);

       if (pos >= data_len) pos = 0;

       if (data[pos])
       {
           SDL_RenderCopy(sdlRenderer, send1, NULL, NULL);
       }
       else
       {
           SDL_RenderCopy(sdlRenderer, send0, NULL, NULL);
       }
       SDL_RenderPresent(sdlRenderer);
       
       pos++;
  };
};

void usage()
{
  printf("\nwrong parameters ! read readme file!\n\n");
  exit(1);
};

int main(int argc, char *argv[])
{
  carrier=10000000.0;
  resx=1024;
  resy=768;
  horizontalspan=1400;
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

  if (argc!=7) usage();
  pixelclock=atof(argv[1]);
  resx=atol(argv[2]);
  resy=atol(argv[3]);
  horizontalspan=atol(argv[4]);
  carrier=atof(argv[5]);
  filename=argv[6];

  printf("\n"
	 "Pixel Clock %.0f Hz\n"
	 "X Resolution %d Pixels\n"
	 "Y Resolution %d Pixels\n"
	 "Horizontal Total %d Pixels\n"
	 "AM Carrier Frequency %.0f Hz\n"
	 "\n\n",
	 pixelclock,resx,resy,horizontalspan,carrier);

  sdlWindow = SDL_CreateWindow("waha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_CreateRGBSurface(0, resx, resy, 32, 0, 0, 0, 0);
  send0 = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, resx, resy);
  send1 = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, resx, resy);
  SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
  SDL_RenderClear(sdlRenderer);
  SDL_RenderPresent(sdlRenderer);
  

  double variance = horizontalspan*resy / 2 / 3;
  variance *= variance;
  double mean = horizontalspan*625 / 2;

  for (int posy = 0; posy < resy; posy++)
  {
     for (int posx = 0; posx < resx; posx++)
     {
        int pos = posy * horizontalspan + posx;
        int val = cos(2*M_PI*carrier/pixelclock*pos) * 128 + 127;
        double mask = exp(-pow(pos - mean, 2)/2/variance);
        ((uint32_t*)screen->pixels)[posy * resx + posx] = SDL_MapRGB(screen->format, int(mask*val), int(mask*val), int(mask*val));
     }
  }
  SDL_UpdateTexture(send1, NULL, screen->pixels, resx * sizeof(uint32_t));

  for (int posy = 0; posy < resy; posy++)
  {
     for (int posx = 0; posx < resx; posx++)
     {
        int pos = posy * horizontalspan + posx;
        int val = 0;-cos(2*M_PI*carrier/pixelclock*pos) * 128 + 127;
        double mask = exp(-pow(pos - mean, 2)/2/variance);
        ((uint32_t*)screen->pixels)[posy * resx + posx] = SDL_MapRGB(screen->format, int(mask*val), int(mask*val), int(mask*val));
     }
  }
  SDL_UpdateTexture(send0, NULL, screen->pixels, resx * sizeof(uint32_t));


  play ();

  return 0;
};
