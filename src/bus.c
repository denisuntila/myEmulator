#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bus.h"
#include "cartridge.h"
#include "bios.h"

//General Internal Memory
//
//  00000000-00003FFF   BIOS - System ROM         (16 KBytes)
//  00004000-01FFFFFF   Not used
//  02000000-0203FFFF   WRAM - On-board Work RAM  (256 KBytes) 2 Wait
//  02040000-02FFFFFF   Not used
//  03000000-03007FFF   WRAM - On-chip Work RAM   (32 KBytes)
//  03008000-03FFFFFF   Not used
//  04000000-040003FE   I/O Registers
//  04000400-04FFFFFF   Not used
//
//Internal Display Memory
//
//  05000000-050003FF   BG/OBJ Palette RAM        (1 Kbyte)
//  05000400-05FFFFFF   Not used
//  06000000-06017FFF   VRAM - Video RAM          (96 KBytes)
//  06018000-06FFFFFF   Not used
//  07000000-070003FF   OAM - OBJ Attributes      (1 Kbyte)
//  07000400-07FFFFFF   Not used
//
//External Memory (Game Pak)
//
//  08000000-09FFFFFF   Game Pak ROM/FlashROM (max 32MB) - Wait State 0
//  0A000000-0BFFFFFF   Game Pak ROM/FlashROM (max 32MB) - Wait State 1
//  0C000000-0DFFFFFF   Game Pak ROM/FlashROM (max 32MB) - Wait State 2
//  0E000000-0E00FFFF   Game Pak SRAM    (max 64 KBytes) - 8bit Bus width
//  0E010000-0FFFFFFF   Not used
//
//Unused Memory Area
//
//  10000000-FFFFFFFF   Not used (upper 4bits of address bus unused)
//

#define NO_IMPL { fprintf(stderr, "NOT YET IMPLEMENTED: BUS\n"); exit(-5); }


uint8_t on_board_wram[262144];    // 256  KB
uint8_t on_chip_wram[32768];      // 32   kB


uint8_t read_ob_wram_byte(uint32_t address);
uint16_t read_ob_wram_halfword(uint32_t address);
uint32_t read_ob_wram_word(uint32_t address);

void write_ob_wram_byte(uint32_t address, uint8_t value);
void write_ob_wram_halfword(uint32_t address, uint16_t value);
void write_ob_wram_word(uint32_t address, uint32_t value);


uint8_t read_oc_wram_byte(uint32_t address);
uint16_t read_oc_wram_halfword(uint32_t address);
uint32_t read_oc_wram_word(uint32_t address);

void write_oc_wram_byte(uint32_t address, uint8_t value);
void write_oc_wram_halfword(uint32_t address, uint16_t value);
void write_oc_wram_word(uint32_t address, uint32_t value);


uint8_t bus_read(uint32_t address)
{
  if (address <= 0x00003FFF)
  {
    return bios_read_byte(address);
  }
  else if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    address &= 0x07FFFFFF;
    return cartridge_read_byte(address);
  }
  else if (address >= 0x02000000 && address <= 0x0203FFFF)
  {
    address &= 0x0003FFFF;
    return read_ob_wram_byte(address);
  }
  else if (address >= 0x03000000 && address <= 0x03007FFF)
  {
    address &= 0x00007FFF;
    return read_oc_wram_byte(address);
  }
  else
  {
    printf("Unused address\n");
    return 0;
  }
  

  NO_IMPL
}


void bus_write(uint32_t address, uint8_t value)
{
  if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    address &= 0x07FFFFFF;
    cartridge_write_byte(address, value);
    return;
  }
  else if (address >= 0x02000000 && address <= 0x0203FFFF)
  {
    address &= 0x0003FFFF;
    write_ob_wram_byte(address, value);
    return;
  }
  else if (address >= 0x03000000 && address <= 0x03007FFF)
  {
    address &= 0x00007FFF;
    write_oc_wram_byte(address,value);
    return;
  }
  
  NO_IMPL
}


uint16_t bus_read_halfword(uint32_t address)
{
  // Read on the ROM
  if (address <= 0x00003FFF)
  {
    return bios_read_halfword(address);
  }
  else if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    address &= 0x07FFFFFF;
    return cartridge_read_halfword(address);
  }
  else if (address >= 0x02000000 && address <= 0x0203FFFF)
  {
    address &= 0x0003FFFF;
    return read_ob_wram_halfword(address);
  }
  else if (address >= 0x03000000 && address <= 0x03007FFF)
  {
    address &= 0x00007FFF;
    return read_oc_wram_halfword(address);
  }

  NO_IMPL
}


void bus_write_halfword(uint32_t address, uint16_t value)
{
  if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    address &= 0x07FFFFFF;
    cartridge_write_halfword(address, value);
    return;
  }
  else if (address >= 0x02000000 && address <= 0x0203FFFF)
  {
    address &= 0x0003FFFF;
    write_ob_wram_halfword(address, value);
    return;
  }
  else if (address >= 0x03000000 && address <= 0x03007FFF)
  {
    address &= 0x00007FFF;
    write_oc_wram_halfword(address,value);
    return;
  }
  else if (address >= 0x04000000 && address <= 0x040003FF)
  {
    printf("Write 0x%04x to 0x%08x (IO registers)\n", value, address);
    return;
  }
  else if (address >= 0x05000000 && address <= 0x050003FF)
  {
    printf("Write 0x%04x to 0x%08x (OBJ/BG vram)\n", value, address);
    return;
  }
  NO_IMPL
}


uint32_t bus_read_word(uint32_t address)
{
  if (address <= 0x00003FFF)
  {
    return bios_read_word(address);
  }
  else if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    address &= 0x07FFFFFF;
    return cartridge_read_word(address);
  }
  else if (address >= 0x02000000 && address <= 0x0203FFFF)
  {
    address &= 0x0003FFFF;
    return read_ob_wram_word(address);
  }
  else if (address >= 0x03000000 && address <= 0x03007FFF)
  {
    address &= 0x00007FFF;
    return read_oc_wram_word(address);
  }
  else
  {
    printf("Unused address\n");
    return 0;
  }

  NO_IMPL
}

void bus_write_word(uint32_t address, uint32_t value)
{
  if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    address &= 0x07FFFFFF;
    cartridge_write_word(address, value);
    return;
  }
  else if (address >= 0x02000000 && address <= 0x0203FFFF)
  {
    address &= 0x0003FFFF;
    write_ob_wram_word(address, value);
    return;
  }
  else if (address >= 0x03000000 && address <= 0x03007FFF)
  {
    address &= 0x00007FFF;
    write_oc_wram_word(address,value);
    return;
  }
  
  NO_IMPL
}





uint8_t read_ob_wram_byte(uint32_t address)
{
  return on_board_wram[address];
}

uint16_t read_ob_wram_halfword(uint32_t address)
{
  return *((uint16_t *)&on_board_wram[address]);
}

uint32_t read_ob_wram_word(uint32_t address)
{
  return *((uint32_t *)&on_board_wram[address]);
}


void write_ob_wram_byte(uint32_t address, uint8_t value)
{
  on_board_wram[address] = value;
}

void write_ob_wram_halfword(uint32_t address, uint16_t value)
{
  *((uint16_t *)&on_board_wram[address]) = value;
}

void write_ob_wram_word(uint32_t address, uint32_t value)
{
  *((uint32_t *)&on_board_wram[address]) = value;
}



uint8_t read_oc_wram_byte(uint32_t address)
{
  return on_chip_wram[address];
}

uint16_t read_oc_wram_halfword(uint32_t address)
{
  return *((uint16_t *)&on_chip_wram[address]);
}

uint32_t read_oc_wram_word(uint32_t address)
{
  return *((uint32_t *)&on_chip_wram[address]);
}


void write_oc_wram_byte(uint32_t address, uint8_t value)
{
  on_chip_wram[address] = value;
}

void write_oc_wram_halfword(uint32_t address, uint16_t value)
{
  *((uint16_t *)&on_chip_wram[address]) = value;
}

void write_oc_wram_word(uint32_t address, uint32_t value)
{

  *((uint32_t *)&on_chip_wram[address]) = value;
}

