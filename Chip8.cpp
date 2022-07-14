//
// Created by miguelcoelho on 11-07-2022.
//

#include <iostream>
#include "Chip8.h"
#include "fstream"
#include "Platform.h"
#include <chrono>
#include <random>
#include <cstdio>
#include <cstring>
#include <string>
#include "algorithm"
//#include "include/spdlog/spdlog.h"

using namespace std;

Chip8::Chip8() {
    pc = START_ADDRESS;
    LoadFontset();

    //Init RNG
    unsigned seed = chrono::steady_clock::now().time_since_epoch().count();
    randGen.seed(seed);
    randByte = uniform_int_distribution<int>(0, 255);

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
    std::memset(display, 0, sizeof(display));
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


// SNE Vx, Vy
// Skip next instruction if Vx != Vy
void Chip8::OP_9xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy]) {
        pc += 2;
    }
}


// LD I, addr
// Set I = nnn
void Chip8::OP_Annn() {
    index = opcode & 0x0FFFu;
}


// JP V0, addr
// Jump to location nnn + V0
void Chip8::OP_Bnnn() {
    uint16_t address = (opcode & 0x0FFFu) + registers[0];

    pc = address;
}


// RND Vx, byte
// Set Vx = random byte AND kk
void Chip8::OP_Cxkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = opcode & 0x00FFu;

    registers[Vx] = value & getRandomByte();
}


// DRW Vx, Vy, nibble
// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
void Chip8::OP_Dxyn() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t value = opcode & 0x000Fu;

    // Wrap if going beyond screen boundaries
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int row = 0; row < value; ++row) {
        uint8_t spriteByte = memory[index + row];

        for (unsigned int col = 0; col < 8; ++col) {
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            uint32_t *screenPixel = &display[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            // Sprite pixel is on
            if (spritePixel) {
                // Screen pixel also on - collision
                if (*screenPixel == 0xFFFFFFFF) {
                    registers[0xF] = 1;
                }

                // Effectively XOR with the sprite pixel
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }


}


// SKP Vx
// Skip next instruction if key with the value of Vx is pressed
void Chip8::OP_Ex9E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (keypad[registers[Vx]]) {
        pc += 2;
    }

}


// SKNP Vx
// Skip next instruction if key with the value of Vx is not pressed
void Chip8::OP_ExA1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (!keypad[registers[Vx]]) {
        pc += 2;
    }
}


// LD Vx, DT
// Set Vx = delay timer value
void Chip8::OP_Fx07() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delay_timer;
}


// LD Vx, K
// Wait for a key press, store the value of the key in Vx
void Chip8::OP_Fx0A() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (int i = 0; i < size(keypad); i++) {
        if (keypad[i]) {
            registers[Vx] = i;
            return;
        }
    }
    pc -= 2;
}


// LD DT, Vx
// Set delay timer = Vx
void Chip8::OP_Fx15() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    delay_timer = registers[Vx];
}


// LD ST, Vx
// Set sound timer = Vx
void Chip8::OP_Fx18() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    sound_timer = registers[Vx];
}


// ADD I, Vx
// Set I = I + Vx
void Chip8::OP_Fx1E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index += registers[Vx];
}


// LD F, Vx
// Set I = location of sprite for digit Vx
void Chip8::OP_Fx29() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    index = memory[FONTSET_START_ADDRESS + (registers[Vx] * 5)];

}


// LD B, Vx
// Store BCD representation of Vx in memory locations I, I+1, and I+2
//
//The interpreter takes the decimal value of Vx, and places the hundreds digit in memory
// at location in I, the tens digit at location I+1, and the ones digit at location I+2
void Chip8::OP_Fx33() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t value = registers[Vx];

    memory[index + 2] = value % 10;
    value /= 10;

    memory[index + 1] = value % 10;
    value /= 10;

    memory[index] = value % 10;


}


// LD [I], Vx
// Store registers V0 through Vx in memory starting at location I.
void Chip8::OP_Fx55() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; i++) {
        memory[index + i] = registers[i];
    }

}


// LD Vx, [I]
// Read registers V0 through Vx from memory starting at location I
void Chip8::OP_Fx65() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; i++) {
        registers[i] = memory[index + i];
    }

}



// Master Tables for opcode translation

void Chip8::OP_NULL() {}


void Chip8::opcode_translation(uint16_t opcode) {

    uint8_t first_digit = (opcode & 0xF000u) >> 12u;

    switch (first_digit) {
        case 0x0: {
            uint8_t last_digit = opcode & 0x000Fu;
            switch (last_digit) {
                case 0x0:
                    OP_00E0();
                    break;
                case 0xE:
                    OP_00EE();
                    break;
                default:
                    OP_NULL();
                    break;
            }
        }
        case 0x8: {
            uint8_t last_digit = opcode & 0x000Fu;
            switch (last_digit) {
                case 0x0:
                    OP_8xy0();
                    break;
                case 0x1:
                    OP_8xy1();
                    break;
                case 0x2:
                    OP_8xy2();
                    break;
                case 0x3:
                    OP_8xy3();
                    break;
                case 0x4:
                    OP_8xy4();
                    break;
                case 0x5:
                    OP_8xy5();
                    break;
                case 0x6:
                    OP_8xy6();
                    break;
                case 0x7:
                    OP_8xy7();
                    break;
                case 0xE:
                    OP_8xyE();
                    break;
                default:
                    OP_NULL();
                    break;
            }
            break;
        }
        case 0xE: {
            uint8_t last_digit = opcode & 0x000Fu;
            switch (last_digit) {
                case 0x1:
                    OP_ExA1();
                    break;
                case 0xE: 
                    OP_Ex9E();
                    break;
                default:
                    OP_NULL();
                    break;
            }
            break;
        }
        case 0xF: {
            uint16_t last2digits = (opcode & 0x00FFu);
            switch (last2digits) {
                case 0x07: 
                    OP_Fx07();
                    break;
                case 0x0A: 
                    OP_Fx0A();
                    break;
                case 0x15: 
                    OP_Fx15();
                    break;
                case 0x18: 
                    OP_Fx18();
                    break;
                case 0x1E: 
                    OP_Fx1E();
                    break;
                case 0x29: 
                    OP_Fx29();
                    break;
                case 0x33: 
                    OP_Fx33();
                    break;
                case 0x55: 
                    OP_Fx55();
                    break;
                case 0x65: 
                    OP_Fx65();
                    break;
                default:
                    OP_NULL();
                    break;
            }
            break;
        }
        case 0x1: 
            OP_1nnn();
            break;
        case 0x2: 
            OP_2nnn();
            break;
        case 0x3: 
            OP_3xkk();
            break;
        case 0x4: 
            OP_4xkk();
            break;
        case 0x5: 
            OP_5xy0();
            break;
        case 0x6: 
            OP_6xkk();
            break;
        case 0x7: 
            OP_7xkk();
            break;
        case 0x9:
            OP_9xy0();
            break;
        case 0xA: 
            OP_Annn();
            break;
        case 0xB: 
            OP_Bnnn();
            break;
        case 0xC: 
            OP_Cxkk();
            break;
        case 0xD: 
            OP_Dxyn();
            break;
        default:
            OP_NULL();
            break;
    }

}


void Chip8::Cycle() {

    // Fetch the next instruction
    opcode = (memory[pc] << 8u) | memory[pc + 1];

    pc += 2;

    //Translate opcode into function
    //uint8_t first_digit = (opcode & 0xF000u) >> 12u;
    //((*this).*(table[first_digit]))();
    opcode_translation(opcode);

    // Decrement timers
    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        --sound_timer;
    }

}


int main(int argc, char *argv[]) {
    //spdlog::set_level(spdlog::level::debug);
    Chip8 chip8;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <ROM> <Scale> <Delay>\n";
        std::exit(EXIT_FAILURE);
    }

    char const *rom_filename = argv[1];
    unsigned int scale = chip8.DEFAULT_SCALE;
    unsigned int delay = chip8.DEFAULT_DELAY;

    if (argc > 2) {
        scale = stoi(argv[2]);
    }
    if (argc > 3) {
        delay = stoi(argv[3]);
    }

    Platform platform("CHIP-8 Emulator", chip8.VIDEO_WIDTH * scale, chip8.VIDEO_HEIGHT * scale, chip8.VIDEO_WIDTH,
                      chip8.VIDEO_HEIGHT);

    chip8.LoadROM(rom_filename);

    int videoPitch = sizeof(chip8.display[0]) * chip8.VIDEO_WIDTH;
    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit) {
        quit = platform.ProcessInput(chip8.keypad);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(
                currentTime - lastCycleTime).count();

        if (dt > delay) {
            lastCycleTime = currentTime;

            chip8.Cycle();

            platform.Update(chip8.display, videoPitch);
        }
    }

    return 0;

}

