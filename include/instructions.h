#ifndef HH_INSTRUCTIONS_HH
#define HH_INSTRUCTIONS_HH

#include <stdint.h>

typedef enum
{
  ARM_DP
} arm_instruction_type;

typedef enum
{
  THUMB
} thumb_instruction_type;

void (*decode_instruction(uint32_t instruction))(uint32_t);


#endif