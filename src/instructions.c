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


void switch_mode(cpu_context *cpu, uint8_t mode)
{
  switch (mode)
  {
  case 0x00:
    printf("Switching to OLD USER MODE\n");
    // not implemented
    fprintf(stderr, "OLD USER mode not yet implemented\n");
    exit(EXIT_FAILURE);
    break;
  
  case 0x11:
    printf("Switching to FIQ MODE\n");
    cpu->current_SPSR = &cpu->SPSR_fiq;
    *cpu->current_SPSR = cpu->CPSR;
    for(int i = 8; i < 15; ++i)
    {
      cpu->regs[i] = &cpu->regs_fiq[i - 8];
    }
    break;
  
  case 0x1F:
    printf("Switching to SYSTEM MODE\n");
    if(cpu->current_SPSR == NULL)
    {
      fprintf(stderr, "Tying to switch to SYS mode but current spsr is null!\n");
      exit(EXIT_FAILURE);
    }
    cpu->CPSR = (*cpu->current_SPSR & (0xFFFFFF00)) | 0x0000001F;
    cpu->current_SPSR = NULL;
    for(int i = 8; i < 15; ++i)
    {
      cpu->regs[i] = &cpu->regs_sys_usr[i];
    }
    break;
  
  default:
    fprintf(stderr, "Invalid mode set!\n");
    exit(EXIT_FAILURE);
  }
}


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

bool verify_condition(cpu_context *cpu, uint8_t cond)
{
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
  thumb_flush(cpu);
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
  printf("}%c\n", s_flag ? '^' : '\0');

  uint32_t base_address = REGS(Rn);
  uint32_t base_store_address = base_address;
  bool first_flag;
  bool base_reg_in_rlist = false;
  bool base_reg_first_in_rlist = false;
  int32_t increment = 0;
  switch (pu)
  {
  case AMOD_DA:
    if((cpu->instruction_to_exec & 0xFFFF) == 0)
    {
      if(load)
      {
        base_address -= 0x3C;
        REGS(15) = bus_read_word(base_address);
        base_address -= 0x04;
      }
      else
      {
        base_address -= 0x3C;
        bus_write_word(base_address, REGS(15) + 4);
        base_address -= 0x04;
      }
    }
    else
    {
      first_flag = false;
      for (char i = 0xf; i >= 0; --i)
      {
        if ((cpu->instruction_to_exec >> i) & 0x1)
        {
          if(i == Rn)
          {
            base_reg_in_rlist = true;
          }
          if (load)
          {
            REGS(i) = bus_read_word(base_address);
            if(base_reg_in_rlist)
              writeback = 0;
          }
          else
          {
            if((i == Rn))
            {
              base_store_address = base_address;
              first_flag = true;
            }
            else
              first_flag = false;

            bus_write_word(base_address, REGS(i) + ((i == 15) ? 0x4 : 0x0));
          }

          base_address -= 4;
          increment -= 4;
        }
      }
      base_reg_first_in_rlist = first_flag;
    }
    break;

  case AMOD_IA:
    if((cpu->instruction_to_exec & 0xFFFF) == 0)
    {
      if(load)
      {
        REGS(15) = bus_read_word(base_address);
        base_address += 0x40;
      }
      else
      {
        bus_write_word(base_address, REGS(15) + 4);
        base_address += 0x40;
      }
    }
    else
    {
      first_flag = true;
      for (char i = 0; i < 0x10; ++i)
      {
        if ((cpu->instruction_to_exec >> i) & 0x1)
        {
          if(i == Rn)
          {
            base_reg_in_rlist = true;
          }
          if (load)
          {
            if(base_reg_in_rlist)
              writeback = 0;
            if(s_flag)
              cpu->regs_sys_usr[i] = bus_read_word(base_address);
            else
              REGS(i) = bus_read_word(base_address);
          }
          else
          {
            if (i == Rn)
            {
              base_store_address = base_address;
            }
            if(base_reg_in_rlist && first_flag)
            {
              base_reg_first_in_rlist = true;
              base_store_address = base_address;
              base_address -= 4;
              increment -=4;
            }   
            bus_write_word(base_address, REGS(i) + ((i == 15) ? 0x4 : 0x0));
          }
          base_address += 4;
          increment += 4;
          first_flag = false;
        }
      }
    }
    break;

  case AMOD_DB:
    if((cpu->instruction_to_exec & 0xFFFF) == 0)
    {
      if(load)
      {
        base_address -= 0x40;
        REGS(15) = bus_read_word(base_address);
      }
      else
      {
        base_address -= 0x40;
        bus_write_word(base_address, REGS(15) + 4);
      }
    }
    else
    {
      first_flag = false;
      for (char i = 0xf; i >= 0; --i)
      {
        if ((cpu->instruction_to_exec >> i) & 0x1)
        {
          if(i == Rn)
          {
            base_reg_in_rlist = true;
          }
        
          base_address -= 4;
          increment -= 4;

          if (load)
          {
            if(base_reg_in_rlist)
              writeback = 0;
            if(s_flag)
              cpu->regs_sys_usr[i] = bus_read_word(base_address);
            else
              REGS(i) = bus_read_word(base_address);
          }
          else
          {
            uint32_t val;
            if (s_flag)
              val = cpu->regs_sys_usr[i];
            else
              val = REGS(i);
            if((i == Rn))
            {
              base_store_address = base_address;
              first_flag = true;
            }
            else
              first_flag = false;
            bus_write_word(base_address, val + ((i == 15) ? 0x4 : 0x0));
          }
        }
      }
      base_reg_first_in_rlist = first_flag;
    }
    break;

  case AMOD_IB:
    if((cpu->instruction_to_exec & 0xFFFF) == 0)
    {
      if(load)
      {
        base_address += 0x04;
        REGS(15) = bus_read_word(base_address);
        base_address += 0x3C;
      }
      else
      {
        base_address += 0x04;
        bus_write_word(base_address, REGS(15) + 4);
        base_address += 0x3C;
      }
    }
    else
    {
      first_flag = true;
      for (char i = 0; i < 0x10; ++i)
      {
        if ((cpu->instruction_to_exec >> i) & 0x1)
        {
          if(i == Rn)
          {
            base_reg_in_rlist = true;
          }

          base_address += 4;
          increment += 4;
          if (load)
          {
            if(base_reg_in_rlist)
              writeback = 0;
            REGS(i) = bus_read_word(base_address);
          }  
          else
          { 
            if (i == Rn)
            {
              base_store_address = base_address;
            }
            if(base_reg_in_rlist && first_flag)
            {
              base_reg_first_in_rlist = true;
              base_store_address = base_address;
              base_address -= 4;
              increment -=4;
            } 
            bus_write_word(base_address, REGS(i) + ((i == 15) ? 0x4 : 0x0));
          }
          first_flag = false;
        }
      }
    }
    break;
  }

  if (writeback)
    REGS(Rn) = base_address;

  //if (base_reg_in_rlist && ((int32_t)(base_address - base_store_address) != increment))
  //  bus_write_word(base_store_address, base_address);
  if (!load && base_reg_in_rlist)
  {
    if (base_reg_first_in_rlist)  // Store the old value
    {
      bus_write_word(base_store_address, base_address - increment);
    }
    else
    {
      bus_write_word(base_store_address, base_address);
    }
  }
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
  uint32_t offset = cpu->instruction_to_exec & 0xFFF;
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
  
  // Implementation
  uint32_t address = REGS(Rn);
  //uint32_t offset;

  if(!I)
  {
    offset = cpu->instruction_to_exec & 0x00000FFF;
  }
  else
  {
    offset = cpu->instruction_to_exec & 0xF;
    uint8_t shift = (cpu->instruction_to_exec >> 7) & 0x1F;

    switch((alu_shift_t)((cpu->instruction_to_exec >> 5) & 0x3))
    {
    case 0x0:
      offset = offset << shift;
      break;

    case 0x1:
      offset = offset >> shift;
      break;
    
    case 0x2:
      offset = (uint32_t)(((int32_t)offset) >> shift);
      break;
    
    case 0x3:
      //printf("SHIFT = 0x%08x\n", shift);
      if(shift != 0)
        offset = (offset >> shift) | (offset << (32 - shift));
      else
        offset = (offset >> 1) | ((cpu->CPSR & 0x20000000) << 2);

      //printf("OFFSET = 0x%08x\n", offset);
      break;
    }
  }
  
  uint8_t rotation_in_word = offset % 4;
  offset -= rotation_in_word;
  if(pre_indexed)
  {
    if(up)
      address += offset;
    else
      address -= offset;
  }
  

  if(byte)
  {
    if(load)
      REGS(Rd) = (uint32_t)((int32_t)bus_read(address));
    else
      bus_write(address, (uint8_t)REGS(Rd));
  }
  else
  {
    if(load)
    {
      uint32_t temp = bus_read_word(address);
      REGS(Rd) = (temp >> (rotation_in_word * 8)) |
        (temp << (32 - (rotation_in_word * 8)));
    }  
    else
    {
      uint32_t value = REGS(Rd) + ((Rd == 15) ? 4 : 0);
      bus_write_word(address, value);
    }
      
  }
  if(!pre_indexed)
  {
    if(up)
      address += offset;
    else
      address -= offset;
  }

  if((writeback || !pre_indexed) && !(load && (Rn == Rd))) REGS(Rn) = address;

  //if((writeback || !pre_indexed) && !(load && (Rn == Rd))) printf("ZIO PERA\n");

}

void arm_single_data_swap(cpu_context *cpu)
{
  uint8_t byte = (cpu->instruction_to_exec >> 22) & 0x1;
  uint8_t Rn = (cpu->instruction_to_exec >> 16) & 0xF;
  uint8_t Rd = (cpu->instruction_to_exec >> 12) & 0xF;
  uint8_t Rm = cpu->instruction_to_exec & 0xF;
  printf("swp%c\tr%d, r%d, [r%d]\n", byte ? 'b' : '\0',
    Rd, Rm, Rn);

  // Implementation
  uint32_t address = REGS(Rn);
  uint8_t rotation_in_word = address % 4;
  address -= rotation_in_word;

  if(byte)
  {
    REGS(Rd) = bus_read(address);
    bus_write(address, (uint8_t)REGS(Rm));
  }
  else
  {
    uint32_t temp = bus_read_word(address);
    uint32_t value_to_store = REGS(Rm);
    REGS(Rd) = (temp >> (rotation_in_word * 8)) |
        (temp << (32 - (rotation_in_word * 8)));
    bus_write_word(address, value_to_store);
  }
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

  // Implmentation
  if(!accumulate)
  {
    uint32_t a = REGS(Rm);
    uint32_t b = REGS(Rs);
    uint32_t result = a * b;

    REGS(Rd) = result;
  }
  else
  {
    int32_t a = REGS(Rm);
    int32_t b = REGS(Rs);
    int32_t result = a * b;
    result += (uint32_t)REGS(Rn);

    REGS(Rd) = result;
  }
  
  if (set_condition_codes)
    printf("CPSR flags not yet implemnted for this instruction\n");

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
    is_unsigned ? 's' : 'u', accumulate ? "mlal" : "mull", 
    set_condition_codes ? 's' : '\0', Rn, Rd, Rm, Rs);
  
  
  // Implementation
  //printf("UA flags = 0b%02b\n", (cpu->instruction_to_exec >> 21) & 0x3);
  switch ((cpu->instruction_to_exec >> 21) & 0x3)
  {
  case 0x0: // umull
    //printf("Not yet implemented!\n");
    uint32_t a = REGS(Rm);
    uint32_t b = REGS(Rs);
    uint64_t result = (uint64_t)a * (uint64_t)b;
    
    // RdHi
    REGS(Rd) = *(uint32_t *)((void *)&result + 4);
    // RdLo
    REGS(Rn) = *(uint32_t *)((void *)&result);

    if(set_condition_codes)
    {
      cpu->CPSR = (cpu->CPSR & 0x7FFFFFFF) |
        (result & 0x80000000);

      cpu->CPSR = (cpu->CPSR & 0xBFFFFFFF) |
        ((result == 0) << 30);
    }
    break;
  
  case 0x1: // umlal
    //printf("Not yet implemented!\n");
    a = REGS(Rm);
    b = REGS(Rs);
    result = (uint64_t)a * (uint64_t)b;
    result += (int64_t)(((uint64_t)REGS(Rd) << 32) | ((uint64_t)REGS(Rn)));
    
    // RdHi
    REGS(Rd) = *(uint32_t *)((void *)&result + 4);
    // RdLo
    REGS(Rn) = *(uint32_t *)((void *)&result);

    
    if(set_condition_codes)
    {
      cpu->CPSR = (cpu->CPSR & 0x7FFFFFFF) |
        (result & 0x80000000);

      cpu->CPSR = (cpu->CPSR & 0xBFFFFFFF) |
        ((result == 0) << 30);
    }
    break;

  case 0x2: // smull
    int32_t a_s = REGS(Rm);
    int32_t b_s = REGS(Rs);
    int64_t result_s = (int64_t)a_s * (int64_t)b_s;
    
    // RdHi
    REGS(Rd) = *(uint32_t *)((void *)&result_s + 4);
    // RdLo
    REGS(Rn) = *(uint32_t *)((void *)&result_s);

    if(set_condition_codes)
    {
      cpu->CPSR = (cpu->CPSR & 0x7FFFFFFF) |
        (result_s & 0x80000000);
        
      cpu->CPSR = (cpu->CPSR & 0xBFFFFFFF) |
        ((result_s == 0) << 30);
    }
    break;

  default:
    a_s = REGS(Rm);
    b_s = REGS(Rs);
    result_s = (int64_t)a_s * (int64_t)b_s;
    result_s += (((int64_t)REGS(Rd) << 32) | ((int64_t)REGS(Rn)));
    
    // RdHi
    REGS(Rd) = *(uint32_t *)((void *)&result_s + 4);
    // RdLo
    REGS(Rn) = *(uint32_t *)((void *)&result_s);

    if(set_condition_codes)
    {
      cpu->CPSR = (cpu->CPSR & 0x7FFFFFFF) |
        (result_s & 0x80000000);
        
      cpu->CPSR = (cpu->CPSR & 0xBFFFFFFF) |
        ((result_s == 0) << 30);
    }
    break;
    
  }

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
  printf(pre_indexed ? ", %cr%d]%c\n" : "], %c#%d%c\n",
    up ? '\0' : '-', Rm, writeback ? '!' : '\0');


  // Implementation
  uint32_t base_address = REGS(Rn);
  uint32_t offset = REGS(Rm);
  uint8_t rotation_in_word = offset % 2;
  offset -= rotation_in_word;
  if (load)
  {
    switch (sh)
    {
    case LTYPES_LDRH:
      if (pre_indexed)
      {
        if (up)
          base_address += offset;
        else
          base_address -= offset;
        REGS(Rd) = 0x00000000 | bus_read_halfword(base_address);
      }
      else
      {
        REGS(Rd) = 0x00000000 | bus_read_halfword(base_address);
        if (up)
          base_address += offset;
        else
          base_address -= offset;
      }
      break;

    case LTYPES_LDRSB:
      uint8_t value;
      if (pre_indexed)
      {
        if (up)
          base_address += offset;
        else
          base_address -= offset;
        value =  bus_read_halfword(base_address);
      }
      else
      {
        value =  bus_read_halfword(base_address);
        if (up)
          base_address += offset;
        else
          base_address -= offset;
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
        valueh = valueh >> (rotation_in_word * 8);
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
      uint32_t negative_mask = (rotation_in_word) ? 0xFFFFFF00 : 0xFFFF0000;
      uint32_t sign_bit_mask = (rotation_in_word) ? 0x00000080 : 0x00008000;
      uint32_t sign_extensionh = (valueh & sign_bit_mask) ? negative_mask : 0x00000000;

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

  //if (writeback)
  //  REGS(Rn) = base_address;
  if((writeback || !pre_indexed) && !(load && (Rn == Rd))) 
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
  
  uint8_t ofs = ((cpu->instruction_to_exec >> 4) & 0xF0) +
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
    up ? '\0' : '-', ofs, writeback ? '!' : '\0');
  
  
  // Implementation
  uint32_t offset = (cpu->instruction_to_exec & 0x0000000F) |
  ((cpu->instruction_to_exec >> 4) & 0x000000F0);
  uint8_t rotation_in_word = offset % 2;
  offset -= rotation_in_word;

  uint32_t base_address = REGS(Rn);
  if (load)
  {
    switch (sh)
    {
    case LTYPES_LDRH:
      if (pre_indexed)
      {
        if (up)
          base_address += offset;
        else
          base_address -= offset;
        uint32_t temp = bus_read_halfword(base_address);
        printf("temp = 0x%08x\n", temp);
        temp = (temp >> (rotation_in_word * 8)) |
          (temp << (32 - (rotation_in_word * 8)));
        REGS(Rd) = temp;
        printf("temp = 0x%08x\n", temp);
      }
      else
      {
        uint32_t temp = bus_read_halfword(base_address);
        temp = (temp >> (rotation_in_word * 8)) |
          (temp << (32 - (rotation_in_word * 8)));
        REGS(Rd) = temp;
        if (up)
          base_address += offset;
        else
          base_address -= offset;
      }
      break;

    case LTYPES_LDRSB:
      uint8_t value;
      if (pre_indexed)
      {
        if (up)
          base_address += offset;
        else
          base_address -= offset;
        value =  bus_read_halfword(base_address);
      }
      else
      {
        value =  bus_read_halfword(base_address);
        if (up)
          base_address += offset;
        else
          base_address -= offset;
      }
      // sign extension
      uint32_t sign_extension = (value & 0x80) ? 0xFFFFFF00 : 0x00000000;

      REGS(Rd) = sign_extension | value;
      break;

    case LTYPES_LDRSH:
      uint32_t valueh;
      if (pre_indexed)
      {
        if (up)
          base_address += offset;
        else
          base_address -= offset;
        valueh = bus_read_halfword(base_address);
        valueh = valueh >> (rotation_in_word * 8);
      }
      else
      {
        valueh =  bus_read_halfword(base_address);
        if (up)
          base_address += offset;
        else
          base_address -= offset;
      }
      // sign extension
      uint32_t negative_mask = (rotation_in_word) ? 0xFFFFFF00 : 0xFFFF0000;
      uint32_t sign_bit_mask = (rotation_in_word) ? 0x00000080 : 0x00008000;
      uint32_t sign_extensionh = (valueh & sign_bit_mask) ? negative_mask : 0x00000000;

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
          base_address += offset;
        else
          base_address -= offset;
        bus_write_halfword(base_address, REGS(Rd));
      }
      else
      {
        bus_write_halfword(base_address, REGS(Rd));
        if (up)
          base_address += offset;
        else
          base_address -= offset;
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

  if((writeback || !pre_indexed) && !(load && (Rn == Rd))) 
    REGS(Rn) = base_address;

  
}

void arm_mrs(cpu_context *cpu)
{
  uint8_t pos = (cpu->instruction_to_exec >> 22) & 0x1;
  uint8_t Rd = (cpu->instruction_to_exec >> 12) & 0xF;
  printf("mrs\tr%d, %cpsr\n", Rd, pos ? 's' : 'c');

  // implementation
  REGS(Rd) = pos ? *cpu->current_SPSR : cpu->CPSR;
}

#define F_MASK 0xFF000000
#define S_MASK 0x00FF0000
#define X_MASK 0x0000FF00
#define C_MASK 0x000000FF

void arm_msr(cpu_context *cpu)
{
  uint8_t immediate = (cpu->instruction_to_exec >> 25) & 0x1;
  uint8_t psr = (cpu->instruction_to_exec >> 22) & 0x1;
  uint32_t value = cpu->instruction_to_exec & 0x0F0;
  uint16_t shift = (cpu->instruction_to_exec >> 7) & 0x1E;
  uint8_t Rm = cpu->instruction_to_exec & 0xF;

  uint8_t f = (cpu->instruction_to_exec >> 19) & 0x1;
  uint8_t s = (cpu->instruction_to_exec >> 18) & 0x1;
  uint8_t x = (cpu->instruction_to_exec >> 17) & 0x1;
  uint8_t c = (cpu->instruction_to_exec >> 16) & 0x1;


  value += Rm;
  value = (value >> shift) | (value << (32 - shift));

  printf("msr\t%cpsr%c%c%c%c%c, ", psr ? 's' : 'c', (f | s | x | c) ? '_' : '\0',
    f ? 'f' : '\0', s ? 's' : '\0', x ? 'x' : '\0', c ? 'c' : '\0');
  immediate ? printf("#0x%x\n", value) : printf("r%d\n", Rm);


  // Implementation
  uint32_t *psr_ptr;
  if (0 == psr)
    psr_ptr = &cpu->CPSR;
  else
  {
    psr_ptr = cpu->current_SPSR;
    if (psr_ptr == NULL)
    {
      fprintf(stderr, "SPSR pointer to null!\n");
      exit(EXIT_FAILURE);
    }
  }   
  //uint32_t mask = 0x00000000;
  uint32_t mask = 0x00000000;
  if(f) mask |= F_MASK;
  if(c) mask |= C_MASK;

  uint8_t old_mode = *psr_ptr & 0x1F;
  if (immediate)
    *psr_ptr = (*psr_ptr & (~mask)) | (value & mask);
  else
    *psr_ptr = (*psr_ptr & (~mask)) | (REGS(Rm) & mask);
  
  uint8_t new_mode = *psr_ptr & 0x1F;

  if ((old_mode != new_mode))
    switch_mode(cpu, new_mode);
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
    false, false, false, false, false, false,
    true, true,
    false, false,
    true, true, true, true
  };


  // Implementation
  void (*function)(alu_args *) = alu_functions[opcode];
  uint32_t op2;
  alu_args args;
  args.check_carry = false;
  //printf("%s\n", is_logical[opcode] ? "LOGICAL" : "ARITHMETIC");

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
    uint8_t Rm = cpu->instruction_to_exec & 0xF;
    uint32_t val = REGS(Rm);

    //if (Rm == 15) printf("HEREE\n");
    // only 2 reads can be done simultaneously!
    if ((Rm == 15) && shift_by_register) val += 4;
    if ((((cpu->instruction_to_exec >> 16) & 0xF) == 15) && shift_by_register) val += 4;
    
    //if ((Rm == 15) && opcode == ALU_ADD) val += 4;
    switch ((alu_shift_t)((cpu->instruction_to_exec >> 5) & 0x3))
    {
    case LSL:   // optimize this please!!
      if (shift < 32)
      {
        op2 = val << shift;

        if (((cpu->instruction_to_exec >> 20) & 0x1) && (shift != 0) && is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val << (shift - 1)) >> 2);
      }
      else
      {
        op2 = 0;
        if (((cpu->instruction_to_exec >> 20) & 0x1) && is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            (((val & 0x1) && (shift == 32)) << 29);
      }     
      break;

    case LSR:
      if ((shift == 0) && (!shift_by_register))
      {
        op2 = 0;
        if ((cpu->instruction_to_exec >> 20) & 0x1 & is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x80000000) >> 2);
      }
      else if (shift >= 32)
      {
        op2 = 0;
        if ((cpu->instruction_to_exec >> 20) & 0x1 & is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF);
      }
      else
      {
        op2 = val >> shift; 
        if ((cpu->instruction_to_exec >> 20) & 0x1 & is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val >> (shift - 1)) << 29); 
      }
      break;

    case ASR:
      if ((shift == 0) && (!shift_by_register))
      {
        op2 = (val & 0x80000000) ? 0xFFFFFFFF : 0x0;
        if ((cpu->instruction_to_exec >> 20) & 0x1 & is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x80000000) >> 2);
      }
      else
      {
        op2 = (int32_t)(val) >> shift; 
        if ((cpu->instruction_to_exec >> 20) & 0x1 & is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            (((int32_t)(val) >> (shift - 1)) << 29); 
      }
      break;
    
    case ROR:
      if ((shift == 0) && (!shift_by_register))
      {
        op2 = (val >> 1) | ((cpu->CPSR << 2) & 0x80000000);
        if ((cpu->instruction_to_exec >> 20) & 0x1 & is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x1) << 29);
      }
      else if ((shift == 0) && (shift_by_register))
      {
        op2 = val;
        if ((cpu->instruction_to_exec >> 20) & 0x1 & is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x1) << 29);
      }
      else if (shift == 32)
      {
        op2 = val;
        if ((cpu->instruction_to_exec >> 20) & 0x1 & is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            ((val & 0x80000000) >> 2);
      }
      else
      {
        op2 = (val >> shift) | (val << (32 - shift));
        if ((cpu->instruction_to_exec >> 20) & 0x1 & is_logical[opcode])
          cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
            //((val & 0x1) << 29);
            ((op2 & 0x80000000) >> 2);
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
  // if Rd = 15 and is not a tst/teq/cmp/cmn
  if ((args.Rd == 15) && (!((opcode & 0xC) == 0x8)))
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
  int16_t offset = (cpu->thumb_exec << 1) & 0xFFF;
  offset |= (offset & 0x800) ? 0xF000 : 0x0000;

  printf("b\t0x%08x\n", REGS(15) + 2 + offset);
  REGS(15) = ((int32_t)REGS(15) + (int32_t)offset);
  thumb_flush(cpu);
}

void thumb_conditional_branch(cpu_context *cpu)
{
  uint8_t cond = (cpu->thumb_exec >> 8) & 0xF;
  int16_t offset = (cpu->thumb_exec << 1) & 0x1FF;
  offset |= (offset >> 8) ? 0xFE00 : 0x0000;

  printf("b%s\t0x%08x\n", conds[cond], REGS(15) + 2 + offset);

  if (verify_condition(cpu, cond))
  {
    REGS(15) = ((int32_t)REGS(15) + (int32_t)offset);
    thumb_flush(cpu);
  }
  else
    printf("NOT EXECUTED DUE TO UNSATISFIED CONDITION!\n");
  
}


// Writeback with Rb included in Rlist: Store OLD base if Rb is FIRST 
// entry in Rlist, otherwise store NEW base (STM/ARMv4), always store 
// OLD base (STM/ARMv5), no writeback (LDM/ARMv4/ARMv5; at this point, 
// THUMB opcodes work different than ARM opcodes).


// 76543210
// 11100000
// 

void thumb_multiple_load_store(cpu_context *cpu)
{
  printf("MLS\n");
  uint8_t rlist = (uint8_t)cpu->thumb_exec;
  uint8_t Rb = (cpu->thumb_exec >> 8) & 0x7;
  uint8_t load = (cpu->thumb_exec >> 10) & 0x2;

  uint8_t load_valid_rlist = load | (rlist != 0);

  uint8_t base_in_rlist = (rlist >> Rb) & 0x1;
  uint8_t base_first_in_rlist = (rlist & (0xFF >> (8 - Rb))) == 0;    // is zero also if rlist is 0


  switch (load_valid_rlist)
  {
    case 0b11:
      printf("ldmia");
      for (uint8_t i = 0; i < 8; ++i)
      {
        if ((rlist >> i) & 0x1)
        {
          REGS(i) = bus_read_word(REGS(Rb));
          REGS(Rb) += 4;
        }
      }
      break;
    
    case 0b01:
      printf("stmia");
      uint32_t base = REGS(Rb);
      uint32_t real_address;
      for (uint8_t i = 0; i < 8; ++i)
      {
        if ((rlist >> i) & 0x1)
        {
          if (i == Rb)
            real_address = base;
          bus_write_word(base, REGS(i));
          base += 4;
        }
      }
      REGS(Rb) = base;
      if (base_in_rlist && base_first_in_rlist)
      {
        bus_write_word(real_address, real_address);
      }
      if (base_in_rlist && !base_first_in_rlist)
        bus_write_word(real_address, REGS(Rb));
      break;

    case 0b10:
      printf("Load invalid rlist\n");
      printf("ldm");
      REGS(15) = bus_read_word(REGS(Rb));
      REGS(Rb) += 0x40;
      break;
    
    case 0b00:
      printf("Store invalid rlist\n");
      printf("stm");
      bus_write_word(REGS(Rb), REGS(15) + 4);     // due to the pipeline
      REGS(Rb) += 0x40;
      break;
      
  }

  printf("\tr%d!, {", Rb);
  for (uint8_t i = 0; i < 8; ++i)
  {
    if ((rlist >> i) & 0x1)
      printf("r%d ", i);
  }

  printf("}\n");
}

void thumb_long_branch_and_link(cpu_context *cpu)
{
  uint32_t offset = (cpu->thumb_exec) & 0x07FF;

  if ((cpu->thumb_exec >> 11) & 0x1)
  {
    printf("Long bl: temp = ...; PC = LR + 0x%03x << 1;\n", offset);
    // DEBUG HERE!!!
    uint32_t temp = *cpu->regs[15];
    *cpu->regs[15] = *cpu->regs[14] + (offset << 1);
    *cpu->regs[14] = temp | 0x1;
  }
  else
  {
    printf("Long bl: LR = PC + 0x%03x << 12\n", offset);
    *cpu->regs[14] = *cpu->regs[15] + 
      (((offset >> 10) ? 0xFFC00000 : 0x0) | (offset << 12));   // HERE
  }
}

// 0000 0000 0000 0000 0000 0100 0000 0000
// 0000 0000 0000 0000 0000 0111 1111 1111
// 0000 0000 0011 1111 1111 1111 1111 1111
// 0000 0000 0010 0000 0000 0000 0000 0000

void thumb_add_offset_to_sp(cpu_context *cpu)
{
  uint8_t is_negative = (cpu->thumb_exec >> 7) & 0x01;
  uint16_t imm = (cpu->thumb_exec & 0x7F) << 2;

  printf("add\tsp, %c0x%x\n", is_negative ? '-' : '\0', imm);

  REGS(13) += (int32_t)(is_negative ? -1 : 1) * (int32_t) imm;
}

void thumb_push_pop_registers(cpu_context *cpu)
{
  uint8_t rlist = (uint8_t)cpu->thumb_exec;
  uint8_t pc_lr = (cpu->thumb_exec >> 8) & 0x1;
  uint8_t pop = (cpu->thumb_exec >> 11) & 0x1;

  if (pop)
  {
    printf("pop");

    if (pc_lr)
    {
      REGS(15) = bus_read_word(REGS(13));
      REGS(13) +=4;
    }

    for (int8_t i = 7; i >= 0; --i)
    {
      if ((rlist >> i) & 0x1)
      {
        REGS(i) = bus_read_word(REGS(13));
        REGS(13) += 4;
      }
    }

  }
  else
  {
    printf("push");
    for (uint8_t i = 0; i < 8; ++i)
    {
      if ((rlist >> i) & 0x1)
      {
        REGS(13) -= 4;
        bus_write_word(REGS(13), REGS(i));
      }
    }
    if (pc_lr)
    {
      REGS(13) -=4;
      bus_write_word(REGS(13), REGS(14));
    }
  }

  printf("\t{");
  for (uint8_t i = 0; i < 8; ++i)
  {
    if ((rlist >> i) & 0x1)
      printf("r%d ", i);
  }

  if (pc_lr)
    printf((pop) ? "pc " : "lr ");
  printf("}\n");
}

void thumb_load_store_halfword(cpu_context *cpu)
{
  uint8_t Rd = (cpu->thumb_exec) & 0x7;
  uint8_t Rb = (cpu->thumb_exec >> 3) & 0x7;
  uint32_t nn = (cpu->thumb_exec >> 5) & 0x3E;
  uint8_t load = (cpu->thumb_exec >> 11) & 0x1;

  uint32_t address = REGS(Rb) + nn;

  if (load)
  {
    printf("ldrh");
    uint32_t temp = (uint32_t)bus_read_halfword(address & 0xFFFFFFFE);
    if (address & 0x1)
      REGS(Rd) = ((temp >> 8) | (temp << 24));
    else
      REGS(Rd) = temp;

  }
  else
  {
    printf("strh");
    bus_write_halfword(address & 0xFFFFFFFE, (uint16_t)REGS(Rd));
  }

  printf("\tr%d, [r%d, #0x%02x]\n", Rd, Rb, nn);
}

void thumb_sp_relative_load_store(cpu_context *cpu)
{
  uint32_t nn = ((uint8_t)cpu->thumb_exec) << 0x2;
  uint8_t Rd = (cpu->thumb_exec >> 8) & 0x7;
  uint8_t load = (cpu->thumb_exec >> 11) & 0x1;

  uint32_t address = REGS(13) + nn;

  if (load)
  {
    printf("ldr");
    uint32_t temp = bus_read_word(address & 0xFFFFFFFC);
    uint8_t ror = (address & 0x3) * 8;
    REGS(Rd) = ((temp >> ror) | (temp << (32 - ror)));
  }
  else
  {
    printf("str");
    bus_write_word(address & 0xFFFFFFFC, REGS(Rd));
  }

  printf("\tr%d, [sp, #0x%03x]\n", Rd, nn);
}

void thumb_load_address(cpu_context *cpu)
{
  uint8_t Rd = (cpu->thumb_exec >> 8) & 0x7;
  uint8_t sp = (cpu->thumb_exec >> 11) & 0x1;
  uint16_t offset = (cpu->thumb_exec << 2) & 0x03FC; 
  printf("add\tr%d, %s, #0x%x\n", Rd, sp ? "sp" : "pc", offset);

  // Implementation
  void (*function)(alu_args *) = thumb_alu_functions[2];  // add
  alu_args args;
  args.cpu = cpu;
  args.Rd = Rd;
  args.Rn = sp ? 13 : 15;
  args.op2 = offset;
  // consider the pipline
  if (!sp && ((REGS(15) & 0x2) == 0))    // force bit 1 of the PC to 0
    offset += 2;

  args.set_condition_codes = false; // Temporarly
  function(&args);

}

void thumb_load_store_imm_ofs(cpu_context *cpu)
{
  uint8_t Rd = (cpu->thumb_exec) & 0x7;
  uint8_t Rb = (cpu->thumb_exec >> 3) & 0x7;
  uint32_t nn = (cpu->thumb_exec >> 6) & 0x1F;
  uint8_t opcode = (cpu->thumb_exec >> 11) & 0x3;

  if (!(opcode >> 1))
    nn = nn << 2;
  
  uint32_t address = REGS(Rb) + nn;
  //uint8_t flag = address & 0x1;
  //address &= 0xFFFFFFFE;

  switch (opcode)
  {
    case 0:
      printf("str");
      bus_write_word(address & 0xFFFFFFFC, REGS(Rd));
      break;

    case 1:
      printf("ldr");
      uint32_t temp = bus_read_word(address & 0xFFFFFFFC);
      uint8_t ror = (address & 0x3) * 8;
      REGS(Rd) = ((temp >> ror) | (temp << (32 - ror)));
      break;

    case 2:
      printf("strb");
      bus_write(address, (uint8_t)REGS(Rd));
      break;

    case 3:
      printf("ldrb");
      REGS(Rd) = (uint32_t)bus_read(address);
      break;
  }

  printf("\tr%d, [r%d, #0x%02x]\n", Rd, Rb, nn);
}

void thumb_load_store_reg_ofs(cpu_context *cpu)
{
  uint8_t Rd = cpu->thumb_exec & 0x7;
  uint8_t Rb = (cpu->thumb_exec >> 3) & 0x7;
  uint8_t Ro = (cpu->thumb_exec >> 6) & 0x7;
  uint8_t byte = (cpu->thumb_exec >> 10) & 0x1;
  uint8_t load = (cpu->thumb_exec >> 11) & 0x1;

  printf("%s%c\tr%d, [r%d, r%d]\n", load ? "ldr" : "str", byte ? 'b' : ' ', Rd, Rb, Ro);

  uint32_t address = (REGS(Rb) + REGS(Ro)); 
  //address -= (address & 0x1) << 2;                       // halfword alignement (?)
  uint8_t flag = (uint8_t)address & 0x1;
  address &= 0xFFFFFFFE;
  uint32_t temp;

  if (byte)
  {
    if(load)
    {
      temp = (uint32_t)((int32_t)bus_read(address));
      if (flag)
        REGS(Rd) = ((temp >> 8) | (temp << 24));
      else
        REGS(Rd) = temp;
    }
    else
      bus_write(address, (uint8_t)REGS(Rd));
  }
  else
  {
    if(load)
    {
      temp = (uint32_t)((int32_t)bus_read_word(address));
      if (flag)
        REGS(Rd) = ((temp >> 8) | (temp << 24));
      else
        REGS(Rd) = temp;
    }
    else
      bus_write_word(address, (uint32_t)REGS(Rd));
  }

}

void thumb_load_store_sign_ext_b_h(cpu_context *cpu)
{
  uint8_t Rd = cpu->thumb_exec & 0x7;
  uint8_t Rb = (cpu->thumb_exec  >> 3) & 0x7;
  uint8_t Ro = (cpu->thumb_exec  >> 6) & 0x7;
  uint8_t opcode = (cpu->thumb_exec >> 10) & 0x3;

  uint32_t address = REGS(Rb) + REGS(Ro);

  uint8_t flag = (uint8_t)address & 0x1;
  address &= 0xFFFFFFFE;

  uint32_t temp;

  switch (opcode)
  {
    case 0:
      printf("strh");
      bus_write_halfword(address, (uint16_t)REGS(Rd));
      break;
    
    case 1:
      printf("ldsb");
      temp = (uint32_t)bus_read(address);
      REGS(Rd) = (temp | ((temp >> 7) ? 0xFFFFFF00 : 0x00000000));
      break;
    
    case 2:
      printf("ldrh");
      temp = (uint32_t)bus_read_halfword(address);
      if (flag)
        REGS(Rd) = ((temp >> 8) | (temp << 24));
      else
        REGS(Rd) = temp;
      break;

    case 3:
      printf("ldsh");
      temp = (uint32_t)bus_read_halfword(address);
      if (flag)
      {   // TO CHECK
        temp = ((temp >> 8) | (temp << 24));
        REGS(Rd) = (temp | ((temp >> 7) ? 0xFFFFFF00 : 0x00000000));
        //REGS(Rd) = ((temp >> 8) | (temp << 24));
      }
      else
      {
        REGS(Rd) = (temp | ((temp >> 15) ? 0xFFFF0000 : 0x00000000));
      }

      break;
  }

  printf("\tr%d, [r%d, r%d]\n", Rd, Rb, Ro);
}

void thumb_pc_relative_load (cpu_context *cpu)
{
  uint8_t Rd = (cpu->thumb_exec >> 8) & 0x7;
  int16_t imm = (((cpu->thumb_exec & 0xFF) << 2) | (((cpu->thumb_exec >> 7) & 0x1) ? 0xFC00 : 0));
  printf("ldr\tr%d, [pc, %d]\n", Rd, imm);

  REGS(Rd) = bus_read_word((int32_t)REGS(15) + (int32_t)imm);
}

void thumb_hi_regs_ops_bx(cpu_context *cpu)
{
  uint8_t opcode = (cpu->thumb_exec >> 8) & 0x3;
  uint8_t Rd = (cpu->thumb_exec & 0x7) | ((cpu->thumb_exec >> 4)) & 0x8;
  uint8_t Rs = (cpu->thumb_exec >> 3) & 0xF;

  switch (opcode)
  {
  case 0x0:   // Add
    printf("add\tR%d, R%d\n", Rd, Rs);
    uint32_t a = REGS(Rd);
    uint32_t b = REGS(Rs);
    b += (15 == Rs) ? 2 : 0;
    uint32_t result = a + b;
    REGS(Rd) = result;

    break;
  
  case 0x1:
    printf("cmp\tR%d, R%d\n", Rd, Rs);
    a = REGS(Rd);
    b = REGS(Rs);
    printf("Computing 0x%08x - 0x%08x\n", a, b);

    result = a - b;

    // Set cpsr flags
    cpu->CPSR = (cpu->CPSR & 0x7FFFFFFF) |
      (result & 0x80000000);

    cpu->CPSR = (cpu->CPSR & 0xBFFFFFFF) |
      ((result == 0) << 30);

    cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
      (((a >= b)) << 29);

    cpu->CPSR = (cpu->CPSR & 0xEFFFFFFF) |
      (((int32_t)((a ^ b) & (result ^ a)) < 0) << 28);
    break;
  
  case 0x2:
    printf("mov\tR%d, R%d\n", Rd, Rs);
    REGS(Rd) = REGS(Rs) + ((15 == Rs) ? 2 : 0);
    
    if ((15 == Rd))   // halfword alignment
    {
      REGS(15) &= 0xFFFFFFFE;
    }

    break;
  
  case 0x3:
    printf("b%sx\tR%d\n", (Rd & 0x8) ? "l" : "" , Rs);
    REGS(15) = ((REGS(Rs)) & 0xFFFFFFFC) - 2; // -2
    cpu->CPSR = (cpu->CPSR & 0xFFFFFFDF) | ((REGS(Rs) & 0x1) << 5); // MARK HERE
    printf("FLUSHING THE PIPELINE\n");
    flush(cpu);
    thumb_flush(cpu);
    break;
  }

}

void thumb_alu_operations(cpu_context *cpu)
{
  const char *ops[] =
  {
    "and", "eor", "lsl", "lsr",
    "asr", "adc", "sbc", "ror",
    "tst", "neg", "cmp", "cmn",
    "orr", "mul", "bic", "mvn"
  };

  uint8_t opcode = (cpu->thumb_exec >> 6) & 0xF;
  uint8_t Rs = (cpu->thumb_exec >> 3) & 0x7;
  uint8_t Rd = cpu->thumb_exec & 0x7;

  printf("%s ...\n", ops[opcode]);

  void (*function)(alu_args *) = thumb_alu_functions_complete[opcode];
  uint32_t op2;
  alu_args args;

  args.Rd = Rd;
  args.Rn = Rd;
  args.op2 = *cpu->regs[Rs];
  args.set_condition_codes = true;
  args.cpu = cpu;

  function(&args);

  if ((15 == Rd))   // halfword alignment
    REGS(15) &= 0xFFFFFFFE;
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
  args.Rn = Rd;
  args.op2 = nn;
  args.set_condition_codes = true; // Temporarly
  function(&args);
}

void thumb_add_sub(cpu_context *cpu)
{
  uint8_t Rd = cpu->thumb_exec & 0x7;
  uint8_t Rs = (cpu->thumb_exec >> 0x3) & 0x7;
  uint8_t Rn = (cpu->thumb_exec >> 0x6) & 0x7;
  
  printf("%s\tR%d, R%d, %c%d\n", (cpu->thumb_exec & 0x200) ? 
    "sub" : "add", Rd, Rs, (cpu->thumb_exec & 0x400) ? 
    '#' : 'R', Rn);
  
  // Implementation
  alu_args args;
  args.cpu = cpu;
  args.set_condition_codes = true;
  args.Rd = Rd;
  args.Rn = Rs;

  if (cpu->thumb_exec & 0x400)
    args.op2 = Rn;
  else
    args.op2 = REGS(Rn);

  if (cpu->thumb_exec & 0x200)
    alu_sub(&args);
  else
    alu_add(&args);
}

void thumb_mov_shifted_regs(cpu_context *cpu)
{
  uint8_t opcode = (cpu->thumb_exec >> 11) & 0x3;
  uint8_t Rd = cpu->thumb_exec & 0x7;
  uint8_t Rn = (cpu->thumb_exec >> 3) & 0x7;
  uint8_t shift = (cpu->thumb_exec >> 6) & 0x1F;

  const char *s_types[] =
  {
    "lsl", "lsr", "asr", "INVALID"
  };

  printf("%s\tR%d, R%d, #%d\n", s_types[opcode], Rd, Rn, shift);

  uint32_t op2;
  uint32_t val = REGS(Rn);
  switch (opcode)
  {
  case LSL: // lsl
    op2 = val << shift;

    if (shift != 0)
      cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
        ((val << (shift - 1)) >> 2);
    break;
  
  case LSR: // lsr

    if (shift != 0)
    {
      op2 = val >> shift;
      cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
        ((val >> (shift - 1)) << 29);
    }
    else
    {
      op2 = 0;
      cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
        ((val & 0x80000000) >> 2);
    }
    break;
  

  case ASR: // asr

    if (shift != 0)
    {
      op2 = (int32_t)(val) >> shift;
      cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
        (((int32_t)(val) >> (shift - 1)) << 29); 
    }
    else
    {
      op2 = (val & 0x80000000) ? 0xFFFFFFFF : 0x0;
      cpu->CPSR = (cpu->CPSR & 0xDFFFFFFF) |
        ((val & 0x80000000) >> 2);
    }
    break;
  
    
  default:
    fprintf(stderr, "Invalid opcode in mov shifted regs instruction!\n");
    exit(EXIT_FAILURE);
  }

  REGS(Rd) = op2;

  cpu->CPSR = (cpu->CPSR & 0x7FFFFFFF) |
    (op2 & 0x80000000);

  cpu->CPSR = (cpu->CPSR & 0xBFFFFFFF) |
    ((op2 == 0) << 30);

  
  if ((15 == Rd))   // halfword alignment
    REGS(15) &= 0xFFFFFFFE;
}


void thumb_flush(cpu_context *cpu)
{
  cpu->thumb_decode   = THUMB_NOP;
  cpu->thumb_fetch    = THUMB_NOP;
}

