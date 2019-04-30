#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <pulse_shaper.h>
#include <mixer.h>

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

  pulse_shaper ps;
  pulse_shaper_params ps_params;

  ps_params.sps = verticalspan;
  ps_params.delay = 2;
  ps_params.beta = 0.5;
  ps_params.gain = 84;
  pulse_shaper_init(&ps, ps_params);

  mixer m;
  mixer_params m_params;
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
          mixer_mix(&m, buff[i], 0, horbuff, NULL);

          for (int j = 0; j < resx; j++)
          {
            ((uint32_t*)screen->pixels)[i * resx + j] = SDL_MapRGBA(screen->format, horbuff[j] + 128, horbuff[j] + 128, horbuff[j] + 128, 128);
          }
       }

       SDL_UpdateTexture(send, NULL, screen->pixels, screen->pitch);
       SDL_RenderCopy(sdlRenderer, send, NULL, NULL);
       SDL_RenderPresent(sdlRenderer);
       pos %= data_len;
ticksCurr = SDL_GetTicks();
if (ticksCurr - ticksLast > 20) missed++;
SDL_Delay(500);
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

  sdlWindow = SDL_CreateWindow("waha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_CreateRGBSurface(0, resx, resy, 32, 0, 0, 0, 0);
  send = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, resx, resy);
  SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
  SDL_RenderClear(sdlRenderer);
  SDL_RenderPresent(sdlRenderer);

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

  play();

  return 0;
};
