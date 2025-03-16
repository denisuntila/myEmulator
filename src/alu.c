#include <stdio.h>
#include <stdlib.h>
#include "alu.h"

#define NO_IMPL {printf("ALU OP NOT YET IMPLEMENTED\n");};
#define REGS(id) *args->cpu->regs[id]


void alu_and(alu_args *args)
{
  uint32_t result = REGS(args->Rn) & args->op2;
  REGS(args->Rd) = result;

  if (!args->set_condition_codes)
    return;

  printf("SET CONDITIONS\n");
}


void alu_eor(alu_args *args)
{
  uint32_t result = REGS(args->Rn) ^ args->op2;
  REGS(args->Rd) = result;

  if (!args->set_condition_codes)
    return;

  printf("SET CONDITIONS\n");
}


void alu_sub(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  uint32_t b = args->op2;
  uint32_t result = a - b;
  REGS(args->Rd) = result;
    
  if (!args->set_condition_codes)
    return;
  
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    (((a >= b)) << 29);

  args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
    (((int32_t)((a ^ b) & (result ^ a)) < 0) << 28);
  
}


void alu_rsb(alu_args *args)
{
  uint32_t b = REGS(args->Rn);
  uint32_t a = args->op2;
  uint32_t result = a - b;
  REGS(args->Rd) = result;
    
  if (!args->set_condition_codes)
    return;
  
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    (((a >= b)) << 29);

  args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
    (((int32_t)((a ^ b) & (result ^ a)) < 0) << 28);
}


void alu_add(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  uint32_t b = args->op2;
  uint32_t result = a + b;
  REGS(args->Rd) = result;
    
  if (!args->set_condition_codes)
    return;
  
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    (((result < a)) << 29);
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
    (((int32_t)((a ^ result) & (b ^ result)) < 0) << 28);
}


void alu_adc(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  //uint8_t carry_in = (args->cpu->CPSR >> 29) & 0x1;
  uint32_t b = args->op2 + ((args->cpu->CPSR >> 29) & 0x1);
  uint32_t result = a + b;
  REGS(args->Rd) = result;
    
  if (!args->set_condition_codes)
    return;
  
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
  //args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
  //  (((result < a) || (carry_in && result == a)) << 29);
  args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    (((result < a)) << 29);

  args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
    (((int32_t)((a ^ result) & (b ^ result)) < 0) << 28);
}


void alu_sbc(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  //uint8_t carry_in = (args->cpu->CPSR >> 29) & 0x1;
  uint32_t b = args->op2 + 1 - ((args->cpu->CPSR >> 29) & 0x1);
  uint32_t result = a - b;
  //uint32_t result = a - args->op2 + carry_in - 1;
  REGS(args->Rd) = result;
    
  if (!args->set_condition_codes)
    return;
  
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
  //args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
  //  (((a >= (args->op2 + (1 - carry_in)))) << 29);
  args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    (((a >= b)) << 29);
  
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
    (((int32_t)((a ^ b) & (result ^ a)) < 0) << 28);
}


void alu_rsc(alu_args *args)
{
  uint32_t b = REGS(args->Rn) + 1 - ((args->cpu->CPSR >> 29) & 0x1);
  uint32_t a = args->op2;
  uint32_t result = a - b;
  REGS(args->Rd) = result;
    
  if (!args->set_condition_codes)
    return;
  
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  

  args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    (((a >= b)) << 29);
  
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
    (((int32_t)((a ^ b) & (result ^ a)) < 0) << 28);
}


void alu_tst(alu_args *args)
{
  uint32_t result = REGS(args->Rn) & args->op2;
  REGS(args->Rd) = result;

  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
}


void alu_teq(alu_args *args)
{
  uint32_t result = REGS(args->Rn) ^ args->op2;
  REGS(args->Rd) = result;

  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
}


void alu_cmp(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  uint32_t b = args->op2;
  uint32_t result = a - b;
  
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    (((a >= b)) << 29);

  args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
    (((int32_t)((a ^ b) & (result ^ a)) < 0) << 28);
  
}


void alu_cmn(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  uint32_t b = args->op2;
  uint32_t result = a + b;
  
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    (((result < a)) << 29);
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
    (((int32_t)((a ^ result) & (b ^ result)) < 0) << 28);
}


void alu_orr(alu_args *args)
{
  REGS(args->Rd) = REGS(args->Rn) | args->op2;
  if (!args->set_condition_codes)
    return;

  //uint32_t cpsr = args->cpu->CPSR;

  //cpsr &= 0x0FFFFFFF;
  // N flag
  //cpsr |= (REGS(args->Rd) & 0x80000000);
  // Z flag
  //cpsr |= ((REGS(args->Rd) == 0) ? 0x40000000 : 0x00000000);
  // C flag (carry)
  // not yet implemented
  printf("SET CONDITION FLAG\n");
  exit(-5);
  
  // V flag (overflow) (Not affected since is a logical operation)
  
  
}


void alu_mov(alu_args *args)
{
  REGS(args->Rd) = args->op2;

  if (!args->set_condition_codes)
    return;
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (args->op2 & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((args->op2 == 0) << 30);
  
  
}


void alu_bic(alu_args *args)
{
  uint32_t result = REGS(args->Rn) & (~args->op2);
  REGS(args->Rd) = result;

  if (!args->set_condition_codes)
    return;

  printf("SET CONDITIONS\n");
}


void alu_mvn(alu_args *args)
{
  REGS(args->Rd) = ~args->op2;

  if (!args->set_condition_codes)
    return;

  printf("SET CONDITIONS\n");
  // Modify cpsr flags
  //args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
  //  (args->op2 & 0x80000000);

  //args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
  //  ((args->op2 == 0) << 30);
  
}