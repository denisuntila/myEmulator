#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "instructions.h"
#include "cpu.h"
#include "bus.h"


// defining the nop instruction as mov r0, r0
#define NOP 0xe1a00000

// define the condition states
#define COND_EQ 0x0
#define COND_NE 0x1
#define COND_HS 0x2
#define COND_LO 0x3
#define COND_MI 0x4
#define COND_PL 0x5
#define COND_VS 0x6
#define COND_VC 0x7
#define COND_HI 0x8
#define COND_LS 0x9
#define COND_GE 0xA
#define COND_LT 0xB
#define COND_GT 0xC
#define COND_LE 0xD
#define COND_AL 0xE
#define COND_NV 0xF


#define AMOD_DA 0x0
#define AMOD_IA 0x1
#define AMOD_DB 0x2
#define AMOD_IB 0x3


static const uint32_t instruction_type_format_masks[][2] = 
{
  {0x012FFF10, 0x0FFFFFF0},           // BE format
  {0x08000000, 0x0E000000},           // Block data transfer
  {0x0A000000, 0x0E000000},           // B and BL
  {0x0F000000, 0x0F000000},           // Software interrupt
  {0x06000010, 0x0E000010},           // Undefined
  {0x04000000, 0x0C000000},           // Single data transfer
  {0x01000090, 0x0F800FF0},           // Single data swap
  {0x00000090, 0x0F8000F0},           // Multiply
  {0x00800090, 0x0F8000F0},           // Multiply long
  {0x00000090, 0x0E400F90},           // Halfword data transfer
  {0x00400090, 0x0E400090},           // Halfword data transfer immediate
  {0x010F0000, 0x0FBF0000},           // MRS
  {0x0120F000, 0x0DB0F000},           // MSR
  {0x00000000, 0x0C000000}            // Data processing
};


static const char *conds[] =
{
  "eq", "ne", "hs", "lo",
  "mi", "pl", "vs", "vc",
  "hi", "ls", "ge", "lt",
  "gt", "le", "", ""
};


void arm_branch_and_exchange      (cpu_context *cpu);
void arm_block_data_transfer      (cpu_context *cpu);
void arm_branch_branch_link       (cpu_context *cpu);
void arm_software_interrupt       (cpu_context *cpu);
void arm_undefined                (cpu_context *cpu);
void arm_single_data_transfer     (cpu_context *cpu);
void arm_single_data_swap         (cpu_context *cpu);
void arm_multiply                 (cpu_context *cpu);
void arm_multiply_long            (cpu_context *cpu);
void arm_halfword_transfer        (cpu_context *cpu);
void arm_halfword_transfer_imm    (cpu_context *cpu);
void arm_mrs                      (cpu_context *cpu);
void arm_msr                      (cpu_context *cpu);
void arm_data_processing          (cpu_context *cpu);


static void (*functions[])(cpu_context *) =
{
  &arm_branch_and_exchange,
  &arm_block_data_transfer,
  &arm_branch_branch_link,
  &arm_software_interrupt,
  &arm_undefined,
  &arm_single_data_transfer,
  &arm_single_data_swap,
  &arm_multiply,
  &arm_multiply_long,
  &arm_halfword_transfer,
  &arm_halfword_transfer_imm,
  &arm_mrs,
  &arm_msr,
  &arm_data_processing
};

void arm_no_impl(cpu_context *cpu)
{
  printf("Instruction 0x%x is not implemnted!\n", cpu->instruction_to_exec);
}


void (*decode_instruction(uint32_t instruction))(cpu_context *)
{
  for (uint8_t i = 0; i < 14; ++i)
  {
    const uint32_t *current_inst = instruction_type_format_masks[i];
    if((instruction & current_inst[1]) == current_inst[0])
    {
      return functions[i];
    }
  }
  return &arm_no_impl;
}

bool verify_condition(cpu_context *cpu)
{
  uint8_t cond = (cpu->instruction_to_exec >> 28) && 0xF;
  uint32_t state_register = cpu->CPSR;
  switch (cond)
  {
  case COND_EQ:
    // z == 1
    return ((state_register >> 30) & 0x1) == 1;
  
  case COND_NE:
    // z == 0
    return ((state_register >> 30) & 0x1) == 0;
  
  case COND_HS:
    // c == 1
    return ((state_register >> 29) & 0x1) == 1;
  
  case COND_LO:
    // c == 0
    return ((state_register >> 29) & 0x1) == 0;
  
  case COND_MI:
    // n == 1
    return ((state_register >> 31) & 0x1) == 1;
  
  case COND_PL:
    // n == 0
    return ((state_register >> 31) & 0x1) == 0;
  
  case COND_VS:
    // v == 1
    return ((state_register >> 28) & 0x1) == 1;
  
  case COND_VC:
    // v == 0
    return ((state_register >> 28) & 0x1) == 0;
  
  case COND_HI:
    // c == 1 and z == 0
    return (((state_register >> 29) & 0x1) == 1) && (((state_register >> 30) & 0x1) == 0);
  
  case COND_LS:
    // c == 0 or z == 1
    return (((state_register >> 29) & 0x1) == 0) || (((state_register >> 30) & 0x1) == 1);
  
  case COND_GE:
    // n == v
    return (((state_register >> 31) & 0x1) & ((state_register >> 28) & 0x1)) == 1;
  
  case COND_LT:
    // n != v
    return (((state_register >> 31) & 0x1) & ((state_register >> 28) & 0x1)) == 0;
  
  case COND_GT:
    // z == 0 and n == v
    return (((state_register >> 30) & 0x1) == 0) && ((((state_register >> 31) & 0x1) & ((state_register >> 28) & 0x1)) == 1);
  
  case COND_LE:
    // z == 1 or n != v
    return (((state_register >> 30) & 0x1) == 1) || ((((state_register >> 31) & 0x1) & ((state_register >> 28) & 0x1)) == 0);
  
  case COND_AL:
    return true;

  default:
    fprintf(stderr, "Undefined condition!");
    exit(EXIT_FAILURE);
  }
}


void arm_branch_and_exchange(cpu_context *cpu)
{
  uint8_t Rn = cpu->instruction_to_exec & 0x0F;
  printf("bx \tr%d\n", Rn);
}


void arm_block_data_transfer(cpu_context *cpu)
{
  uint8_t cond = (cpu->instruction_to_exec >> 28) & 0xF;
  uint8_t pu = (cpu->instruction_to_exec >> 23) & 0x3;
  uint8_t s_flag = (cpu->instruction_to_exec >> 22) & 0x1;
  uint8_t writeback = (cpu->instruction_to_exec >> 21) & 0x1;
  uint8_t load = (cpu->instruction_to_exec >> 20) & 0x1;
  uint8_t Rn = (cpu->instruction_to_exec >> 16) & 0xF;

  const char *amod[] =
  {
    "da", "ia", "db", "ib"
  };

  printf(load ? "ldm" : "stm");
  printf("%s%s\tr%d%c, {", amod[pu], conds[cond], Rn, writeback ? '!' : '\0');
  bool comma = false;
  for (uint8_t i = 0; i < 0x10; ++i)
  {
    if ((cpu->instruction_to_exec >> i) & 0x1)
    {
      printf("%sr%d", comma ? ", " : "", i);
      comma = comma | true;
    }
  }
  printf("}\n");

  uint32_t base_address = cpu->regs[Rn];
  
  switch (pu)
  {
  case AMOD_DA:
    for (char i = 0xf; i >= 0; --i)
    {
      if ((cpu->instruction_to_exec >> i) & 0x1)
      {
        if (load)
          cpu->regs[i] = bus_read_word(base_address);
        else
          bus_write_word(base_address, cpu->regs[i]);
        //printf("%d\t(0x%08x) -> 0x%08x\n", i, cpu->regs[i], base_address);
        base_address -= 4;
      }
    }
    break;

  case AMOD_IA:
    for (char i = 0; i < 0x10; ++i)
    {
      if ((cpu->instruction_to_exec >> i) & 0x1)
      {
        if (load)
          cpu->regs[i] = bus_read_word(base_address);
        else
          bus_write_word(base_address, cpu->regs[i]);
        //printf("%d\t(0x%08x) -> 0x%08x\n", i, cpu->regs[i], base_address);
        base_address += 4;
      }
    }
    break;

  case AMOD_DB:
    for (char i = 0xf; i >= 0; --i)
    {
      if ((cpu->instruction_to_exec >> i) & 0x1)
      {
        base_address -= 4;
        if (load)
          cpu->regs[i] = bus_read_word(base_address);
        else
          bus_write_word(base_address, cpu->regs[i]);
        //printf("%d\t(0x%08x) -> 0x%08x\n", i, cpu->regs[i], base_address);
      }
    }
    break;

  case AMOD_IB:
    for (char i = 0; i < 0x10; ++i)
    {
      if ((cpu->instruction_to_exec >> i) & 0x1)
      {
        base_address += 4;
        if (load)
          cpu->regs[i] = bus_read_word(base_address);
        else
          bus_write_word(base_address, cpu->regs[i]);
        //printf("%d\t(0x%08x) -> 0x%08x\n", i, cpu->regs[i], base_address);
      }
    }
    break;
  }

  if (writeback)
    cpu->regs[Rn] = base_address;
}




void arm_branch_branch_link(cpu_context *cpu)
{
  uint8_t cond = (cpu->instruction_to_exec >> 28) & 0xF;
  uint8_t L = (cpu->instruction_to_exec >> 24) & 1;
  uint32_t offset = (cpu->instruction_to_exec & 0xFFFFFF) << 2;
  printf("%s%s \t#0x%x\n", (L ? "bl" : "b"), conds[cond], (offset + cpu->regs[15]) & 0xFFFFFF);
  
  if (L)
    cpu->regs[14] = cpu->regs[15] - 8;  // due to the pipeline
  //cpu->decoded_instruction = NOP;
  //cpu->fetched_instruction = NOP;
  cpu->regs[15] += offset - 4;
}

void arm_software_interrupt(cpu_context *cpu)
{
  printf("Software interrupt\n");
}

void arm_undefined(cpu_context *cpu)
{
  printf("Instruction undefined\n");
}

void arm_single_data_transfer(cpu_context *cpu)
{
  uint8_t Rn = (cpu->instruction_to_exec >> 16) & 0xF;
  uint8_t Rd = (cpu->instruction_to_exec >> 12) & 0xF;
  uint16_t offset = cpu->instruction_to_exec & 0xFFF;
  uint8_t I = (cpu->instruction_to_exec >> 25) & 1;
  uint8_t pre_indexed = (cpu->instruction_to_exec >> 24) & 1;
  uint8_t up = (cpu->instruction_to_exec >> 23) & 1;
  uint8_t byte = (cpu->instruction_to_exec >> 22) & 1;
  uint8_t writeback = (cpu->instruction_to_exec >> 21) & 1;
  uint8_t load = (cpu->instruction_to_exec >> 20) & 1;
  printf("%s%c\t", load ? "ldr" : "str", byte ? 'b' : '\0');
  printf("r%d, [r%d", Rd, Rn);
  char wb = writeback ? '!' : '\0';
  char sign = up ? '\0' : '-';
  if (!I)
    printf(pre_indexed ? ", %c#%d]%c\n" : "], %c#%d%c\n", sign, offset, wb);
  else
  {
    char *types[] = 
    {
      "lsl",
      "lsr",
      "asr",
      "ror"
    };
    printf(pre_indexed ? ", %cr%d, %s #%d]%c\n" : "], %cr%d, %s #%d%c\n", 
      sign, offset & 0xF, types[(offset >> 5) & 0x3], (offset >> 7) & 0x1F, wb);
  }
  //printf("Rn = R%d, Rd = R%d, Offset = #0x%03x\n", Rn, Rd, offset);
}

void arm_single_data_swap(cpu_context *cpu)
{
  uint8_t byte = (cpu->instruction_to_exec >> 22) & 0x1;
  uint8_t Rn = (cpu->instruction_to_exec >> 16) & 0xF;
  uint8_t Rd = (cpu->instruction_to_exec >> 12) & 0xF;
  uint8_t Rm = cpu->instruction_to_exec & 0xF;
  printf("swp%c\tr%d, r%d, [r%d]\n", byte ? 'b' : '\0',
    Rd, Rm, Rn);
}

void arm_multiply(cpu_context *cpu)
{
  uint8_t accumulate = (cpu->instruction_to_exec >> 21) & 0x1;
  uint8_t set_condition_codes = (cpu->instruction_to_exec >> 20) & 0x1;
  uint8_t Rd = (cpu->instruction_to_exec >> 16) & 0xF;
  uint8_t Rn = (cpu->instruction_to_exec >> 12) & 0xF;
  uint8_t Rs = (cpu->instruction_to_exec >> 8) & 0xF;
  uint8_t Rm = (cpu->instruction_to_exec) & 0xF;
  printf("%s%c\tr%d, r%d, r%d",
    accumulate ? "mla" : "mul", set_condition_codes ? 's' : '\0',
    Rd, Rm, Rs);
  
  accumulate ? printf(", r%d\n", Rn) : printf("\n");
}

void arm_multiply_long(cpu_context *cpu)
{
  uint8_t is_unsigned = (cpu->instruction_to_exec >> 22) & 0x1;
  uint8_t accumulate = (cpu->instruction_to_exec >> 21) & 0x1;
  uint8_t set_condition_codes = (cpu->instruction_to_exec >> 20) & 0x1;
  uint8_t Rd = (cpu->instruction_to_exec >> 16) & 0xF;
  uint8_t Rn = (cpu->instruction_to_exec >> 12) & 0xF;
  uint8_t Rs = (cpu->instruction_to_exec >> 8) & 0xF;
  uint8_t Rm = (cpu->instruction_to_exec) & 0xF;
  printf("%c%s%c\tr%d, r%d, r%d, r%d\n",
    is_unsigned ? 'u' : 's', accumulate ? "mlal" : "mull", 
    set_condition_codes ? 's' : '\0', Rn, Rd, Rm, Rs);

}

void arm_halfword_transfer(cpu_context *cpu)
{
  uint8_t cond = (cpu->instruction_to_exec >> 28) & 0xF;
  uint8_t pre_indexed = (cpu->instruction_to_exec >> 24) & 1;
  uint8_t up = (cpu->instruction_to_exec >> 23) & 1;
  uint8_t writeback = (cpu->instruction_to_exec >> 21) & 1;
  uint8_t load = (cpu->instruction_to_exec >> 20) & 1;
  uint8_t sh = (cpu->instruction_to_exec >> 5) & 0x3;

  uint8_t Rn = (cpu->instruction_to_exec >> 16) & 0xF;
  uint8_t Rd = (cpu->instruction_to_exec >> 12) & 0xF;
  uint8_t Rm = cpu->instruction_to_exec & 0xF;

  const char *l_types[] =
  {
    "RESERVED",
    "ldrh",
    "ldrsb",
    "ldrsh"
  };

  const char *s_types[] =
  {
    "RESERVED",
    "strh",
    "ldrd",
    "strd"
  };

  printf("%s%s\tr%d, [r%d",
    (load ? l_types : s_types)[sh], conds[cond],
    Rd, Rn);
  printf(pre_indexed ? ", %c#%d]%c\n" : "], %c#%d%c\n",
    up ? '\0' : '-', Rm, writeback ? '!' : '\0');
  //printf("%s%ch\tr%d, [r%d, %cr%d]%c\n", load ? "ldr" : "str", 
  //  is_signed ? 's' : '\0', Rd, Rn, up ? '\0' : '-', Rm,
  //  writeback ? '!' : '\0');

}

void arm_halfword_transfer_imm(cpu_context *cpu)
{
  uint8_t cond = (cpu->instruction_to_exec >> 28) & 0xF;
  uint8_t pre_indexed = (cpu->instruction_to_exec >> 24) & 1;
  uint8_t up = (cpu->instruction_to_exec >> 23) & 1;
  uint8_t writeback = (cpu->instruction_to_exec >> 21) & 1;
  uint8_t load = (cpu->instruction_to_exec >> 20) & 1;
  uint8_t sh = (cpu->instruction_to_exec >> 5) & 0x3;

  uint8_t Rn = (cpu->instruction_to_exec >> 16) & 0xF;
  uint8_t Rd = (cpu->instruction_to_exec >> 12) & 0xF;
  
  uint8_t offset = ((cpu->instruction_to_exec >> 4) & 0xF0) +
    (cpu->instruction_to_exec & 0xF);

  const char *l_types[] =
  {
    "RESERVED",
    "ldrh",
    "ldrsb",
    "ldrsh"
  };

  const char *s_types[] =
  {
    "RESERVED",
    "strh",
    "ldrd",
    "strd"
  };
  
  printf("%s%s\tr%d, [r%d",
    (load ? l_types : s_types)[sh], conds[cond],
    Rd, Rn);

  printf(pre_indexed ? ", %c#%d]%c\n" : "], %c#%d%c\n",
    up ? '\0' : '-', offset, writeback ? '!' : '\0');
  
}

void arm_mrs(cpu_context *cpu)
{
  uint8_t pos = (cpu->instruction_to_exec >> 22) & 0x1;
  uint8_t Rd = (cpu->instruction_to_exec >> 12) & 0xF;
  printf("mrs\tr%d, %cpsr\n", Rd, pos ? 's' : 'c');
}

void arm_msr(cpu_context *cpu)
{
  uint8_t immediate = (cpu->instruction_to_exec >> 25) & 0x1;
  uint8_t pos = (cpu->instruction_to_exec >> 22) & 0x1;
  uint16_t value = cpu->instruction_to_exec & 0xFF0;
  uint8_t Rm = cpu->instruction_to_exec & 0xF;
  printf("msr\t%cpsr, ", pos ? 's' : 'c');
  immediate ? printf("#0%d\n", value + Rm) : printf("r%d\n", Rm);
}

void arm_data_processing(cpu_context *cpu)
{
  uint8_t cond = (cpu->instruction_to_exec >> 28) & 0xF;
  uint8_t immediate = (cpu->instruction_to_exec >> 25) & 0x1;
  uint8_t opcode = (cpu->instruction_to_exec >> 21) & 0xF;
  uint8_t set_condition_codes = (cpu->instruction_to_exec >> 20) & 0x1;

  const char *ops[] =
  {
    "and", "eor", "sub", "rsb",
    "add", "adc", "sbc", "rsc",
    "tst", "teq", "cmp", "cmn",
    "orr", "mov", "bic", "mvn"
  };

  const char *shift_types[] =
  {
    "lsl", "lsr",
    "asr", "ror"
  };

  uint8_t Rn = (cpu->instruction_to_exec >> 16) & 0xF;
  uint8_t Rd = (cpu->instruction_to_exec >> 12) & 0xF;

  printf("%s%s\t", ops[opcode], conds[cond]);
  (opcode & 0xC) == 0x8 ? printf("") : printf("r%d, ", Rd); 
  (opcode & 0xD) == 0xD ? printf("") : printf("r%d, ", Rn);

  if (immediate)
  {
    uint8_t nn = (cpu->instruction_to_exec) & 0xFF;
    uint8_t Is = (cpu->instruction_to_exec >> 7) & 0x1E;  //multiplied by 2
    // ror shift
    uint32_t value = (nn >> Is) | (nn << (32 - Is));
    printf("#0x%x\n", value);
  }
  else
  {
    uint8_t shift_by_register = (cpu->instruction_to_exec >> 4) & 0x1;
    uint8_t Rm = cpu->instruction_to_exec & 0xF;

    printf("r%d, %s ", Rm, shift_types[(cpu->instruction_to_exec >> 5) & 0x3]);

    if (shift_by_register)
    {
      uint8_t Rs = (cpu->instruction_to_exec >> 8) & 0xF;
      printf("r%d\n", Rs);
    }
    else
    {
      printf("#%d\n", (cpu->instruction_to_exec >> 7) & 0x1F);
    }
  }
}

/*

  1110 0010 1000 0001 0000 0010 0000 0111
  E2810207
  

*/


void flush(cpu_context *cpu)
{
  //printf("Flushing the pipeline!\n");
  cpu->decoded_instruction = NOP;
  cpu->fetched_instruction = NOP;
}

