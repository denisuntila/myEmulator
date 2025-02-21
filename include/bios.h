#ifndef HH_BIOS_HH
#define HH_BIOS_HH

#include <stdint.h>
#include <stdbool.h>


bool load_bios(char *file_name);
uint8_t bios_read_byte(uint32_t address);
uint16_t bios_read_halfword(uint32_t address);
uint32_t bios_read_word(uint32_t address);


#endif