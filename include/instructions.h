#ifndef HH_INSTRUCTIONS_HH
#define HH_INSTRUCTIONS_HH

#include <stdint.h>
#include "cpu.h"


void (*decode_instruction(uint32_t instruction))(cpu_context *);

#endif