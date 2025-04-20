#ifndef HH_INSTRUCTIONS_HH
#define HH_INSTRUCTIONS_HH

#include <stdint.h>
#include "cpu.h"


void (*decode_instruction(uint32_t instruction))(cpu_context *);
bool verify_condition(cpu_context *cpu, uint8_t cond);
void flush(cpu_context *cpu);

void (*thumb_decode_instruction(uint16_t instruction))(cpu_context *);
void thumb_flush(cpu_context *cpu);

#endif