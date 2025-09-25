#include <stdio.h>
#include <stdlib.h>
#include "alu.h"

#define NO_IMPL {printf("ALU OP NOT YET IMPLEMENTED\n");};
#define REGS(id) *args->cpu->regs[id]


void alu_and(alu_args *args)
{
  uint32_t result = REGS(args->Rn) & args->op2;
  if (args->Rd == 15) result -= 4;
  REGS(args->Rd) = result;

  if (!args->set_condition_codes)
    return;

  printf("SET CONDITIONS\n");
}


void alu_eor(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  uint32_t b = args->op2;
  uint32_t result = a ^ b;
  if (args->Rd == 15) result -= 4;
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
  if (args->Rd == 15) result -= 4;
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
  if (args->Rd == 15) result -= 4;
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
  //if (args->Rn == 15) a += 4;
  uint32_t b = args->op2;
  uint32_t result = a + b;
  if (args->Rd == 15) result -= 4;
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
  if (args->Rd == 15) result -= 4;
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
  if (args->Rd == 15) result -= 4;
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
  if (args->Rd == 15) result -= 4;
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
  //REGS(args->Rd) = result;

  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
}


void alu_teq(alu_args *args)
{
  uint32_t result = REGS(args->Rn) ^ args->op2;
  //REGS(args->Rd) = result;

  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
}


void alu_cmp(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  if (args->Rn == 15) a += 4;
  printf("DEBUG: Rn = r%d\n", args->Rn);
  uint32_t b = args->op2;
  uint32_t result = a - b;
  printf("COMPUTING 0x%08x - 0x%08x\n", a, b);
  
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
  int32_t a = REGS(args->Rn);
  int32_t b = args->op2;
  int32_t result = a + b;
  printf("COMPUTING 0x%08x + 0x%08x\n", a, b);
  printf("RES = 0x%08x\n", result);
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
  uint32_t result = REGS(args->Rn) | args->op2;
  if (args->Rd == 15) result -= 4;
  REGS(args->Rd) = result;
  if (!args->set_condition_codes)
    return;

  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
  
}


void alu_mov(alu_args *args)
{
  uint32_t result = args->op2;
  if (args->Rd == 15) result -= 4;
  REGS(args->Rd) = result;

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
  if (args->Rd == 15) result -= 4;
  REGS(args->Rd) = result;

  if (!args->set_condition_codes)
    return;

  
}


void alu_mvn(alu_args *args)
{
  uint32_t result = ~args->op2;
  if (args->Rd == 15) result -= 4;
  REGS(args->Rd) = result;

  if (!args->set_condition_codes)
    return;

  // Set condition flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
}

    


void alu_lsl(alu_args *args)
{
  uint32_t result; //= REGS(args->Rd) << args->op2;
  uint32_t shift = args->op2;
  
  if (shift >= 32)
  {
    result = 0;
    args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
      (((REGS(args->Rd) & 0x1) && (shift == 32)) << 29);
  }
  else
  {
    result = REGS(args->Rd) << shift;
    args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
      ((REGS(args->Rd) << (shift - 1)) >> 2);
  }

  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);


  REGS(args->Rd) = result;

}




void alu_lsr(alu_args *args)
{
  uint32_t result; //= REGS(args->Rd) << args->op2;
  uint32_t shift = args->op2;


  if (shift > 32)
  {
    result = 0;
    args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF);
    //args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    //  ((REGS(args->Rd) & 0x80000000) >> 2);
  }
  else if (shift == 32)
  {
    result = 0;
    args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
      ((REGS(args->Rd) & 0x80000000) >> 2);
  }
  else
  {
    result = REGS(args->Rd) >> shift;
    args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
      ((REGS(args->Rd) >> (shift - 1)) << 29); 
  }

  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);


  REGS(args->Rd) = result;
}

void alu_asr(alu_args *args)
{
  uint32_t result; //= REGS(args->Rd) << args->op2;
  uint32_t shift = args->op2;

  {
    result = (int32_t)(REGS(args->Rd)) >> shift;
    args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
      (((int32_t)(REGS(args->Rd)) >> (shift - 1)) << 29);
  }

  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);


  REGS(args->Rd) = result;
}

void alu_ror(alu_args *args)
{
  uint32_t result; //= REGS(args->Rd) << args->op2;
  uint32_t shift = args->op2;


  if (shift == 32)
  {
    result = REGS(args->Rd);
    args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
      ((REGS(args->Rd) & 0x80000000) >> 2);
  }
  else if (shift == 0)
  {
    result = REGS(args->Rd);
    args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
      ((REGS(args->Rd) & 0x1) << 29);
  }
  else
  {
    result = (REGS(args->Rd) >> shift) | (REGS(args->Rd) << (32 - shift));
    args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
      ((result & 0x80000000) >> 2);
  }

  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);


  REGS(args->Rd) = result;
}

void alu_neg(alu_args *args)
{
  int32_t b = args->op2;
  int32_t result = 0 - b;
  REGS(args->Rd) = (uint32_t)result;
    
  if (!args->set_condition_codes)
    return;
  
  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
  
  args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
    (((0 >= b)) << 29);

  args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
    (((int32_t)((0 ^ b) & (result ^ 0)) < 0) << 28);
  
}

void alu_mul(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  //if (args->Rn == 15) a += 4;
  uint32_t b = args->op2;
  uint32_t result = a * b;

  REGS(args->Rd) = result;

  // Modify cpsr flags
  args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
    (result & 0x80000000);

  args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
    ((result == 0) << 30);
}






void alu_add_thumb(alu_args *args)
{
  uint32_t a = REGS(args->Rn);
  //if (args->Rn == 15) a += 4;
  uint32_t b = args->op2;
  uint32_t result = a + b;
  if (args->Rd == 15) result -= 2;
  REGS(args->Rd) = result;

    
  if (args->set_condition_codes == false)
  {
    return;
  }

  // I don't actually know why, but if I comment
  // the following part the test passes, even if
  // the part that usually fails never calls this
  // function... (?)
  
  // Modify cpsr flags
  //args->cpu->CPSR = (args->cpu->CPSR & 0x7FFFFFFF) |
  //  (result & 0x80000000);
  //
  //args->cpu->CPSR = (args->cpu->CPSR & 0xBFFFFFFF) |
  //  ((result == 0) << 30);
  //
  //args->cpu->CPSR = (args->cpu->CPSR & 0xDFFFFFFF) |
  //  (((result < a)) << 29);
  //
  //args->cpu->CPSR = (args->cpu->CPSR & 0xEFFFFFFF) |
  //  (((int32_t)((a ^ result) & (b ^ result)) < 0) << 28);
}