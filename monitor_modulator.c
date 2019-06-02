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
   mm->gain = 1.0;

   ps_params.sps = mod_params.spany;
   ps_params.delay = 2;
   ps_params.beta = 0.5;
   ps_params.gain = 80;

   mix_params.fc = mod_params.fc;
   mix_params.fs = mod_params.fs;
   mix_params.upsample = mod_params.spanx;

   mm->col_buffer_pos = 0;
   mm->col_buffer_i = malloc(mod_params.spany);
   mm->col_buffer_q = malloc(mod_params.spany);
   memset(mm->col_buffer_i, 0, mod_params.spany);
   memset(mm->col_buffer_q, 0, mod_params.spany);

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

void monitor_modulator_destroy(monitor_modulator_t* mm)
{
   pulse_shaper_destroy(mm->pulse_shaper_i);
   pulse_shaper_destroy(mm->pulse_shaper_q);
   mixer_destroy(mm->mixer);
   free(mm->pulse_shaper_i);
   free(mm->pulse_shaper_q);
   free(mm->mixer);
   free(mm->col_buffer_i);
   free(mm->col_buffer_q);
}

void monitor_modulator_display(monitor_modulator_t* mm)
{
   mm->texture = SDL_CreateTextureFromSurface(mm->renderer, mm->surface);
   SDL_RenderCopy(mm->renderer, mm->texture, NULL, NULL);
   SDL_RenderPresent(mm->renderer);
   SDL_DestroyTexture(mm->texture);
   //SDL_Delay(8);
}

int monitor_modulator_bpsk_map(int data)
{
   if (data) return 1;
   else return -1;
}

void monitor_modulator_qpsk_map(int* data, int* samp)
{
   if (!data[0] && !data[1])
   {
      samp[0] = 1;
      samp[1] = 1;
   }
   else if (!data[0] && data[1])
   {
      samp[0] = -1;
      samp[1] = 1;
   }
   else if (data[0] && data[1])
   {
      samp[0] = -1;
      samp[1] = -1;
   }
   else if (data[0] && !data[1])
   {
      samp[0] = 1;
      samp[1] = -1;
   }
}

void monitor_modulator_dqpsk_map(int* data, int* samp)
{
   // todo: bit lazy, this, but we are outta time
   static int last_samp[2] = {1, 1};

   if (!data[0] && !data[1])
   {
      samp[0] = last_samp[0];
      samp[1] = last_samp[1];
   }
   else if (!data[0] && data[1])
   {
      samp[0] = last_samp[1];
      samp[1] = -last_samp[0];
   }
   else if (data[0] && data[1])
   {
      samp[0] = -last_samp[0];
      samp[1] = -last_samp[1];
   }
   else if (data[0] && !data[1])
   {
      samp[0] = -last_samp[1];
      samp[1] = last_samp[0];
   }

   last_samp[0] = samp[0];
   last_samp[1] = samp[1];
}

void monitor_modulator_draw(monitor_modulator_t* mm)
{
   for (int i = 0; i < mm->params.resy; i++)
   {
      mixer_mix_u(mm->mixer, mm->col_buffer_i[i], mm->col_buffer_q[i], 128 * mm->gain, &((uint8_t*)mm->surface->pixels)[i * mm->params.resx], NULL);
   }
}

void monitor_modulator_am_draw(monitor_modulator_t* mm, uint8_t samp)
{
   mm->col_buffer_i[mm->col_buffer_pos++] = (samp / 2) * mm->gain;

   if (mm->col_buffer_pos < mm->params.spany)
      return;

   monitor_modulator_draw(mm);
   monitor_modulator_display(mm);
   mm->col_buffer_pos = 0;
}

void monitor_modulator_bpsk_draw(monitor_modulator_t* mm, int sym)
{
   pulse_shaper_advance(mm->pulse_shaper_i, sym, mm->col_buffer_i);
   
   monitor_modulator_draw(mm);
   monitor_modulator_display(mm);
}

void monitor_modulator_qpsk_draw(monitor_modulator_t* mm, int isym, int qsym)
{
   pulse_shaper_advance(mm->pulse_shaper_i, isym, mm->col_buffer_i);
   pulse_shaper_advance(mm->pulse_shaper_q, qsym, mm->col_buffer_q);
   
   monitor_modulator_draw(mm);
   monitor_modulator_display(mm);
}

void monitor_modulator_transmit(monitor_modulator_t* mm, int data)
{
   int sym;
   int qpsk_sym[2];

   switch (mm->params.mod_mode)
   {
      case MOD_MODE_ERR: printf("Bad modulation scheme\n"); break;
      case MOD_MODE_AM: 
         monitor_modulator_am_draw(mm, data);
         break;
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
      default: break;
   }
}

void monitor_modulator_transmit_byte(monitor_modulator_t* mm, uint8_t data)
{
   if (mm->params.mod_mode == MOD_MODE_AM)
   {
      monitor_modulator_transmit(mm, data);
   }
   else
   {
      for (int i = 0; i < 8; i++)
      {
         monitor_modulator_transmit(mm, data & (128>>i));
      }
   }
}
