#include <stdint.h>
#include <stdio.h>
#include "instructions.h"



static const uint32_t instruction_type_format_masks[][2] = 
{
  {0x012FFF10, 0x0FFFFFF0},           // BE format
  {0x08000000, 0x0E000000},           // Block data transfer
  {0x0A000000, 0x0F000000},           // B and BL
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


void arm_branch_and_exchange      (uint32_t instruction);
void arm_block_data_transfer      (uint32_t instruction);
void arm_branch_branch_link       (uint32_t instruction);
void arm_software_interrupt       (uint32_t instruction);
void arm_undefined                (uint32_t instruction);
void arm_single_data_transfer     (uint32_t instruction);
void arm_single_data_swap         (uint32_t instruction);
void arm_multiply                 (uint32_t instruction);
void arm_multiply_long            (uint32_t instruction);
void arm_halfword_transfer        (uint32_t instruction);
void arm_halfword_transfer_imm    (uint32_t instruction);
void arm_mrs                      (uint32_t instruction);
void arm_msr                      (uint32_t instruction);
void arm_data_processing          (uint32_t instruction);


static void (*functions[])(uint32_t) =
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


void (*decode_instruction(uint32_t instruction))(uint32_t)
{
  for (uint8_t i = 0; i < 14; ++i)
  {
    const uint32_t *current_inst = instruction_type_format_masks[i];
    
    if((instruction & current_inst[1]) == current_inst[0])
    {
      return functions[i];
    }
  }
  printf("Not implemented!\n");
  return NULL;
}


void arm_branch_and_exchange(uint32_t instruction)
{
  printf("Branch and Exchange\n");
}


void arm_block_data_transfer(uint32_t instruction)
{
  printf("Block data transfer\n");
}

void arm_branch_branch_link(uint32_t instruction)
{
  printf("Branch or Branch and link\n");
}

void arm_software_interrupt(uint32_t instruction)
{
  printf("Software interrupt\n");
}

void arm_undefined(uint32_t instruction)
{
  printf("Instruction undefined\n");
}

void arm_single_data_transfer(uint32_t instruction)
{
  printf("Single data transfer\n");
}

void arm_single_data_swap(uint32_t instruction)
{
  printf("Single data swap\n");
}

void arm_multiply(uint32_t instruction)
{
  printf("Multiply\n");
}

void arm_multiply_long(uint32_t instruction)
{
  printf("Multiply long\n");
}

void arm_halfword_transfer(uint32_t instruction)
{
  printf("Halfword data transfer\n");
}

void arm_halfword_transfer_imm(uint32_t instruction)
{
  printf("Halfword data transfer immediate\n");
}

void arm_mrs(uint32_t instruction)
{
  printf("MRS\n");
}

void arm_msr(uint32_t instruction)
{
  printf("MSR\n");
}

void arm_data_processing(uint32_t instruction)
{
  printf("Data processing\n");
}

