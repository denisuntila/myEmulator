#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "instructions.h"
#include "cpu.h"
#include "bus.h"
#include "alu.h"


// defining the nop instruction as mov r0, r0
#define NOP 0xe1a00000
#define THUMB_NOP 0x46C0

#define NO_IMPL { fprintf(stderr, "NOT YET IMPLEMENTED: INSTRUCTIONS\n"); }

#define REGS(id) *cpu->regs[id]

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


// For hw dw signed load store instructions
#define LTYPES_LDRH 0x1
#define LTYPES_LDRSB 0x2
#define LTYPES_LDRSH 0x3

#define STYPES_STRH 0x1
#define STYPES_LDRD 0x2
#define STYPES_STRD 0x3



// alu opcodes
#define ALU_AND 0x0
#define ALU_EOR 0x1
#define ALU_SUB 0x2
#define ALU_RSB 0x3
#define ALU_ADD 0x4
#define ALU_ADC 0x5
#define ALU_SBC 0x6
#define ALU_RSC 0x7
#define ALU_TST 0x8
#define ALU_TEQ 0x9
#define ALU_CMP 0xA
#define ALU_CMN 0xB
#define ALU_ORR 0xC
#define ALU_MOV 0xD
#define ALU_BIC 0xE
#define ALU_MVN 0xF


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


static const uint16_t thumb_instruction_type_format_masks[][2] =
{
  {0xDF00, 0xFF00},                   // Software interrupt
  {0xE000, 0xF800},                   // Unconditional branch
  {0xD000, 0xF000},                   // Conditional branch
  {0xC000, 0xF000},                   // Multiple load / store
  {0xF000, 0xF000},                   // Long BL
  {0xB000, 0xFF00},                   // Add offset to SP
  {0xB400, 0xF600},                   // Push / Pop registers
  {0x8000, 0xF000},                   // Load / Store halfword
  {0x9000, 0xF000},                   // SP relative l / s
  {0xA000, 0xF000},                   // Load address
  {0x6000, 0xE000},                   // L / S with immediate offset
  {0x5000, 0xF200},                   // L / S with register offset
  {0x5200, 0xF200},                   // L / S sign-ext. byte / halfword
  {0x4800, 0xF800},                   // PC relative load
  {0x4400, 0xFC00},                   // Hi reg ops / BX
  {0x4000, 0xFC00},                   // ALU operations
  {0x2000, 0xE000},                   // mov/cmp/add/sub immediate
  {0x1800, 0xF800},                   // add / sub
  {0x0000, 0xE000}                    // mov shifted registers
};



static const char *conds[] =
{
  "eq", "ne", "hs", "lo",
  "mi", "pl", "vs", "vc",
  "hi", "ls", "ge", "lt",
  "gt", "le", "", ""
};


void arm_branch_and_exchange          (cpu_context *cpu);
void arm_block_data_transfer          (cpu_context *cpu);
void arm_branch_branch_link           (cpu_context *cpu);
void arm_software_interrupt           (cpu_context *cpu);
void arm_undefined                    (cpu_context *cpu);
void arm_single_data_transfer         (cpu_context *cpu);
void arm_single_data_swap             (cpu_context *cpu);
void arm_multiply                     (cpu_context *cpu);
void arm_multiply_long                (cpu_context *cpu);
void arm_halfword_transfer            (cpu_context *cpu);
void arm_halfword_transfer_imm        (cpu_context *cpu);
void arm_mrs                          (cpu_context *cpu);
void arm_msr                          (cpu_context *cpu);
void arm_data_processing              (cpu_context *cpu);



void thumb_software_interrupt         (cpu_context *cpu);
void thumb_unconditional_branch       (cpu_context *cpu);
void thumb_conditional_branch         (cpu_context *cpu);
void thumb_multiple_load_store        (cpu_context *cpu);
void thumb_long_branch_and_link       (cpu_context *cpu);
void thumb_add_offset_to_sp           (cpu_context *cpu);
void thumb_push_pop_registers         (cpu_context *cpu);
void thumb_load_store_halfword        (cpu_context *cpu);
void thumb_sp_relative_load_store     (cpu_context *cpu);
void thumb_load_address               (cpu_context *cpu);
void thumb_load_store_imm_ofs         (cpu_context *cpu);
void thumb_load_store_reg_ofs         (cpu_context *cpu);
void thumb_load_store_sign_ext_b_h    (cpu_context *cpu);
void thumb_pc_relative_load           (cpu_context *cpu);
void thumb_hi_regs_ops_bx             (cpu_context *cpu);
void thumb_alu_operations             (cpu_context *cpu);
void thumb_mov_cmp_add_sub_imm        (cpu_context *cpu);
void thumb_add_sub                    (cpu_context *cpu);
void thumb_mov_shifted_regs           (cpu_context *cpu);



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

static void (*thumb_functions[])(cpu_context *) =
{
  thumb_software_interrupt,
  thumb_unconditional_branch,
  thumb_conditional_branch,
  thumb_multiple_load_store,
  thumb_long_branch_and_link,
  thumb_add_offset_to_sp,
  thumb_push_pop_registers,
  thumb_load_store_halfword,
  thumb_sp_relative_load_store,
  thumb_load_address,
  thumb_load_store_imm_ofs,
  thumb_load_store_reg_ofs,
  thumb_load_store_sign_ext_b_h,
  thumb_pc_relative_load,
  thumb_hi_regs_ops_bx,
  thumb_alu_operations,
  thumb_mov_cmp_add_sub_imm,
  thumb_add_sub,
  thumb_mov_shifted_regs
};

void arm_no_impl(cpu_context *cpu)
{
  printf("Instruction 0x%x is not implemnted!\n", cpu->decoded_instruction);
}

void thumb_no_impl(cpu_context *cpu)
{
  printf("Instruction 0x%x is not implemnted!\n", cpu->thumb_decode);
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
  uint8_t cond = cpu->instruction_to_exec >> 28;
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
    return ((state_register >> 31) & 0x1) == ((state_register >> 28) & 0x1);
  
  case COND_LT:
    // n != v
    return ((state_register >> 31) & 0x1) != ((state_register >> 28) & 0x1);
  
  case COND_GT:
    // z == 0 and n == v
    return (((state_register >> 30) & 0x1) == 0) && (((state_register >> 31) & 0x1) == ((state_register >> 28) & 0x1));
  
  case COND_LE:
    // z == 1 or n != v
    return (((state_register >> 30) & 0x1) == 1) || (((state_register >> 31) & 0x1) != ((state_register >> 28) & 0x1));
  
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
  //REGS(15) = (REGS(Rn) & 0xFFFFFFFE) - 4;
  REGS(15) = (REGS(Rn) & 0xFFFFFFFE);
  //cpu->CPSR |= 0x00000020;
  cpu->CPSR |= ((REGS(Rn) & 0x1) << 5);
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

  uint32_t base_address = REGS(Rn);
  
  switch (pu)
  {
  case AMOD_DA:
    for (char i = 0xf; i >= 0; --i)
    {
      if ((cpu->instruction_to_exec >> i) & 0x1)
      {
        if (load)
          REGS(i) = bus_read_word(base_address);
        else
          bus_write_word(base_address, REGS(i));
        //printf("%d\t(0x%08x) -> 0x%08x\n", i, REGS(i), base_address);
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
          REGS(i) = bus_read_word(base_address);
        else
          bus_write_word(base_address, REGS(i));
        //printf("%d\t(0x%08x) -> 0x%08x\n", i, REGS(i), base_address);
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
          REGS(i) = bus_read_word(base_address);
        else
          bus_write_word(base_address, REGS(i));
        //printf("%d\t(0x%08x) -> 0x%08x\n", i, REGS(i), base_address);
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
          REGS(i) = bus_read_word(base_address);
        else
          bus_write_word(base_address, REGS(i));
        //printf("%d\t(0x%08x) -> 0x%08x\n", i, REGS(i), base_address);
      }
    }
    break;
  }

  if (writeback)
    REGS(Rn) = base_address;
}


void arm_branch_branch_link(cpu_context *cpu)
{
  uint8_t cond = (cpu->instruction_to_exec >> 28) & 0xF;
  uint8_t L = (cpu->instruction_to_exec >> 24) & 1;
  int32_t offset = (cpu->instruction_to_exec & 0xFFFFFF) << 2;
  offset |= (0 - (offset & 0x800000));        // sign extension
  printf("%s%s \t#0x%x\n", (L ? "bl" : "b"), conds[cond], (offset + (int32_t)REGS(15)));
  
  if (L)
    REGS(14) = REGS(15) - 4;  // due to the pipeline
  //cpu->decoded_instruction = NOP;
  //cpu->fetched_instruction = NOP;
  //printf("---> %x + %x - 0x4\n", REGS(15), offset);
  //REGS(15) += offset - 4;
  REGS(15) += offset;

  flush(cpu);
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
  // TO VERIFY!!!
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
  printf(pre_indexed ? ", %cr%d]%c\n" : "], %c#%d%c\n",
    up ? '\0' : '-', Rm, writeback ? '!' : '\0');
  //printf("%s%ch\tr%d, [r%d, %cr%d]%c\n", load ? "ldr" : "str", 
  //  is_signed ? 's' : '\0', Rd, Rn, up ? '\0' : '-', Rm,
  //  writeback ? '!' : '\0');


  // Implementation
  uint32_t base_address = REGS(Rn);
  if (load)
  {
    switch (sh)
    {
    case LTYPES_LDRH:
      if (pre_indexed)
      {
        if (up)
          base_address += REGS(Rm);
        else
          base_address -= REGS(Rm);
        REGS(Rd) = 0x00000000 | bus_read_halfword(base_address);
      }
      else
      {
        REGS(Rd) = 0x00000000 | bus_read_halfword(base_address);
        if (up)
          base_address += REGS(Rm);
        else
          base_address -= REGS(Rm);
      }
      break;

    case LTYPES_LDRSB:
      uint8_t value;
      if (pre_indexed)
      {
        if (up)
          base_address += REGS(Rm);
        else
          base_address -= REGS(Rm);
        value =  bus_read_halfword(base_address);
      }
      else
      {
        value =  bus_read_halfword(base_address);
        if (up)
          base_address += REGS(Rm);
        else
          base_address -= REGS(Rm);
      }
      // sign extension
      uint32_t sign_extension = (value & 0x80) ? 0xFFFFFF00 : 0x00000000;

      REGS(Rd) = sign_extension | value;
      break;

    case LTYPES_LDRSH:
      uint16_t valueh;
      if (pre_indexed)
      {
        if (up)
          base_address += REGS(Rm);
        else
          base_address -= REGS(Rm);
        valueh =  bus_read_halfword(base_address);
      }
      else
      {
        valueh =  bus_read_halfword(base_address);
        if (up)
          base_address += REGS(Rm);
        else
          base_address -= REGS(Rm);
      }
      // sign extension
      uint32_t sign_extensionh = (valueh & 0x8000) ? 0xFFFF0000 : 0x00000000;

      REGS(Rd) = sign_extensionh | valueh;
      break;

    default:
      fprintf(stderr, "Invalid load type");
      exit(EXIT_FAILURE);
      break;
    }
  }
  else
  {
    switch (sh)
    {
    case STYPES_STRH:
      if (pre_indexed)
      {
        if (up)
          base_address += REGS(Rm);
        else
          base_address -= REGS(Rm);
        bus_write_halfword(base_address, REGS(Rd));
      }
      else
      {
        bus_write_halfword(base_address, REGS(Rd));
        if (up)
          base_address += REGS(Rm);
        else
          base_address -= REGS(Rm);
      }
      break;

    case STYPES_LDRD:
      fprintf(stderr, "ldrd unsupported on this architecture!\n");
      exit(EXIT_FAILURE);
      break;

    case STYPES_STRD:
      fprintf(stderr, "strd unsupported on this architecture!\n");
      exit(EXIT_FAILURE);
      break;

    default:
      fprintf(stderr, "Invalid store type!\n");
      exit(EXIT_FAILURE);
      break;
    }
  }

  if (writeback)
    REGS(Rn) = base_address;

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
  uint8_t psr = (cpu->instruction_to_exec >> 22) & 0x1;
  uint32_t value = cpu->instruction_to_exec & 0x0F0;
  uint16_t shift = (cpu->instruction_to_exec >> 7) & 0x1E;
  uint8_t Rm = cpu->instruction_to_exec & 0xF;

  value += Rm;
  value = (value >> shift) | (value << (32 - shift));

  printf("msr\t%cpsr, ", psr ? 's' : 'c');
  immediate ? printf("#0x%x\n", value) : printf("r%d\n", Rm);


  // Implementation
  uint32_t *psr_ptr;
  if (0 == psr)
    psr_ptr = &cpu->CPSR;
  else
    NO_IMPL;

  if (immediate)
    *psr_ptr = value;
  else
    *psr_ptr = REGS(Rm);  
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

  printf("%s%s%s\t", ops[opcode], set_condition_codes ? "s" : "", 
    conds[cond]);
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

  const bool is_logical[] =
  {
    true, true,
    false, false, false, false, false,
    true, true,
    false, false,
    true, true, true, true
  };


  // Implementation
  void (*function)(alu_args *) = alu_functions[opcode];
  uint32_t op2;
  alu_args args;
  args.check_carry = false;

  if (((cpu->instruction_to_exec >> 25) & 0x1) == 1)
  {
    uint8_t nn = (cpu->instruction_to_exec) & 0xFF;
    uint8_t Is = (cpu->instruction_to_exec >> 7) & 0x1E;
    op2 = (nn >> Is) | (nn << (32 - Is));
    if (((cpu->instruction_to_exec >> 20) & 0x1) & is_logical[opcode])
      cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
        ((op2 & 0x80000000) >> 2);
  }
  else
  {
    uint32_t shift = 0x00000000;
    uint8_t shift_by_register = (cpu->instruction_to_exec >> 4) & 0x1;
    if (shift_by_register)
    {
      shift = REGS((cpu->instruction_to_exec >> 8) & 0xF) & 0xFF;
    }
    else
    {
      shift = (cpu->instruction_to_exec >> 7) & 0x1F;
    }
    uint32_t val = REGS(cpu->instruction_to_exec & 0xF);
    switch ((alu_shift_t)((cpu->instruction_to_exec >> 5) & 0x3))
    {
    case LSL:   // optimize this please!!
      if (shift < 32)
      {
        op2 = val << shift;

        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val << (shift - 1)) >> 2);
      }
      else
      {
        op2 = 0;
        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            (((val & 0x1) && (shift == 32)) << 29);
      }     
      break;

    case LSR:
      if ((shift == 0) && (!shift_by_register))
      {
        op2 = 0;
        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x80000000) >> 2);
      }
      else if (shift >= 32)
      {
        op2 = 0;
        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF);
      }
      else
      {
        op2 = val >> shift; 
        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val >> (shift - 1)) << 29); 
      }
      break;

    case ASR:
      if ((shift == 0) && (!shift_by_register))
      {
        op2 = (val & 0x80000000) ? 0xFFFFFFFF : 0x0;
        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x80000000) >> 2);
      }
      else
      {
        op2 = (int32_t)(val) >> shift; 
        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            (((int32_t)(val) >> (shift - 1)) << 29); 
      }
      break;
    
    case ROR:
      if ((shift == 0) && (!shift_by_register))
      {
        op2 = (val >> 1) | ((cpu->CPSR << 2) & 0x80000000);
        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x1) << 29);
      }
      else if (shift == 32)
      {
        op2 = val;
        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x80000000) >> 2);
      }
      else
      {
        op2 = (val >> shift) | (val << (32 - shift));
        if ((cpu->instruction_to_exec >> 20) & 0x1)
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x1) << 29);
      }
      break;

    default:
      break;
    }
  }
  
  args.cpu = cpu;
  args.set_condition_codes = (((cpu->instruction_to_exec >> 20) & 0x1) == 1);
  args.Rd = (cpu->instruction_to_exec >> 12) & 0xF;
  args.Rn = (cpu->instruction_to_exec >> 16) & 0xF;
  args.op2 = op2;
  //printf("FLAG S = %s\n", args.set_condition_codes ? "TRUE" : "FALSE");
  function(&args);
  //printf("nzcv = 0x%04b\n", cpu->CPSR >> 28);
  if (args.Rd == 15)
    flush(cpu);
}



void flush(cpu_context *cpu)
{
  //printf("Flushing the pipeline!\n");
  cpu->decoded_instruction = NOP;
  cpu->fetched_instruction = NOP;
}




void (*thumb_decode_instruction(uint16_t instruction))(cpu_context *)
{
  for (uint8_t i = 0; i < 19; ++i)
  {
    const uint16_t *current_inst = thumb_instruction_type_format_masks[i];
    if((instruction & current_inst[1]) == current_inst[0])
    {
      return thumb_functions[i];
    }
  }
  return &thumb_no_impl;
}


void thumb_software_interrupt(cpu_context *cpu)
{
  printf("SWI\n");
  NO_IMPL;
}

void thumb_unconditional_branch(cpu_context *cpu)
{
  printf("UNCOND\n");
  NO_IMPL;
}

void thumb_conditional_branch(cpu_context *cpu)
{
  printf("COND\n");
  NO_IMPL;
}

void thumb_multiple_load_store(cpu_context *cpu)
{
  printf("MLS\n");
  NO_IMPL;
}

void thumb_long_branch_and_link(cpu_context *cpu)
{
  printf("LBL\n");
  NO_IMPL;
}

void thumb_add_offset_to_sp(cpu_context *cpu)
{
  printf("AOSP\n");
  NO_IMPL;
}

void thumb_push_pop_registers(cpu_context *cpu)
{
  printf("PPR\n");
  NO_IMPL;
}

void thumb_load_store_halfword(cpu_context *cpu)
{
  printf("LSH\n");
  NO_IMPL;
}

void thumb_sp_relative_load_store(cpu_context *cpu)
{
  printf("SPLS\n");
  NO_IMPL;
}

void thumb_load_address(cpu_context *cpu)
{
  uint8_t Rd = (cpu->thumb_exec >> 8) & 0x7;
  uint8_t sp = (cpu->thumb_exec >> 11) & 0x1;
  uint16_t offset = (cpu->thumb_exec << 2) & 0x03FC; 
  printf("add\tR%d, %s, #0x%x\n", Rd, sp ? "sp" : "pc", offset);

  // Implementation
  void (*function)(alu_args *) = thumb_alu_functions[2];  // add
  alu_args args;
  args.cpu = cpu;
  args.Rd = Rd;
  args.Rn = sp ? 13 : 15;
  args.op2 = offset;
  args.set_condition_codes = false; // Temporarly
  function(&args);
}

void thumb_load_store_imm_ofs(cpu_context *cpu)
{
  printf("LSIO\n");
  NO_IMPL;
}

void thumb_load_store_reg_ofs(cpu_context *cpu)
{
  printf("LSRO\n");
  NO_IMPL;
}

void thumb_load_store_sign_ext_b_h(cpu_context *cpu)
{
  printf("LSSEBH\n");
  NO_IMPL;
}

void thumb_pc_relative_load (cpu_context *cpu)
{
  printf("PRL\n");
  NO_IMPL;
}

void thumb_hi_regs_ops_bx(cpu_context *cpu)
{
  uint8_t opcode = (cpu->thumb_exec >> 8) & 0x3;
  uint8_t Rd = (cpu->thumb_exec & 0x7) | ((cpu->thumb_exec >> 4)) & 0x8;
  uint8_t Rs = (cpu->thumb_exec >> 3) & 0x8;

  switch (opcode)
  {
  case 0x0:   // Add
    printf("add\tR%d, R%d\n", Rd, Rs);
    break;
  
  case 0x1:
    printf("cmp\tR%d, R%d\n", Rd, Rs);
    break;
  
  case 0x2:
    printf("mov\tR%d, R%d\n", Rd, Rs);
    REGS(Rd) = REGS(Rs);
    break;
  
  case 0x3:
    printf("b%sx\tR%d\n", (Rd & 0x8) ? "l" : "" , Rs);
    REGS(15) = (REGS(Rs)) - 2;
    cpu->CPSR &= 0xFFFFFFDF;
    break;
  }

}

void thumb_alu_operations(cpu_context *cpu)
{
  NO_IMPL;
}

void thumb_mov_cmp_add_sub_imm(cpu_context *cpu)
{
  uint8_t opcode = (cpu->thumb_exec >> 11) & 0x3;
  uint8_t Rd = (cpu->thumb_exec >> 8) & 0x7;
  uint8_t nn = cpu->thumb_exec & 0xFF;

  const char *ops[] =
  {
    "mov", "cmp",
    "add", "sub"
  };

  printf("%s\tR%d, #0x%x\n", ops[opcode], Rd, nn);

  // Implementation
  void (*function)(alu_args *) = thumb_alu_functions[opcode];
  alu_args args;
  args.cpu = cpu;
  args.Rd = Rd;
  args.op2 = nn;
  args.set_condition_codes = false; // Temporarly
  function(&args);
}

void thumb_add_sub(cpu_context *cpu)
{
  NO_IMPL;
}

void thumb_mov_shifted_regs(cpu_context *cpu)
{
  NO_IMPL;
}


void thumb_flush(cpu_context *cpu)
{
  //printf("Flushing the pipeline!\n");
  cpu->thumb_decode   = THUMB_NOP;
  cpu->thumb_fetch    = THUMB_NOP;
}

