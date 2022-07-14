cmake_minimum_required(VERSION 3.22)
project(chip8_interpreter)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH "C:\\Program\ Files\\CMake\\share\\cmake-3.23\\Modules")

set(SDL2_PATH "C:\\Users\\CubeSky\\Projects\\SDL2-2.0.22")
find_package(SDL2 REQUIRED)
include_directories( ${SDL2_INCLUDE_DIR} )

add_executable(chip8_interpreter
        chip8.cpp Platform.cpp Platform.h)

target_link_libraries(chip8_interpreter ${SDL2_LIBRARY} )