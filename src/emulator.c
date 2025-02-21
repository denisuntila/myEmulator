#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "emulator.h"
#include "cartridge.h"
#include "cpu.h"

#include "instructions.h"
#include "bus.h"
#include "bios.h"



static emu_context ctx;

emu_context *emu_get_context()
{
  return &ctx;
}


void delay(uint32_t ms)
{
  SDL_Delay(ms);
}



int emu_run(int argc, char **argv)
{
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  cpu_init();

  ctx.running = true;
  ctx.paused = false;
  ctx.ticks = 0;

  load_bios("../bios/gba_bios.bin");
  load_cartridge("../roms/arm.gba");

  for (int i = 0; i < 35; ++i)
  {
    cpu_step();
  }
  
  while (ctx.running)
  {
    if (ctx.paused)
    {
      delay(10);
      continue;
    }
    
    if (!cpu_step())
    {
      printf("CPU stopped!\n");
      return -3;
    }

    ctx.ticks++;
  }

  dealloc_cartridge();

  return 0;
}



