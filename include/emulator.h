#ifndef HH_EMULATOR_HH
#define HH_EMULATOR_HH


#include <stdint.h>


typedef struct
{
  bool paused;
  bool running;
  uint64_t ticks;
} emu_context;

int emu_run(int argc, char **argv);

emu_context *emu_get_context();


#endif