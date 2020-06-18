#pragma once

#include "pch.h"
#include "CHIP8.h"

#define DECODE_X(opcode)    static_cast<uint8_t>((opcode & 0x0F00) >> 8)
#define DECODE_Y(opcode)    static_cast<uint8_t>((opcode & 0x00F0) >> 4)
#define DECODE_N(opcode)    static_cast<uint8_t>(opcode & 0x000F)
#define DECODE_NN(opcode)   static_cast<uint8_t>(opcode & 0x00FF)
#define DECODE_NNN(opcode)  static_cast<uint16_t>(opcode & 0x0FFF)

#define UNDEFINED_OPCODE(x) \
    std::cout << "OPCODE Error: " << std::hex << x << std::dec << " not exist." << std::endl; \
    exit(3)

CHIP8::CHIP8()
{
    initialize();
};

CHIP8::~CHIP8()
{
    // nothing to free in memory.
};

void CHIP8::initialize()
{
    // Reset all the memory address, registers, operation variables to original values.
    unsigned int i;

    for (i = 0; i < CHIP8_REGISTER_SIZE; i++) V[i] = 0;
    for (i = 0; i < CHIP8_MEMORY_SIZE; i++) memory[i] = 0;
    for (i = 0; i < CHIP8_STACK_SIZE; i++) stack[i] = 0;
    for (i = 0; i < CHIP8_KEY_SIZE; i++) key[i] = 0;
    for (i = 0; i < CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT; i++) screen[i] = 0;

    delay_timer = 0;
    sound_timer = 0;

    sp = 0;
    fetched = 0x0;
    I = 0x0;
    PC = 0x200; // Client program starts from 0x200 in memory address;

    // Load font sprites data into memory.
    // Each font sprites are 4*5 pixels.
    //
    // Example : 9's sprite = 1111 0000                     ####
    //                        1001 0000                     #  #
    //                        1111 0000  === On Screen ==>  ####
    //                        0001 0000                        #
    //                        1111 0000                     ####

    uint8_t font_sprites[5 * 16] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80  //F
    };

    for (i = 0; i < 5 * 16; i++) memory[i] = font_sprites[i];
};

void CHIP8::Load(const std::string& filepath)
{
    // Reset hardware everytime loading a new game.
    initialize();

    unsigned int i;
    std::ifstream file;
    file.open(filepath, std::ifstream::binary);

    if (file.is_open())
    {
        std::filebuf* pbuf = file.rdbuf();

        size_t file_size = pbuf->pubseekoff(0, file.end, file.in);
        pbuf->pubseekpos(0, file.in);

        if (file_size > 4096 - 512)
        {
            printf("File Error: Cannot load the game file with size %i\n", file_size);
            exit(-1);
        }

        char* buffer = new char[file_size];
        pbuf->sgetn(buffer, file_size);

        // Load binary data into emulated memory. Starts from 0x200.
        for (i = 0; i < file_size; i++)
            memory[i + 0x200] = (uint8_t)buffer[i];

        file.close();
        delete[] buffer;
    }
    else
    {
        printf("File Error: Cannot open the game at %s\n", filepath);
        exit(-1);
    }
};

void CHIP8::EmulateCycle()
{
    /* Emulated all the process CPU take within one Cycle
    *  Fetching opcode from memory and storing into Program Counter.
    *  Decoding opcode which indicated which operation to take in this cycle.
    *  Executing the given operation.
    *  Update two timers.
    */
    _fetch();
    _decode();
    _execute();
    _timing();
};

void CHIP8::_fetch()
{
    // Opcode is 2 bytes long, but each memory address is only 1 bytes long.
    // So CHIP8 has to fetch two memory address at the same time to construct entire opcode.
    // Example : memory[PC]      = 00 00 00 00 00 11 01 11
    //           memory[PC] << 8 = 00 11 01 11 00 00 00 00
    //           memory[PC+1]    = 00 00 00 00 00 10 00 01
    //           BITWISE & OP ----------------------------
    //           fetched         = 00 11 01 11 00 10 00 01 (original opcode)
    fetched = (uint16_t)memory[PC] << 8 | (uint16_t)memory[PC + 1];
};

void CHIP8::_decode()
{
    // Decode fetched opcode by mask.

    using c = CHIP8;

    // First using 0xF000 to extract most significant byte of opcode.
    switch (fetched & 0xF000)
    {
    case 0x0000:
        switch (fetched & 0x000F)
        {
            // 00E0
        case 0x0000:
            operation = &c::DISPLAY_00E0;
            break;
            // 00EE
        case 0x000E:
            operation = &c::FLOW_00EE;
            break;
        default:
            UNDEFINED_OPCODE(fetched);
            break;
        }
        break;

        // 1NNN
    case 0x1000:
        operation = &c::FLOW_1NNN;
        break;

        // 2NNN
    case 0x2000:
        operation = &c::FLOW_2NNN;
        break;

        // 3XNN
    case 0x3000:
        operation = &c::COND_3XNN;
        break;

        // 4XNN
    case 0x4000:
        operation = &c::COND_4XNN;
        break;

        // 5XY0
    case 0x5000:
        operation = &c::COND_5XY0;
        break;

        // 6XNN
    case 0x6000:
        operation = &c::CONST_6XNN;
        break;

        // 7XNN
    case 0x7000:
        operation = &c::CONST_7XNN;
        break;

    case 0x8000:
        switch (fetched & 0x000F)
        {
            // 8XY0
        case 0x0000:
            operation = &c::ASSIGN_8XY0;
            break;
            // 8XY1
        case 0x0001:
            operation = &c::BITOP_8XY1;
            break;
            // 8XY2
        case 0x0002:
            operation = &c::BITOP_8XY2;
            break;
            // 8XY3
        case 0x0003:
            operation = &c::BITOP_8XY3;
            break;
            // 8XY4
        case 0x0004:
            operation = &c::MATH_8XY4;
            break;
            // 8XY5
        case 0x0005:
            operation = &c::MATH_8XY5;
            break;
            // 8XY6
        case 0x0006:
            operation = &c::BITOP_8XY6;
            break;
            // 8XY7
        case 0x0007:
            operation = &c::MATH_8XY7;
            break;
            // 8XYE
        case 0x000E:
            operation = &c::BITOP_8XYE;
            break;
        default:
            UNDEFINED_OPCODE(fetched);
            break;
        }
        break;

        // 9XY0        
    case 0x9000:
        operation = &c::COND_9XY0;
        break;

        // ANNN
    case 0xA000:
        operation = &c::MEM_ANNN;
        break;

        // BNNN
    case 0xB000:
        operation = &c::FLOW_BNNN;
        break;

        // CXNN
    case 0xC000:
        operation = &c::RAND_CXNN;
        break;

        // DXYN
    case 0xD000:
        operation = &c::DISP_DXYN;
        break;

    case 0xE000:
        switch (fetched & 0x00FF)
        {
            // EX9E
        case 0x009E:
            operation = &c::KEYOP_EX9E;
            break;
            // EXA1
        case 0x00A1:
            operation = &c::KEYOP_EXA1;
            break;
        default:
            UNDEFINED_OPCODE(fetched);
            break;
        }
        break;

    case 0xF000:
        switch (fetched & 0x00FF)
        {
            // FX07
        case 0x0007:
            operation = &c::TIMER_FX07;
            break;
            // FX0A
        case 0x000A:
            operation = &c::KEYOP_FX0A;
            break;
            // FX15
        case 0x0015:
            operation = &c::TIMER_FX15;
            break;
            // FX18
        case 0x0018:
            operation = &c::SOUND_FX18;
            break;
            // FX1E
        case 0x001E:
            operation = &c::MEM_FX1E;
            break;
            // FX29
        case 0x0029:
            operation = &c::MEM_FX29;
            break;
            // FX33
        case 0x0033:
            operation = &c::BCD_FX33;
            break;
            // FX55
        case 0x0055:
            operation = &c::MEM_FX55;
            break;
            // FX65
        case 0x0065:
            operation = &c::MEM_FX65;
            break;
        default:
            UNDEFINED_OPCODE(fetched);
            break;
        }
        break;

    default:
        UNDEFINED_OPCODE(fetched);
    }
};

void CHIP8::_execute()
{
    (this->*operation)();
};

void CHIP8::_timing()
{
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0)
        --sound_timer;
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// CPU Operations ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void CHIP8::CALL_0NNN()
{
    // Call RCA 1802 Program at address of NNN.
    // Skip this function, since most of client program won't use it.
};

void CHIP8::DISPLAY_00E0()
{
    // Clear the scree.
    unsigned int i;
    for (i = 0; i < CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT; i++) screen[i] = 0;
    draw_flag = 1;
    PC += 2;
};

void CHIP8::FLOW_00EE()
{
    // Return from the subroutine.
    PC = stack[--sp];
    // Since the previous PC is the one which enter the subroutine,
    // it is necessary to forward the PC prevent infinate loop.
    PC += 2;
};

void CHIP8::FLOW_1NNN()
{
    // Jump to the address of NNN.
    uint16_t NNN = DECODE_NNN(fetched);
    PC = NNN;
};

void CHIP8::FLOW_2NNN()
{
    // Call the subroutine at NNN.
    uint16_t NNN = DECODE_NNN(fetched);
    stack[sp++] = PC;
    PC = NNN;
};

void CHIP8::COND_3XNN()
{
    // Skip the next 2 bytes of memory if the register VX is equal to NN.
    uint8_t X = DECODE_X(fetched);
    uint8_t NN = DECODE_NN(fetched);
    if (V[X] == NN) PC += 4;
    else PC += 2;
};

void CHIP8::COND_4XNN()
{
    // Skip the next 2 bytes of memory if the register VX is not equal to NN.
    uint8_t X = DECODE_X(fetched);
    uint8_t NN = DECODE_NN(fetched);
    if (V[X] != NN) PC += 4;
    else PC += 2;
};

void CHIP8::COND_5XY0()
{
    // Skip the next 2 bytes of memory if the register VX is equal to VY.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    if (V[X] == V[Y]) PC += 4;
    else PC += 2;
};

void CHIP8::CONST_6XNN()
{
    // Set Vx to NN.
    uint8_t X = DECODE_X(fetched);
    uint8_t NN = DECODE_NN(fetched);
    V[X] = NN;
    PC += 2;
};

void CHIP8::CONST_7XNN()
{
    // Add NN to VX.
    uint8_t X = DECODE_X(fetched);
    uint8_t NN = DECODE_NN(fetched);
    V[X] += NN;
    PC += 2;
};

void CHIP8::ASSIGN_8XY0()
{
    // Assign VY to VX.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    V[X] = V[Y];
    PC += 2;
};

void CHIP8::BITOP_8XY1()
{
    // Assign VX|VY to VX.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    V[X] |= V[Y];
    PC += 2;
};

void CHIP8::BITOP_8XY2()
{
    // Assign VX&VY to VX.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    V[X] &= V[Y];
    PC += 2;
};

void CHIP8::BITOP_8XY3()
{
    // Assign VX^VY to VX. (XOR operation)
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    V[X] ^= V[Y];
    PC += 2;
};

void CHIP8::MATH_8XY4()
{
    // Adds VY to VX. And set VF to 1 if there's carry bit.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    uint8_t tmp = V[X] + V[Y];
    if (static_cast<uint16_t>(V[X]) + static_cast<uint16_t>(V[Y]) > 0xFF)
        V[0xF] = 1;
    else
        V[0xF] = 0;
    V[X] = tmp;
    PC += 2;
};

void CHIP8::MATH_8XY5()
{
    // Substract VY to VX. And set VF to 0 if there's a borrow bit.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    uint8_t tmp = V[X] - V[Y];
    if (V[Y] > V[X])
        V[0xF] = 0;
    else
        V[0xF] = 1;
    V[X] = tmp;
    PC += 2;
};

void CHIP8::BITOP_8XY6()
{
    // Store the least significant bit of VX to VF, then shifts VX to right by 1.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    V[0xF] = V[X] & 0x1; // Using mask to obtain the first bit of VX.
    V[X] >>= 1;
    PC += 2;
};

void CHIP8::MATH_8XY7()
{
    // Set VX equal to VY - VX. And set VF to 0 if there's a borrow bit.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    uint8_t tmp = V[Y] - V[X];
    if (static_cast<uint16_t>(V[X]) > static_cast<uint16_t>(V[Y]))
        V[0xF] = 0;
    else
        V[0xF] = 1;
    V[X] = tmp;
    PC += 2;
};

void CHIP8::BITOP_8XYE()
{
    // Store the most significant bit of VX to VF, then shifts VX to the left by 1.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    V[0xF] = V[X] >> 7;
    V[X] <<= 1;
    PC += 2;
};

void CHIP8::COND_9XY0() {
    // Skip the next 2 bytes if VX != VY.
    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    if (V[X] != V[Y])
        PC += 4;
    else
        PC += 2;
};

void CHIP8::MEM_ANNN()
{
    // Assign NNN to I.
    uint16_t NNN = DECODE_NNN(fetched);
    I = NNN;
    PC += 2;
};

void CHIP8::FLOW_BNNN()
{
    // Jump to (NNN + V0).
    uint16_t NNN = DECODE_NNN(fetched);
    PC = NNN + V[0x0];
};

void CHIP8::RAND_CXNN()
{
    // Assign result of bitwise AND operation between random number and NN to VX.
    uint8_t X = DECODE_X(fetched);
    uint8_t NN = DECODE_NN(fetched);
    V[X] = NN & (rand() % (0xFF + 1)); // make sure random number is within range of 0-255.
    PC += 2;
};

void CHIP8::DISP_DXYN()
{
    // Draw sprite(8p, Np) at (VX, VY) using bitwise XOR operation. 
    // VF is set to 1 if any original pixels are flipped.
    // There are N rows for a sprite, each row(8 bits) is stored in memory[I + N].
    // Example : 0xD003 - Draw (8 * 3) sprite at location(0, 0).
    //           memory [I]     = 00 11 11 00
    //           memory [I + 1] = 11 11 11 11
    //           memory [I + 2] = 00 11 11 00
    //           ----------------------------
    //           original area  = 00 11 11 00
    //                            00 11 11 00
    //                            00 11 11 00
    //           ----------------------------
    //           result         = 00 00 00 00   Some bits in original area are flipped.
    //                            11 00 00 11   Indicating there's collesion, set VF to 1.
    //                            00 00 00 00
    //           ----------------------------

    uint8_t X = DECODE_X(fetched);
    uint8_t Y = DECODE_Y(fetched);
    uint8_t N = DECODE_N(fetched); // height of the sprite.
    uint8_t pixels;

    unsigned int pos;

    V[0xF] = 0;

    for (unsigned int h = 0; h < N; h++)
    {
        pixels = memory[I + h];

        for (unsigned int w = 0; w < 8; w++)
        {
            // Get the position of current pixel on screen.
            pos = V[X] + w + (V[Y] + h) * CHIP8_SCREEN_WIDTH;

            // If both new pixel and old pixel are 1, it indicates collesion detection.
            // Thus, set VF to 1.
            if ((pixels & (0x80 >> w)) && screen[pos])
                V[0xF] = 1;

            // Using 2 bytes mask - 1000 0000 shifts right by w to extract current pixel
            // Example : w = 4
            //           1000 0000 >> 4 = 0000 1000
            //                        XOR 1111 1111 -> original pixels in this row
            //                        -------------
            //                            0000 0000 -> new pixel will be 0x00
            screen[pos] ^= static_cast<uint8_t>(pixels & (0x80 >> w));
        }
    }

    draw_flag = 1;

    PC += 2;
};

void CHIP8::KEYOP_EX9E()
{
    // Skip the next 2 bytes of memeroy if key[VX] is pressed.
    uint8_t X = DECODE_X(fetched);
    if (key[V[X]])
        PC += 4;
    else
        PC += 2;
};

void CHIP8::KEYOP_EXA1()
{
    // Skip the next 2 bytes of memory if key[VX] is not pressed.
    uint8_t X = DECODE_X(fetched);
    if (!key[V[X]])
        PC += 4;
    else
        PC += 2;
};

void CHIP8::TIMER_FX07()
{
    // Assign delay timer's value to VX.
    uint8_t X = DECODE_X(fetched);
    V[X] = delay_timer;
    PC += 2;
};

void CHIP8::KEYOP_FX0A()
{
    // Block IO until next key event is recieved.
    // By not updating the PC value, it is essentially the same as io blocking behaviour.
    uint8_t X = DECODE_X(fetched);

    unsigned int i;

    for (i = 0; i < CHIP8_KEY_SIZE; i++)
    {
        if (key[i])
        {
            V[X] = i;
            PC += 2;
            break;
        }
    }
};

void CHIP8::TIMER_FX15()
{
    // Assign VX to delay timer.
    uint8_t X = DECODE_X(fetched);
    delay_timer = V[X];
    PC += 2;
};

void CHIP8::SOUND_FX18()
{
    // Assign VX to sound timer.
    uint8_t X = DECODE_X(fetched);
    sound_timer = V[X];
    PC += 2;
};

void CHIP8::MEM_FX1E()
{
    // Add VX to I. If the result value is greater than 0xFFF, set VF to 1.
    uint8_t X = DECODE_X(fetched);
    I += V[X];
    if (I > 0xFFF)
        V[0xF] = 1;
    else
        V[0xF] = 0;
    PC += 2;
};

void CHIP8::MEM_FX29()
{
    // Set I to the memory address of font sprite given by VX.
    // The default font sprites are store in first 80 bytes of memory address.
    // Since all sprites are 4x5 pixels and each memeory address is 8 bits long,
    // each sprites will take 5 bytes of memory.
    // Example : 2's font sprite will be located at memory[2 * 5] = memory[10];
    // Thus, the formula will be : font-sprites-X = memory[X * 5].
    uint8_t X = DECODE_X(fetched);
    I = V[X] * 0x5;
    PC += 2;
};

void CHIP8::BCD_FX33()
{
    // Store decimal format of VX into memory.
    // Since the register's storage size is 8 bits, the maximum value it will return will be 127.
    // Extract the hundres digit in memory[I];
    //         the tens digit in memory[I+1];
    //         the ones digit in memory[I+2].
    //uint8_t X = DECODE_X(fetched);
    //memory[I] = X / 100;
    //memory[I + 1] = (X / 10) % 10;
    //memory[I + 2] = X % 10;
    //PC += 2;
    memory[I] = V[(fetched & 0x0F00) >> 8] / 100;
    memory[I + 1] = (V[(fetched & 0x0F00) >> 8] / 10) % 10;
    memory[I + 2] = V[(fetched & 0x0F00) >> 8] % 10;
    PC += 2;
};

void CHIP8::MEM_FX55()
{
    // Store [V0 .. VX] into memory which starts from I.
    uint8_t X = DECODE_X(fetched);
    unsigned int i;
    for (i = 0; i <= X; i++)
        memory[I + i] = V[i];
    PC += 2;
};

void CHIP8::MEM_FX65()
{
    // Fill [V0 .. VX] from memory which starts from I.
    uint8_t X = DECODE_X(fetched);
    unsigned int i;
    for (i = 0; i <= X; i++)
        V[i] = memory[I + i];
    PC += 2;
};
//////////////////////////////////////////////////////////////////////////////////////////////////