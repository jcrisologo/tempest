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

   mm->pulse_shaper_i = malloc(sizeof(pulse_shaper_t));
   mm->pulse_shaper_q = malloc(sizeof(pulse_shaper_t));
   mm->mixer = malloc(sizeof(mixer_t));

   pulse_shaper_init(mm->pulse_shaper_i, ps_params);
   pulse_shaper_init(mm->pulse_shaper_q, ps_params);
   mixer_init(mm->mixer, mix_params);
   monitor_modulator_sdl_init(mm);

   mm->data_buffer_count = 0;
   for (int i = 0; i < sizeof(mm->data_buffer)/sizeof(mm->data_buffer[0]); i++)
   {
      mm->data_buffer[i] = 0;
   }
}

void monitor_modulator_display(monitor_modulator_t* mm)
{
   mm->texture = SDL_CreateTextureFromSurface(mm->renderer, mm->surface);
   SDL_RenderCopy(mm->renderer, mm->texture, NULL, NULL);
   SDL_RenderPresent(mm->renderer);
   SDL_DestroyTexture(mm->texture);
}

int monitor_modulator_bpsk_map(int data)
{
   if (data) return 1;
   else return -1;
}

void monitor_modulator_qpsk_map(int* data, int* samp)
{
   if (data[0]) samp[0] = 1;
   else samp[0] = -1;
   if (data[1]) samp[1] = 1;
   else samp[1] = -1;
}

// DQPSK remaps QPSK constellation points into rotations
void monitor_modulator_dqpsk_map(int* data, int* samp)
{
   // todo: bit lazy, this, but we are outta time
   static int last_samp[2] = {-1, -1};
   int qpsk[2];

   monitor_modulator_qpsk_map(data, qpsk);

   // Rotate last_samp based on the QPSK symbol
   // todo: kludge
   if (qpsk[0] == 1 && qpsk[1] == 1)
   {
      samp[0] = last_samp[0];
      samp[1] = last_samp[1];
   }
   else if (qpsk[0] == -1 && qpsk[1] == 1)
   {
      samp[0] = -last_samp[1];
      samp[1] = last_samp[0];
   }
   else if (qpsk[0] == -1 && qpsk[1] == -1)
   {
      samp[0] = -last_samp[0];
      samp[1] = -last_samp[1];
   }
   else if (qpsk[0] == 1 && qpsk[1] == -1)
   {
      samp[0] = last_samp[1];
      samp[1] = -last_samp[0];
   }

   last_samp[0] = samp[0];
   last_samp[1] = samp[1];
}

void monitor_modulator_bpsk_draw(monitor_modulator_t* mm, int sym)
{
   int8_t pulse_buffer[mm->params.spany];

   pulse_shaper_advance(mm->pulse_shaper_i, sym, pulse_buffer);
   
   for (int i = 0; i < mm->params.resy; i++)
   {
      mixer_mix(mm->mixer, pulse_buffer[i], 0, &((int8_t*)mm->surface->pixels)[i * mm->params.resx], NULL);

      for (int j = 0; j < mm->params.resx; j++)
      {
         ((uint8_t*)mm->surface->pixels)[i * mm->params.resx + j] += 128;
      }
   }

   monitor_modulator_display(mm);
}

void monitor_modulator_qpsk_draw(monitor_modulator_t* mm, int isym, int qsym)
{
   int8_t pulse_buffer_i[mm->params.spany];
   int8_t pulse_buffer_q[mm->params.spany];

   pulse_shaper_advance(mm->pulse_shaper_i, isym, pulse_buffer_i);
   pulse_shaper_advance(mm->pulse_shaper_q, qsym, pulse_buffer_q);
   
   for (int i = 0; i < mm->params.resy; i++)
   {
      mixer_mix(mm->mixer, pulse_buffer_i[i], pulse_buffer_q[i], &((int8_t*)mm->surface->pixels)[i * mm->params.resx], NULL);

      for (int j = 0; j < mm->params.resx; j++)
      {
         ((uint8_t*)mm->surface->pixels)[i * mm->params.resx + j] += 128;
      }
   }

   monitor_modulator_display(mm);
}

void monitor_modulator_transmit(monitor_modulator_t* mm, int data)
{
   int sym;
   int qpsk_sym[2];

   switch (mm->params.mod_mode)
   {
      case MOD_MODE_AM: break;
      case MOD_MODE_OOK: break;
      case MOD_MODE_BPSK:
         sym = monitor_modulator_bpsk_map(data);
         monitor_modulator_bpsk_draw(mm, sym);
         break;
      case MOD_MODE_QPSK: 
         mm->data_buffer[mm->data_buffer_count++] = data;
         if (mm->data_buffer_count == 2)
         {
            monitor_modulator_qpsk_map(mm->data_buffer, qpsk_sym);
            monitor_modulator_qpsk_draw(mm, qpsk_sym[0], qpsk_sym[1]);
            mm->data_buffer_count = 0;
         }
         break;
      case MOD_MODE_DQPSK: 
         mm->data_buffer[mm->data_buffer_count++] = data;
         if (mm->data_buffer_count == 2)
         {
            monitor_modulator_dqpsk_map(mm->data_buffer, qpsk_sym);
            monitor_modulator_qpsk_draw(mm, qpsk_sym[0], qpsk_sym[1]);
            mm->data_buffer_count = 0;
         }
         break;
   }

   SDL_Delay(200);
}

void monitor_modulator_transmit_byte(monitor_modulator_t* mm, unsigned char data)
{
   for (int i = 0; i < 8; i++)
   {
      monitor_modulator_transmit(mm, data & (1<<i));
   }
}
