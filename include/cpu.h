#ifndef HH_CPU_HH
#define HH_CPU_HH

#include <stdint.h>


typedef struct
{
  // both arm and thumb
  uint32_t R0;
  uint32_t R1;
  uint32_t R2;
  uint32_t R3;
  uint32_t R4;
  uint32_t R5;
  uint32_t R6;
  uint32_t R7;

  // only arm7tdmi
  uint32_t R8;
  uint32_t R9;
  uint32_t R10;
  uint32_t R11;
  uint32_t R12;

  uint32_t SP;
  uint32_t LR;
  uint32_t PC;

  uint32_t CPSR;
  uint32_t SPSR;
} register_file;

typedef struct
{
  register_file regs;

  uint32_t fetched_data;
  uint32_t mem_dest;
  bool dest_is_mem;
} cpu_context;

void cpu_init();
bool cpu_step();

#endif