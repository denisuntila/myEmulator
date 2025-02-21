#include "bios.h"

#include <stdio.h>



uint8_t bios_data[16384];
unsigned int bios_size;

bool load_bios(char *file_name)
{
  FILE *file_ptr = fopen(file_name, "r");

  if (!file_ptr)
  {
    printf("Failed to open: %s\n", file_name);
    return false;
  }

  printf("Opened: %s\n", file_name);

  fseek(file_ptr, 0, SEEK_END);
  bios_size = ftell(file_ptr);

  rewind(file_ptr);

  fread(bios_data, bios_size, 1, file_ptr);
  fclose(file_ptr);
}



uint8_t bios_read_byte(uint32_t address)
{
  return bios_data[address];
}

uint16_t bios_read_halfword(uint32_t address)
{
  return *((uint16_t *)&bios_data[address]);
}

uint32_t bios_read_word(uint32_t address)
{
  return *((uint32_t *)&bios_data[address]);
}


