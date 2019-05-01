#include <SDL2/SDL.h>
#include <pulse_shaper.h>
#include <mixer.h>
#include <monitor_modulator.h>

void monitor_modulator_sdl_init(monitor_modulator_t* mm)
{
   mm->window = SDL_CreateWindow("PROSPERO", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
   mm->renderer = SDL_CreateRenderer(mm->window, -1, SDL_RENDERER_PRESENTVSYNC);
   mm->surface = SDL_CreateRGBSurface(0, mm->params.resx, mm->params.resy, 8, 0, 0, 0, 0);

   SDL_Color grayscale_palette[256];
   for (int i = 0; i < 256; i++)
   {
      grayscale_palette[i].r = grayscale_palette[i].g = grayscale_palette[i].b = (uint8_t)i;
   }
   SDL_SetPaletteColors(mm->surface->format->palette, grayscale_palette, 0, 256);

   SDL_SetRenderDrawColor(mm->renderer, 0, 0, 0, 255);
   SDL_RenderClear(mm->renderer);
   SDL_RenderPresent(mm->renderer);
   SDL_Delay(1000);
}

void monitor_modulator_init(
      monitor_modulator_t* mm, 
      monitor_modulator_params_t mod_params)
{
   pulse_shaper_params_t ps_params;
   mixer_params_t mix_params;

   mm->params = mod_params;

   ps_params.sps = mod_params.spany;
   ps_params.delay = 2;
   ps_params.beta = 0.5;
   ps_params.gain = 80;

   mix_params.fc = mod_params.fc;
   mix_params.fs = mod_params.fs;
   mix_params.upsample = mod_params.spanx;

   mm->pulse_shaper = malloc(sizeof(pulse_shaper_t));
   mm->mixer = malloc(sizeof(mixer_t));

   pulse_shaper_init(mm->pulse_shaper, ps_params);
   mixer_init(mm->mixer, mix_params);
   monitor_modulator_sdl_init(mm);
}
