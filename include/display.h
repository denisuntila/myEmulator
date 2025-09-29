#ifndef HH_DISPLAY_HH
#define HH_DISPLAY_HH

#include <stdint.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define GBA_WIDTH   240
#define GBA_HEIGHT  160


typedef struct Display
{
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  uint16_t framebuffer[GBA_WIDTH * GBA_HEIGHT]; // pixel buffer
  pthread_mutex_t fb_mutex;
  pthread_cond_t fb_cond;
  int frame_ready;
  int running;
  pthread_barrier_t barrier;
} Display;

void display_init(Display *display, char *title, uint8_t scale);
void display_update(Display *display);
void display_destroy(Display *display);



#endif