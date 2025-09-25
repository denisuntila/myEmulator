#ifndef HH_ALU_HH
#define HH_ALU_HH

#include <stdint.h>
#include <stdbool.h>

#include "cpu.h"

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


typedef enum
{
  LSL,
  LSR,
  ASR,
  ROR

}alu_shift_t;


typedef struct
{
  cpu_context *cpu;
  bool set_condition_codes;
  uint8_t Rd;
  uint8_t Rn;
  
  uint32_t op2;

  uint32_t check_carry_value;
  uint32_t non_shifted_value;
  bool check_carry;
}alu_args;


void alu_and(alu_args *args);
void alu_eor(alu_args *args);
void alu_sub(alu_args *args);
void alu_rsb(alu_args *args);
void alu_add(alu_args *args);
void alu_adc(alu_args *args);
void alu_sbc(alu_args *args);
void alu_rsc(alu_args *args);
void alu_tst(alu_args *args);
void alu_teq(alu_args *args);
void alu_cmp(alu_args *args);
void alu_cmn(alu_args *args);
void alu_orr(alu_args *args);
void alu_mov(alu_args *args);
void alu_bic(alu_args *args);
void alu_mvn(alu_args *args);


// For thumb instructions:
void alu_lsl(alu_args *args);
void alu_lsr(alu_args *args);
void alu_asr(alu_args *args);
void alu_ror(alu_args *args);
void alu_neg(alu_args *args);
void alu_mul(alu_args *args);

// I didn't find a more elegant way to do it
// without rewriting all the instructions :))
void alu_add_thumb(alu_args *args);


static void (*alu_functions[])(alu_args *) =
{
  &alu_and,
  &alu_eor,
  &alu_sub,
  &alu_rsb,
  &alu_add,
  &alu_adc,
  &alu_sbc,
  &alu_rsc,
  &alu_tst,
  &alu_teq,
  &alu_cmp,
  &alu_cmn,
  &alu_orr,
  &alu_mov,
  &alu_bic,
  &alu_mvn
};


static void (*thumb_alu_functions[])(alu_args *) =
{
  &alu_mov,
  &alu_cmp,
  &alu_add_thumb,
  &alu_sub
};



static void (*thumb_alu_functions_complete[])(alu_args *) = 
{
  &alu_and,
  &alu_eor,
  &alu_lsl,
  &alu_lsr,
  &alu_asr,
  &alu_adc,
  &alu_sbc,
  &alu_ror,
  &alu_tst,
  &alu_neg,
  &alu_cmp,
  &alu_cmn,
  &alu_orr,
  &alu_mul,
  &alu_bic,
  &alu_mvn
};



#endif