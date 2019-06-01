#ifndef _MONITOR_MODULATOR_H_
#define _MONITOR_MODULATOR_H_

typedef enum
{
   MOD_MODE_ERR,
   MOD_MODE_AM,
   MOD_MODE_OOK,
   MOD_MODE_BPSK,
   MOD_MODE_QPSK,
   MOD_MODE_DQPSK
} mod_mode_t;

typedef struct
{
   int resx;
   int resy;
   int spanx;
   int spany;
   double fc;
   double fs;
   mod_mode_t mod_mode;
} monitor_modulator_params_t;

typedef struct
{
   monitor_modulator_params_t params;
   SDL_Window* window;
   SDL_Renderer* renderer;
   SDL_Texture* texture;
   SDL_Surface* surface;
   pulse_shaper_t* pulse_shaper_i;
   pulse_shaper_t* pulse_shaper_q;
   mixer_t* mixer;
   float gain;
   int data_buffer[2]; // buffers i and q samp for QPSK only
   int data_buffer_count;
   int8_t* col_buffer_i;
   int8_t* col_buffer_q;
   int col_buffer_pos;
} monitor_modulator_t;

void monitor_modulator_init(
      monitor_modulator_t* mm, 
      monitor_modulator_params_t params);
void monitor_modulator_destroy(monitor_modulator_t* mm);

void monitor_modulator_sdl_init(monitor_modulator_t* mm);

void monitor_modulator_transmit(monitor_modulator_t* mm, int data);
void monitor_modulator_transmit_byte(monitor_modulator_t* mm, uint8_t data);
void monitor_modulator_display(monitor_modulator_t* mm);

int monitor_modulator_ook_map(int data);
int monitor_modulator_bpsk_map(int data);
void monitor_modulator_qpsk_map(int* data, int* samp);
void monitor_modulator_dqpsk_map(int* data, int* samp);

void monitor_modulator_am_draw(monitor_modulator_t* mm, uint8_t samp);
void monitor_modulator_ook_draw(monitor_modulator_t* mm, int sym);
void monitor_modulator_bpsk_draw(monitor_modulator_t* mm, int sym);
void monitor_modulator_qpsk_draw(monitor_modulator_t* mm, int isym, int qsym);

#endif
