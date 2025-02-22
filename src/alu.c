#include <stdio.h>
#include <stdlib.h>
#include "alu.h"

#define NO_IMPL {printf("ALU OP NOT IMPLEMENTED\n");};


void alu_and(alu_args *args)
{
  NO_IMPL
}


void alu_eor(alu_args *args)
{
  NO_IMPL
}


void alu_sub(alu_args *args)
{
  NO_IMPL
}


void alu_rsb(alu_args *args)
{
  NO_IMPL
}


void alu_add(alu_args *args)
{
  NO_IMPL
}


void alu_adc(alu_args *args)
{
  NO_IMPL
}


void alu_sbc(alu_args *args)
{
  NO_IMPL
}


void alu_rsc(alu_args *args)
{
  NO_IMPL
}


void alu_tst(alu_args *args)
{
  NO_IMPL
}


void alu_teq(alu_args *args)
{
  NO_IMPL
}


void alu_cmp(alu_args *args)
{
  NO_IMPL
}


void alu_cmn(alu_args *args)
{
  NO_IMPL
}


void alu_orr(alu_args *args)
{
  uint32_t op2;
  if (args->immediate)
  {
    op2 = (args->imm_format.nn >> args->imm_format.Is) | 
      (args->imm_format.nn << (32 - args->imm_format.Is));
  }
  else
  {
    uint32_t shift = 0x00000000;
    if (args->non_imm_format.shift_by_register)
    {
      shift = args->cpu->regs[args->non_imm_format.rs_is >> 1];
    }
    else
    {
      shift = args->non_imm_format.rs_is;
    }
    switch (args->non_imm_format.shift_type)
    {
    case LSL:
      op2 = args->cpu->regs[args->non_imm_format.Rm] << shift;
      break;

    case LSR:
      op2 = args->cpu->regs[args->non_imm_format.Rm] >> shift;
      break;

    case ASR:
      op2 = (int32_t)(args->cpu->regs[args->non_imm_format.Rm]) >> shift;
      break;
    
    case ROR:
      op2 = (args->cpu->regs[args->non_imm_format.Rm] >> shift) |
        (args->cpu->regs[args->non_imm_format.Rm] << (32 - shift));
      break;

    default:
      break;
    }
  }
  args->cpu->regs[args->Rd] = args->cpu->regs[args->Rn] | op2;
}


void alu_mov(alu_args *args)
{
  uint32_t op2;
  if (args->immediate)
  {
    op2 = (args->imm_format.nn >> args->imm_format.Is) | 
      (args->imm_format.nn << (32 - args->imm_format.Is));
  }
  else
  {
    uint32_t shift = 0x00000000;
    if (args->non_imm_format.shift_by_register)
    {
      shift = args->cpu->regs[args->non_imm_format.rs_is >> 1];
    }
    else
    {
      shift = args->non_imm_format.rs_is;
    }
    switch (args->non_imm_format.shift_type)
    {
    case LSL:
      op2 = args->cpu->regs[args->non_imm_format.Rm] << shift;
      break;

    case LSR:
      op2 = args->cpu->regs[args->non_imm_format.Rm] >> shift;
      break;

    case ASR:
      op2 = (int32_t)(args->cpu->regs[args->non_imm_format.Rm]) >> shift;
      break;
    
    case ROR:
      op2 = (args->cpu->regs[args->non_imm_format.Rm] >> shift) |
        (args->cpu->regs[args->non_imm_format.Rm] << (32 - shift));
      break;

    default:
      break;
    }
  }
  args->cpu->regs[args->Rd] = op2;
}


void alu_bic(alu_args *args)
{
  NO_IMPL
}


void alu_mvn(alu_args *args)
{
  NO_IMPL
}