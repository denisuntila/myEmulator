#ifndef HH_BUS_HH
#define HH_BUS_HH

#include <stdint.h>
#include <stdbool.h>

uint8_t bus_read(uint32_t address);
void bus_write(uint32_t address, uint8_t value);

uint16_t bus_read_halfword(uint32_t address);
void bus_write_halfword(uint32_t address, uint16_t value);

uint32_t bus_read_word(uint32_t address);
void bus_write_word(uint32_t address, uint32_t value);

#endif