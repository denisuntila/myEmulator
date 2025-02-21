#ifndef HH_CARTRIDGE_HH
#define HH_CARTRIDGE_HH

#include <stdint.h>
#include <stdbool.h>


typedef struct
{
  uint8_t entry[4];
  uint8_t logo[156];

  char title[12];
  uint8_t game_code[4];
  uint8_t maker_code[2];
  uint8_t fixed_value;
  uint8_t main_unit_code;
  uint8_t device_type;
  
  uint8_t software_version;
  uint8_t complement_check;
} rom_header;

bool load_cartridge(char *file_name);
void dealloc_cartridge();
uint8_t cartridge_read_byte(uint32_t address);
void cartridge_write_byte(uint32_t address, uint8_t value);

uint32_t cartridge_read_word(uint32_t address);
void cartridge_write_word(uint32_t address, uint32_t value);

uint16_t cartridge_read_halfword(uint32_t address);
void cartridge_write_halfword(uint32_t address, uint16_t value);

#endif