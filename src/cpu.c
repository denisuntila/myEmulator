#include <stdio.h>
#include <stdbool.h>

#include "cpu.h"
#include "instructions.h"

#include "bus.h"


#define TEST(add)                                                                   \
{                                                                                   \
  uint32_t address = add;                                                           \
  cpu.instruction_to_exec = bus_read_word(address);                                \
  printf("Instruction: 0x%08x\n", cpu.instruction_to_exec);   \
  void (*function)(cpu_context *) = decode_instruction(cpu.instruction_to_exec);    \
  printf("0x%08x:\t", address);                                                     \
  function(&cpu);                                                                   \
  cpu.regs[15] += 4;                                                                \
}

// defining the nop instruction as mov r0, r0
#define NOP 0xe1a00000
#define PC cpu.regs[15]
#define LR cpu.regs[14]
#define SP cpu.regs[13]

static cpu_context cpu;

void cpu_init()
{
  printf("CPU Initialization\n");
  // Set the program counter to 0
  //cpu.regs[15] = 0x07FFFFFC;
  PC = 0x08000000;
  //PC = 0x00000000;
  SP = 0x03007f00;
  cpu.instruction_to_exec = NOP;
  cpu.decoded_instruction = NOP;
  cpu.fetched_instruction = NOP;
  cpu.function = decode_instruction(cpu.instruction_to_exec);
}

bool cpu_step()
{
  printf("PC = 0x%08x\n", PC);
  cpu.fetched_instruction = bus_read_word(PC);
  printf("Fetched instruction: 0x%08x\n", cpu.fetched_instruction);

  //void (*func)(cpu_context *) = decode_instruction(cpu.decoded_instruction);
  printf("Decoded instruction = 0x%08x\n", cpu.decoded_instruction);

  uint32_t old_pc = PC;
  cpu.function(&cpu);
  printf("Executed instruction: 0x%08x\n", cpu.instruction_to_exec);
  // If an instruction changed the pc, then flush the pipeline
  if (old_pc != PC)
    flush(&cpu);

  //cpu.function = func;
  cpu.function = decode_instruction(cpu.decoded_instruction);
  printf("Cond: %s\n", verify_condition(&cpu) ? "TRUE" : "FALSE");
  cpu.instruction_to_exec = cpu.decoded_instruction;
  cpu.decoded_instruction = cpu.fetched_instruction;

  printf("R0 = 0x%08x\n", cpu.regs[0]);
  printf("R1 = 0x%08x\n", cpu.regs[1]);
  printf("R2 = 0x%08x\n", cpu.regs[2]);
  printf("LR = 0x%08x\n", LR);
  printf("SP = 0x%08x\n", SP);

  PC += 4;

  printf("\n");
  return false;
}