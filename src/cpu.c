#include <stdio.h>
#include <stdbool.h>

#include "cpu.h"
#include "instructions.h"

#include "bus.h"


cpu_context cpu;

void cpu_init()
{
  printf("CPU Initialization\n");
  cpu.current_instruction = 0xea00002e;
}

bool cpu_step()
{
  //printf("CPU not implemented yet\n");
  void (*function)(cpu_context *) = decode_instruction(cpu.current_instruction);

  function(&cpu);

  

  for (uint32_t i = 0x080000C0; i < 0x08000140; i += 4)
  {
    cpu.current_instruction = bus_read_word(i);
    function = decode_instruction(cpu.current_instruction);
    function(&cpu);
  }


  return false;
}