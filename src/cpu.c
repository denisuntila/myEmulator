#include <stdio.h>
#include <stdbool.h>

#include "cpu.h"
#include "instructions.h"

#include "bus.h"


#define TEST(add)                                                                   \
{                                                                                   \
  uint32_t address = add;                                                           \
  cpu.current_instruction = bus_read_word(address);                                 \
  void (*function)(cpu_context *) = decode_instruction(cpu.current_instruction);    \
  printf("0x%08x:\t0x%08x:\t", address, cpu.current_instruction);                   \
  function(&cpu);                                                                   \
  cpu.regs[15] += 4;                                                                \
}


static cpu_context cpu;

void cpu_init()
{
  printf("CPU Initialization\n");
  // Set the program counter to 0
  cpu.regs[15] = 0;
}

bool cpu_step()
{
  //printf("CPU not implemented yet\n");
  //uint32_t address = 0x08000000;
  //cpu.current_instruction = bus_read_word(address);
  //void (*function)(cpu_context *) = decode_instruction(cpu.current_instruction);
  //printf("0x%08x:\t", address);
  //function(&cpu);

  TEST(0x08000000);
  TEST(0x080000C0);
  TEST(0x08001EC8);
  TEST(0x08001ED0);
  TEST(0x08001ED4);
  TEST(0x08001ED8);
  TEST(0x08001EDC);
  TEST(0x08001EE0);
  TEST(0x08001EE4);
  TEST(0x08001EE8);
  TEST(0x08001EEC);
  TEST(0x08001EF0);
  TEST(0x08001EF4);
  TEST(0x08001EF8);
  TEST(0x08001EFC);
  TEST(0x08001F00);
  TEST(0x08001F04);
  TEST(0x08001F08);
  TEST(0x08001F0C);
  TEST(0x08001F04);
  TEST(0x08001F08);
  TEST(0x08001F0C);
  TEST(0x08001F10);
  TEST(0x08001F14);
  TEST(0x08001F18);
  TEST(0x08001F1C);
  TEST(0x08001F20);
  TEST(0x08001F24);
  

  

  //for (uint32_t i = 0x080000C0; i < 0x08000140; i += 4)
  //{
  //  cpu.current_instruction = bus_read_word(i);
  //  function = decode_instruction(cpu.current_instruction);
  //  function(&cpu);
  //}


  return false;
}