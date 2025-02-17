#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bus.h"
#include "cartridge.h"

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

#define NO_IMPL { fprintf(stderr, "NOT YET IMPLEMENTED\n"); exit(-5); }


uint8_t bus_read(uint32_t address)
{
  // Read on the ROM
  if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    return cartridge_read_byte((uint16_t)address);
  }

  NO_IMPL
}


void bus_write(uint32_t address, uint8_t value)
{
  if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    cartridge_write_byte((uint16_t)address, value);
    return;
  }
  
  NO_IMPL
}


uint16_t bus_read_halfword(uint32_t address)
{
  // Read on the ROM
  if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    return cartridge_read_halfword((uint16_t)address);
  }
}

uint32_t bus_read_word(uint32_t address)
{
  // Read on the ROM
  if (address >= 0x08000000 && address <= 0x0DFFFFFF)
  {
    return cartridge_read_word((uint16_t)address);
  }
}
