#include "Chip8.hpp"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <random>

#define FONTSET_SIZE 80
#define START_ADDRESS 0x200
#define FONTSET_START_ADDRESS 0x50
#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {

    // Intialize PC
    pc = START_ADDRESS;

    // Load font into memory
    for(unsigned int i = 0; i < FONTSET_SIZE; i++){
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // Initialize RNG
	randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}


void Chip8::LoadROM(char const* filename){
    // Open file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if(file.is_open()){
        std::streampos size = file.tellg(); // Get file size
        char* buffer = new char[size]; // Allocate a buffer

        file.seekg(0, std::ios::beg); // Go to the beginning of the file
        file.read(buffer, size); // fill the buffer
        file.close(); // close file

        // Load ROM content in memory
        for(long i = 0; i < size; i++){
            memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer; // Free the buffer

    }
}

void Chip8::Cycle() {
    //Fetch
    opcode = (memory[pc] << 8u) | memory[pc + 1];

    // Move PC for the next instruction
    pc += 2;

    //Decode and execute
    DecodeAndExecute(opcode);

    // Decrement the delay timer
    if(delayTimer > 0){
        delayTimer--;
    }

    // Decrement the sound timer
    if(soundTimer > 0){
        soundTimer--;
    }
}

void Chip8::DecodeAndExecute(uint16_t opcode){
    switch ((opcode & 0xF000u))
    {
    case 0x0000u:
        switch (opcode & 0x000Fu)
        {
        case 0x0000u:
            Chip8::OP_00E0();
            break;

        case 0x000Eu:
            Chip8::OP_00EE();
            break;

        default:
            Chip8::OP_NULL();
            std::cout << "Unknown opcode [0x0000]: " << ((opcode & 0xF000) >> 12u) << std::endl;
            break;
        }
        break;
    case 0x1000u:
        Chip8::OP_1nnn();
        break;
    case 0x2000u:
        Chip8::OP_2nnn();
        break;
    case 0x3000u:
        Chip8::OP_3xkk();
        break;
    case 0x4000u:
        Chip8::OP_4xkk();
        break;
    case 0x5000u:
        Chip8::OP_5xy0();
        break;
    case 0x6000u:
        Chip8::OP_6xkk();
        break;
    case 0x7000u:
        Chip8::OP_7xkk();
        break;
    case 0x8000u:
        switch (opcode & 0x000Fu)
        {
        case 0x0000u:
            Chip8::OP_8xy0();
            break;

        case 0x0001u:
            Chip8::OP_8xy1();
            break;

        case 0x0002u:
            Chip8::OP_8xy2();
            break;

        case 0x0003u:
            Chip8::OP_8xy3();
            break;

        case 0x0004u:
            Chip8::OP_8xy4();
            break;

        case 0x0005u:
            Chip8::OP_8xy0();
            break;

        case 0x0006u:
            Chip8::OP_8xy6();
            break;

        case 0x0007u:
            Chip8::OP_8xy7();
            break;

        case 0x000Eu:
            Chip8::OP_8xyE();
            break;

        default:
            Chip8::OP_NULL();
            std::cout << "Unknown opcode [0x0000]: " << opcode << std::endl;
            break;
        }
        break;
    case 0x9000u:
        Chip8::OP_9xy0();
        break;
    case 0xA000u:
        Chip8::OP_Annn();
        break;
    case 0xB000u:
        Chip8::OP_Bnnn();
        break;
    case 0xC000u:
        Chip8::OP_Cxkk();
        break;
    case 0xD000u:
        Chip8::OP_Dxyn();
        break;
    case 0xE000u:
        switch (opcode & 0x000Fu)
        {
        case 0x000Eu:
            Chip8::OP_Ex9E();
            break;

        case 0x0001u:
            Chip8::OP_ExA1();
            break;

        default:
            Chip8::OP_NULL();
            std::cout << "Unknown opcode [0x0000]: " << opcode << std::endl;
            break;
        }
        break;
    case 0xF000u:
        switch (opcode & 0x00FFu)
        {
        case 0x0007u:
            Chip8::OP_Fx07();
            break;

        case 0x000Au:
            Chip8::OP_Fx0A();
            break;

        case 0x0015u:
            Chip8::OP_Fx15();
            break;

        case 0x0018u:
            Chip8::OP_Fx18();
            break;

        case 0x001Eu:
            Chip8::OP_Fx1E();
            break;

        case 0x0029u:
            Chip8::OP_Fx29();
            break;

        case 0x0033u:
            Chip8::OP_Fx33();
            break;

        case 0x0055u:
            Chip8::OP_Fx55();
            break;

        case 0x0065u:
            Chip8::OP_Fx65();
            break;

        default:
            Chip8::OP_NULL();
            std::cout << "Unknown opcode [0x0000]: " << opcode << std::endl;
            break;
        }
        break;
    default:
        Chip8::OP_NULL();
        std::cout << "Unknown opcode [0x0000]: " << ((opcode & 0xF000u)) << std::endl;
        break;
    }
}

// Do nothing
void Chip8::OP_NULL(){

}

// Clear the display
void Chip8::OP_00E0(){
    memset(video, 0, sizeof(video));
}

// Return from a subroutine
void Chip8::OP_00EE(){
    sp--;
    pc = stack[sp];
}

void Chip8::OP_1nnn(){
    uint16_t address = opcode & 0x0FFFu;

    pc = address;

}

// Call subroutine at nnn
void Chip8::OP_2nnn(){
    uint16_t address = opcode & 0x0FFFu;

    stack[sp] = pc;
    sp++;
    pc = address;
}

// Skip next instruction if Vx = kk
void Chip8::OP_3xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if(registers[Vx] == byte){
        pc += 2;
    }

}

// Skip next instruction if Vx != kk
void Chip8::OP_4xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if(registers[Vx] != byte){
        pc += 2;
    }
}

// Skip next instruction if Vx = Vy
void Chip8::OP_5xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] == registers[Vy]){
        pc += 2;
    }
}

// Set Vx = kk
void Chip8::OP_6xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = byte;
}

// Set Vx = Vx + kk
void Chip8::OP_7xkk(){
   uint8_t Vx = (opcode & 0x0F00u) >> 8u;
   uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = registers[Vx] + byte;
}

// Set Vx = Vy
void Chip8::OP_8xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

// Set Vx = Vx OR Vy
void Chip8::OP_8xy1(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vx] | registers[Vy];
}

// Set Vx = Vx AND Vy
void Chip8::OP_8xy2(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vx] & registers[Vy];
}

// Set Vx = Vx XOR Vy
void Chip8::OP_8xy3(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vx] ^ registers[Vy];
}

// Set Vx = Vx + Vy, Set VF = CARRY
void Chip8::OP_8xy4(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sum = registers[Vx] + registers[Vy];

    if(sum > 255U){
        registers[0xF] = 1;
    }
    else {
        registers[0xF] = 0;
    }

    registers[Vx] = sum & 0xFF0u;
}

// Set Vx = Vx - Vy, Set VF = NOT BORROW
void Chip8::OP_8xy5(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] > registers[Vy]){
        registers[0xF] = 1;
    }
    else {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vx] - registers[Vy];
}

// Set Vx = Vx SHR 1
void Chip8::OP_8xy6(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x1u);

    registers[Vx] >>= 1;
}

// Set Vx = Vy - Vx, Set VF = NOT BORROW
void Chip8::OP_8xy7(){
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    if(registers[Vy] > registers[Vx]) {
        registers[0xF] = 1;
    }
    else {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];

}

// Set Vx = Vx SHL 1
void Chip8::OP_8xyE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    registers[Vx] <<= 1; // multiplied by 2

}

// Skip next instruction if Vx != Vy 
void Chip8::OP_9xy0(){
    uint8_t Vx  = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] != registers[Vy]){
        pc += 2;
    }
}

// Set I = nnn
void Chip8::OP_Annn(){
    uint16_t addr = (opcode & 0x0FFFu);
    I = addr;
}

// Jump to location nnn + V0
void Chip8::OP_Bnnn(){
    uint16_t addr = (opcode & 0x0FFFu);

    pc = addr + registers[0];
}

// Set Vx = random byte AND kk
void Chip8::OP_Cxkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = (opcode & 0x00FFu);
    registers[Vx] = randByte(randGen) & kk;
}

// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
void Chip8::OP_Dxyn(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = (opcode & 0x000Fu);

    //Wrap is going beyond the screen boundaries
    uint8_t PosX = registers[Vx] % VIDEO_WIDTH;
    uint8_t PosY = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for(unsigned int row = 0; row < height; row++){
        uint8_t spriteByte = memory[I + row];

        for(unsigned int column = 0; column < 8; column++){
            uint8_t spritePixel = spriteByte & (0x80u >> column);
            uint32_t* screenPixel = &video[(PosY + row) * VIDEO_WIDTH + (PosX + column)];

            // Sprite pixel is on
            if(spritePixel){
                // Screen pixel also on - collision
                if(*screenPixel == 0xFFFFFFFF){
                    registers[0xF] = 1;
                }

                // Effectively XOR with the sprite pixel
                *screenPixel = *screenPixel ^ 0xFFFFFFFF;
            }

        }
    }

}

// Skip next instruction if key with the value of Vx is pressed
void Chip8::OP_Ex9E(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t key = registers[Vx];

    if(keypad[key]){
        pc += 2;
    }
}

// Skip next instruction if key with the value of Vx is not pressed
void Chip8::OP_ExA1(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t key = registers[Vx];

    if(!keypad[key]){
        pc += 2;
    }
}

// Set Vx = delay timer value
void Chip8::OP_Fx07(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delayTimer;
}

// Wait for a key press, store the value of the key in Vx
void Chip8::OP_Fx0A(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if(keypad[0]){
        registers[Vx] = 0;
    }
    else if(keypad[1]) {
        registers[Vx] = 1;
    }
    else if(keypad[2]) {
        registers[Vx] = 2;
    }
    else if(keypad[3]) {
        registers[Vx] = 3;
    }
    else if(keypad[4]) {
        registers[Vx] = 4;
    }
    else if(keypad[5]) {
        registers[Vx] = 5;
    }
    else if(keypad[6]) {
        registers[Vx] = 6;
    }
    else if(keypad[7]) {
        registers[Vx] = 7;
    }
    else if(keypad[8]) {
        registers[Vx] = 8;
    }
    else if(keypad[9]) {
        registers[Vx] = 9;
    }
    else if(keypad[10]) {
        registers[Vx] = 10;
    }
    else if(keypad[11]) {
        registers[Vx] = 11;
    }
    else if(keypad[12]) {
        registers[Vx] = 12;
    }
    else if(keypad[13]) {
        registers[Vx] = 13;
    }
    else if(keypad[14]) {
        registers[Vx] = 14;
    }
    else if(keypad[15]) {
        registers[Vx] = 15;
    }
    else {
        pc -= 2;
    }

}

// Set delay timer = Vx
void Chip8::OP_Fx15(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    delayTimer = registers[Vx];
}

// Set sound timer = Vx
void Chip8::OP_Fx18(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    soundTimer = registers[Vx];
}

// Set I = I + Vx
void Chip8::OP_Fx1E(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    I = I + registers[Vx];
}

// Set I = location of sprite for digit Vx
void Chip8::OP_Fx29(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];

    I = FONTSET_START_ADDRESS + (5 * digit);
}

// Store BCD representation of Vx in memory locations I, I+1, and I+2
void Chip8::OP_Fx33(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];
    for(int i = 2; i > -1; i--) {
        memory[I + i] = value % 10;
        value /= 10;
    }
}

// Store registers V0 through Vx in memory starting at location I
void Chip8::OP_Fx55(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for(uint8_t i = 0; i <= Vx; i++) {
        memory[I + i] = registers[i];
    }
}

// Read registers V0 through Vx from memory starting at location I
void Chip8::OP_Fx65(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for(uint8_t i = 0; i <= Vx; i++) {
        registers[i] = memory[I + i];
    }
}
