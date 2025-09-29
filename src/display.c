#include <stdint.h>
#include <stdio.h>

#include "display.h"

void display_init(Display *display, char *title, uint8_t scale)
{
  printf("Display initialization\n");
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    printf("Errore SDL_Init: %s\n", SDL_GetError());
    return;
  }

  display->window = SDL_CreateWindow
  (
    title,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    GBA_WIDTH*scale, GBA_HEIGHT*scale,
    SDL_WINDOW_SHOWN
  );
  display->renderer = SDL_CreateRenderer(display->window, -1, 
    SDL_RENDERER_ACCELERATED);
  display->texture = SDL_CreateTexture
  (
    display->renderer,
    SDL_PIXELFORMAT_RGB565,
    SDL_TEXTUREACCESS_STREAMING,
    GBA_WIDTH, GBA_HEIGHT
  );

  pthread_mutex_init(&display->fb_mutex, NULL);
  pthread_cond_init(&display->fb_cond, NULL);
  display->frame_ready = 0;
  display->running = 1;
}



void display_update(Display* display) {
  pthread_mutex_lock(&display->fb_mutex);
  if (display->frame_ready) {
    SDL_UpdateTexture(display->texture, NULL, display->framebuffer, 
      GBA_WIDTH * sizeof(uint16_t));
    SDL_RenderClear(display->renderer);
    SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
    SDL_RenderPresent(display->renderer);
    display->frame_ready = 0;
  }
  pthread_mutex_unlock(&display->fb_mutex);
}
