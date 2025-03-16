#ifndef HH_CPU_HH
#define HH_CPU_HH

#include <stdint.h>
#include <stdbool.h>


//typedef struct
//{
//  // both arm and thumb
//  uint32_t R0;
//  uint32_t R1;
//  uint32_t R2;
//  uint32_t R3;
//  uint32_t R4;
//  uint32_t R5;
//  uint32_t R6;
//  uint32_t R7;
//
//  // only arm7tdmi
//  uint32_t R8;
//  uint32_t R9;
//  uint32_t R10;
//  uint32_t R11;
//  uint32_t R12;
//
//  uint32_t SP;
//  uint32_t LR;
//  uint32_t PC;
//
//  uint32_t CPSR;
//  uint32_t SPSR;
//} register_file;

typedef struct cpu_context
{
  //register_file regs;

  //uint32_t regs_sys_usr[16];
  //uint32_t regs_fiq[16];
  //uint32_t regs_svc[16];
  //uint32_t regs_abt[16];
  //uint32_t regs_irq[16];
  //uint32_t regs_und[16];

  uint32_t regs_sys_usr[16];

  uint32_t *regs[16];


  uint32_t CPSR;
  uint32_t SPSR;

  //uint32_t current_instruction;
  
  uint32_t fetched_instruction;
  uint32_t decoded_instruction;
  uint32_t instruction_to_exec;

  uint16_t thumb_fetch;
  uint16_t thumb_decode;
  uint16_t thumb_exec;

  void (*function)(struct cpu_context *);
  void (*thumb_function)(struct cpu_context *);
} cpu_context;

void cpu_init();
bool cpu_step();

void cpu_print_failed_test();

#endif