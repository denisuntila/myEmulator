#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cartridge.h"


typedef struct
{
  char file_name[512];
  uint32_t rom_size;
  uint8_t *rom_data;
  rom_header *header;
} cartridge;

static cartridge cart;

bool load_cartridge(char *file_name)
{
  // load the file in memory
  snprintf(cart.file_name, sizeof(cart.file_name), "%s", file_name);

  FILE *file_ptr = fopen(file_name, "r");

  if (!file_ptr)
  {
    printf("Failed to open: %s\n", file_name);
    return false;
  }

  printf("Opened: %s\n", cart.file_name);

  fseek(file_ptr, 0, SEEK_END);
  cart.rom_size = ftell(file_ptr);

  rewind(file_ptr);

  cart.rom_data = malloc(cart.rom_size);
  fread(cart.rom_data, cart.rom_size, 1, file_ptr);
  fclose(file_ptr);

  cart.header = (rom_header *)(cart.rom_data);
  cart.header->title[11] = 0;

  printf("Cartridge loaded:\n");
  printf("\tTitle       : %s\n", cart.header->title);
  printf("\tUnique code : %c\n", cart.header->game_code[0]);
  printf("\tShort title : %c%c\n", cart.header->game_code[1], 
    cart.header->game_code[2]);
  printf("\tLanguage    : %c\n", cart.header->game_code[3]);



  // Checksum
  uint32_t chk = 0;
  for(uint16_t i = 0xA0; i <= 0xBC; ++i)
  {
    chk = chk - cart.rom_data[i] - 1;
  }

  printf("\tChecksum    : %2.2X (%s)\n", cart.header->complement_check,
    (chk & 0xFF) ? "PASSED" : "FAILED");

  printf("\n");
  return true;
}

void dealloc_cartridge()
{
  free(cart.rom_data);
}


uint8_t cartridge_read_byte(uint32_t address)
{
  return cart.rom_data[address];
}

void cartridge_write_byte(uint32_t address, uint8_t value)
{
  // Not implemented
}

uint32_t cartridge_read_word(uint32_t address)
{
  return *((uint32_t *)&cart.rom_data[address]);
}

void cartridge_write_word(uint32_t address, uint32_t value)
{
  // Not implemented
}

uint16_t cartridge_read_halfword(uint32_t address)
{
  return *((uint16_t *)&cart.rom_data[address]);
}

void cartridge_write_halfword(uint32_t address, uint16_t value)
{
  // Not implemented
}


