cmake_minimum_required(VERSION 3.10)
project(Emulator C)
set(CMAKE_C_STANDARD 23)

find_package(SDL2 REQUIRED)
find_package(SDL2_TTF REQUIRED SDL2_ttf)
set(SDL2_TTF_LIBRARIES SDL2_ttf)

file(GLOB SRC_FILES src/*.c)

add_executable(main ${SRC_FILES})


target_include_directories(main PRIVATE ${SDL2_INCLUDE_DIRS} include)
target_link_libraries(main PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

