cmake_minimum_required(VERSION 3.10)
project(Emulator C)

find_package(SDL2 REQUIRED)
find_package(SDL2_TTF REQUIRED SDL2_ttf)
set(SDL2_TTF_LIBRARIES SDL2_ttf)

add_executable(main src/main.c src/emulator.c src/cpu.c src/cartridge.c src/bus.c src/instructions.c src/bios.c src/alu.c)


target_include_directories(main PRIVATE ${SDL2_INCLUDE_DIRS} include)
target_link_libraries(main PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})



