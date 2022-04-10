cmake_minimum_required(VERSION 3.14)
project(Chip8)

set(CMAKE_CXX_STANDARD 11)

find_package(SDL2 REQUIRED)

add_executable(
	Chip8
	src/Chip-8/Chip8.cpp
	src/main.cpp
	src/Display/Display.cpp)

target_compile_options(Chip8 PRIVATE -Wall)

target_link_libraries(Chip8 PRIVATE SDL2::SDL2)
