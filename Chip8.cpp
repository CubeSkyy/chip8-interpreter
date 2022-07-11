//
// Created by miguelcoelho on 11-07-2022.
//

#include <iostream>
#include "Chip8.h"
#include "fstream"
#include <chrono>
#include <random>
#include <cstdio>
#include <cstring>
//#include "include/spdlog/spdlog.h"

using namespace std;

Chip8::Chip8() {
    pc = START_ADDRESS;
    LoadFontset();

    //Init RNG
    unsigned seed = chrono::steady_clock::now().time_since_epoch().count();
    randGen.seed(seed);
    randByte = uniform_int_distribution<uint8_t>(0, 255U);

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
    return randByte(randGen);
}

// Instruction Set

// CLS
// CLear the display
void Chip8::OP_00E0() {
    std::memset(display, 0, sizeof(display);
}

// RET
// Return from the subroutine
void Chip8::OP_00EE() {
    --sp;
    pc = stack[sp];
}

// JP addr
// Jump to location nnn
void Chip8::OP_1nnn() {
    uint16_t address = opcode & 0x0FFFu;

    pc = address;
}

// CALL addr
// Call subroutine at nnn
void Chip8::OP_2nnn() {
    uint16_t address = opcode & 0x0FFFu;

    stack[sp] = pc;
    ++sp;
    pc = address;
}

// SE Vx, byte
// Skip next instruction if Vx = kk
void Chip8::OP_3xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = opcode & 0x00FFu;

    if (registers[Vx] == value) {
        pc += 2;
    }
}

// SNE Vx, byte
// Skip next instruction if Vx != kk
void Chip8::OP_4xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = opcode & 0x00FFu;

    if (registers[Vx] != value) {
        pc += 2;
    }
}

// SE Vx, Vy
// Skip next instruction if Vx = Vy
void Chip8::OP_5xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] == registers[Vy]) {
        pc += 2;
    }
}

// LD Vx, byte
// Set Vx = kk
void Chip8::OP_6xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = opcode & 0x00FFu;

    registers[Vx] = value;
}


// ADD Vx, byte
// Set Vx = Vx + kk
void Chip8::OP_7xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = opcode & 0x00FFu;

    registers[Vx] += value;
}


// LD Vx, Vy
// Set Vx = Vy
void Chip8::OP_8xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}


// OR Vx, Vy
// Set Vx = Vx OR Vy
void Chip8::OP_8xy1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];
}


// AND Vx, Vy
// Set Vx = Vx AND Vy
void Chip8::OP_8xy2() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] &= registers[Vy];
}


// XOR Vx, Vy
// Set Vx = Vx XOR Vy
void Chip8::OP_8xy3() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] ^= registers[Vy];
}


// ADD Vx, Vy
// Set Vx = Vx + Vy, set VF = carry
void Chip8::OP_8xy4() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sum = registers[Vx] + registers[Vy];

    if (sum > 255u) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }

    registers[Vx] = sum & 0xFFu;
}


// SUB Vx, Vy
// Set Vx = Vx - Vy, set VF = NOT borrow
void Chip8::OP_8xy5() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sub = registers[Vx] - registers[Vy];

    if (sub > 0) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }

    registers[Vx] = sub & 0xFFu;
}


// SHR Vx
// Set Vx = Vx SHR 1
void Chip8::OP_8xy6() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x1u);
    registers[Vx] >>= 1;
}


// SUBN Vx, Vy
// Set Vx = Vy - Vx, set VF = NOT borrow
void Chip8::OP_8xy7() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sub = registers[Vx] - registers[Vy];

    if (sub <= 0) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }

    registers[Vx] = sub & 0xFFu;
}


// SHL Vx {, Vy}
// Set Vx = Vx SHL 1
void Chip8::OP_8xyE() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
    registers[Vx] <<= 1;
}

int main() {
    //spdlog::set_level(spdlog::level::debug);

    Chip8 chip8;
    chip8.LoadROM("roms/Trip8 Demo (2008) [Revival Studios].ch8");

    for (int i = 0; i < 20; i++) {
        cout << unsigned(chip8.getRandomByte()) << endl;

    }


}