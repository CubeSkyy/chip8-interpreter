//
// Created by miguelcoelho on 11-07-2022.
//

#include "Chip8.h"
#include "fstream"
#include <chrono>
#include <random>
//#include "include/spdlog/spdlog.h"

using namespace std;

Chip8::Chip8() {
    pc = START_ADDRESS;
    LoadFontset();

    default_random_engine randGen = new default_random_engine (chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<uint8_t> randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}

void Chip8::LoadROM(char const *filename) {
    ifstream rom_file;
    rom_file.open(filename, ios::binary | ios::ate);

    if (rom_file.is_open()) {
        streampos size = rom_file.tellg();
        char *buffer = new char[size];

        rom_file.seekg(0, ios::beg);
        rom_file.read(buffer, size);
        rom_file.close();

        for (long i = 0; i < size; i++) {
            memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
        //spdlog::info("Rom Loaded.");
    }

}

void Chip8::LoadFontset() {
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
    //spdlog::info("FontSet loaded.");
}

uint8_t Chip8::getRandomByte() {
    return randByte()
}


int main() {
    //spdlog::set_level(spdlog::level::debug);

    Chip8 chip8;
    chip8.LoadROM("roms/Trip8 Demo (2008) [Revival Studios].ch8");


}