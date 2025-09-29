#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <pthread.h>

#include "emulator.h"
#include "cartridge.h"
#include "cpu.h"

#include "instructions.h"
#include "bus.h"
#include "bios.h"

#include "display.h"



static emu_context ctx;
static Display display;

emu_context *emu_get_context()
{
  return &ctx;
}

pthread_barrier_t barrier;
bool running = true;
bool update = false;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

void *test(void *arg)
{
  SDL_Init(0);
  SDL_Window* w = SDL_CreateWindow
  (
    "Test Finestra",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    640, 480,
    SDL_WINDOW_SHOWN
  );

  SDL_Renderer* r = SDL_CreateRenderer(w, -1, 0);

  SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
  SDL_RenderClear(r);
  SDL_RenderPresent(r);

  pthread_barrier_wait(&barrier);

  while (running)
  {
    SDL_Delay(20);
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
        pthread_cond_signal(&cv);
      }
    }

  }

  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(w);
  SDL_Quit();
}


int emu_run(int argc, char **argv)
{
  //display_init(&display, "GBA Emulator", 3);
  //display_update(&display);
  pthread_t sdl_thread;
  pthread_barrier_init(&barrier, NULL, 2);
  pthread_create(&sdl_thread, NULL, test, NULL);

  cpu_init();
  pthread_barrier_wait(&barrier);

  ctx.running = true;
  ctx.paused = false;
  ctx.ticks = 0;

  //load_bios("../bios/gba_bios.bin");
  load_cartridge("../roms/arm.gba");
  //load_cartridge("../roms/thumb.gba");
  //load_cartridge("../roms/memory.gba");

  
  while (ctx.running)
  {

    if (!cpu_step())
    {
      printf("CPU stopped!\n");
      cpu_print_failed_test();
      pthread_join(sdl_thread, NULL);
      return -3;
    }

    // 133
    // 636
    // 560
    // 124
    //if (ctx.ticks++ == 1000)
    //  break;
    
  }


  dealloc_cartridge();
  pthread_join(sdl_thread, NULL);
  pthread_barrier_destroy(&barrier);

  return 0;
}



