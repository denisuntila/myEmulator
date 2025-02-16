#include <stdio.h>
#include <stdbool.h>

#include "cpu.h"
#include "instructions.h"


cpu_context cpu;

void cpu_init()
{
  printf("CPU Initialization\n");
  cpu.current_instruction = 0xE12FFF16;
}

bool cpu_step()
{
  //printf("CPU not implemented yet\n");
  void (*function)(cpu_context *) = decode_instruction(cpu.current_instruction);

  function(&cpu);

  cpu.current_instruction = 0x0B0000EB;

  function = decode_instruction(cpu.current_instruction);

  function(&cpu);


  
  cpu.current_instruction = 0xE7312183;

  function = decode_instruction(cpu.current_instruction);

  function(&cpu);


  
  cpu.current_instruction = 0xE1020093;

  function = decode_instruction(cpu.current_instruction);

  function(&cpu);


  //cpu.current_instruction = 0xE0203291;
  cpu.current_instruction = 0xE0100291;

  function = decode_instruction(cpu.current_instruction);

  function(&cpu);

  
  cpu.current_instruction = 0xE0910392;

  function = decode_instruction(cpu.current_instruction);

  function(&cpu);

  

  cpu.current_instruction = 0xE1BFF0FF;

  function = decode_instruction(cpu.current_instruction);

  function(&cpu);



  return false;
}