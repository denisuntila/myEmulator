#include <stdio.h>
#include <stdbool.h>

#include <SDL3/SDL.h>

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


int emu_run(int argc, char **argv)
{
  cpu_init();

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
      return -3;
    }

    // 133
    // 636
    // 560
    // 124
    if (ctx.ticks++ == 1)
      break;
    
  }

  display_init(&display, "Prova", 3);
  for (int i = 0; i < 5; ++i)
  {
    display_update(&display, i);
    SDL_Delay(1000);
  }

  SDL_Delay(2000);
  display_destroy(&display);
  
  SDL_WaitThread(display.thread, NULL);

  dealloc_cartridge();

  return 0;
}



