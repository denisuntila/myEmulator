#include <stdint.h>
#include <stdio.h>

#include "display.h"

int window_thread(void *data) {
    Display *display = (Display *)data;
    display->window = SDL_CreateWindow(
      "SDL3 Finestra con Renderer",
      800,
      600,
      0
    );
    if (!display->window) {
      printf("Errore SDL_CreateWindow: %s\n", SDL_GetError());
      return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(display->window, NULL);
    if (!renderer) {
      printf("Errore SDL_CreateRenderer: %s\n", SDL_GetError());
      SDL_DestroyWindow(display->window);
      return -1;
    }

    SDL_SetRenderDrawColor(renderer, display->color.red, display->color.green, display->color.blue, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Ciclo eventi/rendering
    while (display->running) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
          display->running = 0;
        }
      }

      SDL_LockMutex(display->mutex);
      if (display->refresh)
      {
        SDL_SetRenderDrawColor(renderer, display->color.red, display->color.green, display->color.blue, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        display->refresh = 0;
      }
      SDL_UnlockMutex(display->mutex);


      SDL_Delay(16); // ~60 fps
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(display->window);
    return 0;
}

void display_init(Display *display, char *title, uint8_t scale)
{
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  display->mutex = SDL_CreateMutex();
  display->running = 1;
  display->refresh = 0;
  display->thread = SDL_CreateThread(window_thread, "WindowThread", display);

}

void display_update(Display *display, int i)
{
  uint8_t colors[][3] = 
  {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {255, 255, 255}
  };

  SDL_LockMutex(display->mutex);
  display->color.red = colors[i][0];
  display->color.green = colors[i][1];
  display->color.blue = colors[i][2];
  display->refresh = 1;
  SDL_UnlockMutex(display->mutex);
}


void display_destroy(Display *display)
{
  display->running = 0;
}
