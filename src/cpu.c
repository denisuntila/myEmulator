#include <stdio.h>
#include <stdbool.h>

#include "cpu.h"
#include "instructions.h"

#include "bus.h"


#define TEST(add)                                                                   \
{                                                                                   \
  uint32_t address = add;                                                           \
  cpu.instruction_to_exec = bus_read_word(address);                                 \
  printf("Instruction: 0x%08x\n", cpu.instruction_to_exec);                         \
  void (*function)(cpu_context *) = decode_instruction(cpu.instruction_to_exec);    \
  printf("0x%08x:\t", address);                                                     \
  function(&cpu);                                                                   \
  cpu.regs[15] += 4;                                                                \
}

#define REGS(id) *cpu.regs[id]


// defining the nop instruction as mov r0, r0
#define NOP 0xe1a00000
#define THUMB_NOP 0x46C0
#define PC *cpu.regs[15]
#define LR *cpu.regs[14]
#define SP *cpu.regs[13]

static cpu_context cpu;

bool cpu_arm_step();
bool cpu_thumb_step();

void cpu_init()
{
  printf("CPU Initialization\n");
  // Set the program counter to 0
  //cpu.regs[15] = 0x07FFFFFC;
  //cpu.regs = cpu.regs_sys_usr;
  for (int i = 0; i < 16; ++i)
    cpu.regs[i] = &cpu.regs_sys_usr[i];

  printf("DEBUG\n");
  PC = 0x08000000;
  //PC = 0x00000000;
  SP = 0x03007f00;
  cpu.instruction_to_exec = NOP;
  cpu.decoded_instruction = NOP;
  cpu.fetched_instruction = NOP;

  cpu.thumb_fetch   = THUMB_NOP;
  cpu.thumb_decode  = THUMB_NOP;
  cpu.thumb_exec    = THUMB_NOP;

  cpu.function = decode_instruction(cpu.instruction_to_exec);
  cpu.thumb_function = thumb_decode_instruction(cpu.thumb_exec);
}

bool cpu_step()
{
  bool retval;
  printf("PC = 0x%08x\n", PC);
  if (((cpu.CPSR >> 5) & 0x01) == 1)
    retval = cpu_thumb_step();
  else
    retval = cpu_arm_step();

  return retval;
}


void cpu_print_failed_test()
{
  printf("Failed test = %d\n", cpu.regs[12]);
}


bool cpu_arm_step_old()
{
  cpu.fetched_instruction = bus_read_word(PC);
  printf("Fetched instruction: 0x%08x\n", cpu.fetched_instruction);
  //PC += 4;

  //void (*func)(cpu_context *) = decode_instruction(cpu.decoded_instruction);
  printf("Decoded instruction = 0x%08x\n", cpu.decoded_instruction);

  uint32_t old_pc = PC;
  if (verify_condition(&cpu))
    cpu.function(&cpu);
  else
    printf("NOT EXECUTED DUE TO UNSATISFIED CONDITION\n");
  printf("Executed instruction: 0x%08x\n", cpu.instruction_to_exec);
  // If an instruction changed the pc, then flush the pipeline
  if (old_pc != PC)
    flush(&cpu);


  //cpu.function = func;
  cpu.function = decode_instruction(cpu.decoded_instruction);
  //printf("Cond: %s\n", verify_condition(&cpu) ? "TRUE" : "FALSE");
  cpu.instruction_to_exec = cpu.decoded_instruction;
  cpu.decoded_instruction = cpu.fetched_instruction;

  printf("R0 = 0x%08x\n", cpu.regs[0]);
  printf("R1 = 0x%08x\n", cpu.regs[1]);
  printf("R2 = 0x%08x\n", cpu.regs[2]);
  printf("LR = 0x%08x\n", LR);
  printf("SP = 0x%08x\n", SP);
  printf("nzcv = 0b%04b\n", cpu.CPSR >> 28);

  PC += 4;

  printf("\n");

  // TEMPORARY 'cause I still haven't implemented the display
  if (0x08001d4c == PC)
    return false;

  return true;
}


bool cpu_arm_step()
{
  cpu.fetched_instruction = bus_read_word(PC);
  printf("Fetched instruction: 0x%08x\n", cpu.fetched_instruction);
  printf("Executing instruction: 0x%08x\n", cpu.instruction_to_exec);

  uint32_t old_pc = PC;
  if (verify_condition(&cpu))
    cpu.function(&cpu);
  else
    printf("NOT EXECUTED DUE TO UNSATISFIED CONDITION\n");
  
  // If an instruction changed the pc, then flush the pipeline
  if (old_pc != PC)
    flush(&cpu);

  cpu.function = decode_instruction(cpu.fetched_instruction);
  cpu.instruction_to_exec = cpu.fetched_instruction;

  printf("R0 = 0x%08x\n", REGS(0));
  printf("R1 = 0x%08x\n", REGS(1));
  printf("R2 = 0x%08x\n", REGS(2));
  printf("LR = 0x%08x\n", LR);
  printf("SP = 0x%08x\n", SP);
  printf("nzcv = 0b%04b\n", cpu.CPSR >> 28);

  PC += 4;

  printf("\n");

  // TEMPORARY 'cause I still haven't implemented the display
  if (0x08001d4c == PC)
    return false;

  return true;
}


bool cpu_thumb_step()
{
  cpu.thumb_fetch = bus_read_halfword(PC);
  printf("Fetched THUMB instruction: 0x%04x\n", cpu.thumb_fetch);
  printf("Decoded THUMB instruction: 0x%04x\n", cpu.thumb_decode);

  uint32_t old_pc = PC;
  cpu.thumb_function(&cpu);
  printf("Executed THUMB instruction: 0x%04x\n", cpu.thumb_exec);

  if (old_pc != PC)
    thumb_flush(&cpu);

  
  cpu.thumb_function = thumb_decode_instruction(cpu.thumb_decode);
  cpu.thumb_exec = cpu.thumb_decode;
  cpu.thumb_decode = cpu.thumb_fetch;

  printf("R0 = 0x%08x\n", REGS(0));
  printf("R1 = 0x%08x\n", REGS(1));
  printf("R12 = 0x%08x\n", REGS(12));
  printf("LR = 0x%08x\n", LR);
  printf("SP = 0x%08x\n", SP);
  printf("nzcv = 0b%04b\n", cpu.CPSR >> 28);

  PC += 2;
  printf("\n");
  return true;
}

