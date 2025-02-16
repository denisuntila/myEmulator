#include <stdint.h>
#include <stdio.h>
#include "instructions.h"
#include "cpu.h"



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
  printf("Instruction 0x%08x is not implemnted!\n", cpu->current_instruction);
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


void arm_branch_and_exchange(cpu_context *cpu)
{
  uint8_t Rn = cpu->current_instruction & 0x0F;
  printf("bx \tr%d\n", Rn);
}


void arm_block_data_transfer(cpu_context *cpu)
{
  printf("Block data transfer\n");
}

void arm_branch_branch_link(cpu_context *cpu)
{
  uint8_t L = (cpu->current_instruction >> 24) & 1;
  uint32_t offset = (cpu->current_instruction & 0xFFFFFF) << 2;
  printf("%s \t#%d\n", (L ? "bl" : "b"), offset);
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
  uint8_t Rn = (cpu->current_instruction >> 16) & 0xF;
  uint8_t Rd = (cpu->current_instruction >> 12) & 0xF;
  uint16_t offset = cpu->current_instruction & 0xFFF;
  uint8_t I = (cpu->current_instruction >> 25) & 1;
  uint8_t pre_indexed = (cpu->current_instruction >> 24) & 1;
  uint8_t up = (cpu->current_instruction >> 23) & 1;
  uint8_t byte = (cpu->current_instruction >> 22) & 1;
  uint8_t writeback = (cpu->current_instruction >> 21) & 1;
  uint8_t load = (cpu->current_instruction >> 24) & 1;
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
  uint8_t byte = (cpu->current_instruction >> 22) & 0x1;
  uint8_t Rn = (cpu->current_instruction >> 16) & 0xF;
  uint8_t Rd = (cpu->current_instruction >> 12) & 0xF;
  uint8_t Rm = cpu->current_instruction & 0xF;
  printf("swp%c\tr%d, r%d, [r%d]\n", byte ? 'b' : '\0',
    Rd, Rm, Rn);
}

void arm_multiply(cpu_context *cpu)
{
  uint8_t accumulate = (cpu->current_instruction >> 21) & 0x1;
  uint8_t set_condition_codes = (cpu->current_instruction >> 20) & 0x1;
  uint8_t Rd = (cpu->current_instruction >> 16) & 0xF;
  uint8_t Rn = (cpu->current_instruction >> 12) & 0xF;
  uint8_t Rs = (cpu->current_instruction >> 8) & 0xF;
  uint8_t Rm = (cpu->current_instruction) & 0xF;
  printf("%s%c\tr%d, r%d, r%d",
    accumulate ? "mla" : "mul", set_condition_codes ? 's' : '\0',
    Rd, Rm, Rs);
  
  accumulate ? printf(", r%d\n", Rn) : printf("\n");
}

void arm_multiply_long(cpu_context *cpu)
{
  uint8_t is_unsigned = (cpu->current_instruction >> 22) & 0x1;
  uint8_t accumulate = (cpu->current_instruction >> 21) & 0x1;
  uint8_t set_condition_codes = (cpu->current_instruction >> 20) & 0x1;
  uint8_t Rd = (cpu->current_instruction >> 16) & 0xF;
  uint8_t Rn = (cpu->current_instruction >> 12) & 0xF;
  uint8_t Rs = (cpu->current_instruction >> 8) & 0xF;
  uint8_t Rm = (cpu->current_instruction) & 0xF;
  printf("%c%s%c\tr%d, r%d, r%d, r%d\n",
    is_unsigned ? 'u' : 's', accumulate ? "mlal" : "mull", 
    set_condition_codes ? 's' : '\0', Rn, Rd, Rm, Rs);

}

void arm_halfword_transfer(cpu_context *cpu)
{
  printf("Halfword data transfer\n");
}

void arm_halfword_transfer_imm(cpu_context *cpu)
{
  printf("Halfword data transfer immediate\n");
}

void arm_mrs(cpu_context *cpu)
{
  printf("MRS\n");
}

void arm_msr(cpu_context *cpu)
{
  printf("MSR\n");
}

void arm_data_processing(cpu_context *cpu)
{
  printf("Data processing\n");
}

