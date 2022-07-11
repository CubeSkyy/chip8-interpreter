cmake_minimum_required(VERSION 3.22)
project(chip8_interpreter)

set(CMAKE_CXX_STANDARD 14)

include_directories(. include)

add_executable(chip8_interpreter
        chip8.cpp)
