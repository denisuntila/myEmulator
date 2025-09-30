#ifndef HH_DISPLAY_HH
#define HH_DISPLAY_HH

#include <stdint.h>
#include <SDL3/SDL.h>

#define GBA_WIDTH   240
#define GBA_HEIGHT  160


typedef struct Display
{
  SDL_Thread *thread;
  SDL_Window *window;
  SDL_Mutex *mutex;
  uint8_t running;
  uint8_t refresh;

  // Just for now, before I implement frame buffer
  struct
  {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  } color;
  
} Display;

void display_init(Display *display, char *title, uint8_t scale);
void display_update(Display *display, int i);
void display_destroy(Display *display);



#endif