# myEmulator

A **Game Boy Advance (GBA) emulator** written in C.  
This project is developed as a **hobby and educational exercise**, focusing on the emulation of the **ARM7TDMI architecture** and the core hardware components of the console.  

It is not meant to compete with existing GBA emulators, but rather to serve as a learning tool and exploration of emulator development.  

---

# References

- [GBATEK](https://problemkaputt.de/gbatek.htm) – Complete GBA hardware reference
- [ARM7TDMI Technical Reference Manual](https://developer.arm.com/documentation/ddi0210/latest)

# Testing

- Test ROMs used: [gba-tests repo](https://github.com/jsmolka/gba-tests.git)

---


# Build & Dependencies

This project is written in **C** and uses **SDL2** for rendering (WIP).

## Requirements
- C compiler (tested with GCC/Clang)
- [CMake](https://cmake.org/)
- [SDL2](https://www.libsdl.org/) development libraries

## Compilation & Run
```bash
mkdir build
cd build
cmake ..
make
./main
```


---


# TODO List

## CPU (ARM7TDMI)
- ✅ Implement project skeleton and basic CPU structure
- ✅ Implement initial ARM/Thumb instructions
- ✅ Complete full ARM instruction set
- ✅ Complete full Thumb instruction set
- ✅ Add pipeline emulation (fetch/decode/execute)
- 🔜 Add CPU exception handling (IRQ, FIQ, SWI, etc.)

## Memory & Bus
- ✅ Implement memory map (WRAM, IRAM, ROM, I/O, VRAM, etc.)
- ✅ Implement memory read/write functions
- ✅ Handle memory alignment and access sizes (8/16/32 bit)
- 🔜 Add DMA channels

## Timers & Interrupts
- 🔜 Implement hardware timers
- 🔜 Implement interrupt controller
- 🔜 Hook up interrupts (VBlank, HBlank, timer, keypad, etc.)

## PPU (Graphics)
- 🔜 Implement background rendering (modes 0–5)
- 🔜 Implement sprite rendering (OBJ)
- 🔜 Implement palette management
- 🔜 Implement windowing and blending effects
- 🔜 Add timing and VBlank/HBlank interrupts

## APU (Audio)
- 🔜 Implement square wave channels
- 🔜 Implement wave channel
- 🔜 Implement noise channel
- 🔜 Implement DMA sound (channels A & B)
- 🔜 Mix and output audio

## Input
- 🔜 Implement button input handling
- 🔜 Add key interrupts


