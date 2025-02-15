#ifndef HH_BUS_HH
#define HH_BUS_HH

#include <stdint.h>
#include <stdbool.h>

uint8_t bus_read(uint32_t address);
void bus_write(uint32_t address, uint8_t value);

#endif