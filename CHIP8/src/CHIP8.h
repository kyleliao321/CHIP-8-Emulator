#pragma once

#define CHIP8_REGISTER_SIZE 16
#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_STACK_SIZE 16
#define CHIP8_KEY_SIZE 16
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32
#define CHIP8_MICROSECOND_PER_CYCLE 1300

#include "pch.h"

class CHIP8;
class Window;
class EventHandler;

typedef void (CHIP8::* op_fun)();

class CHIP8
{
    friend class Window;
    friend class EventHandler;
public:
    CHIP8();
    ~CHIP8();

    // Initialize the emulated hardware.
    void initialize();

    // Load ROM into memory.
    void Load(const std::string& filepath);

    // Emulate the operations per CPU cycle.
    void EmulateCycle();
private:
    // Fetch operation from memory and store into opcode.
    void _fetch();
    // Decode opcode and store required value into operation varaiables.
    void _decode();
    // Execute operation based on operation variables.
    void _execute();
    // Update timing based on operation variables.
    void _timing();

private:
    /* Emulated functions for CPU operations.
    * Since most of the operations' name in CHIP-8 start with numbers,
    * to prevent illegle fuction name, the naming convention here will be <TYPE>_<OPCODE>.
    */
    void CALL_0NNN();   void DISPLAY_00E0();    void FLOW_00EE();
    void COND_3XNN();   void COND_4XNN();    void COND_5XY0();
    void CONST_6XNN();   void CONST_7XNN();    void ASSIGN_8XY0();
    void BITOP_8XY1();   void BITOP_8XY2();    void BITOP_8XY3();
    void MATH_8XY4();   void MATH_8XY5();    void BITOP_8XY6();
    void MATH_8XY7();   void BITOP_8XYE();    void COND_9XY0();
    void MEM_ANNN();   void FLOW_BNNN();    void RAND_CXNN();
    void DISP_DXYN();   void KEYOP_EX9E();    void KEYOP_EXA1();
    void TIMER_FX07();   void KEYOP_FX0A();    void TIMER_FX15();
    void SOUND_FX18();   void MEM_FX1E();    void MEM_FX29();
    void BCD_FX33();   void MEM_FX55();    void MEM_FX65();
    void FLOW_1NNN();   void FLOW_2NNN();

private:
    /* Operation varaiables
    *   operation   : function pointer for current operation.
    *   fetched     : current cycle fetched opcode.
    */
    op_fun operation;
    uint16_t fetched;

private:
    /* Memory
    * CHI-8 had 4096 memory addresses. Each of the addresses are 1 bytes long.
    * The first 512 bytes spaces are preserved for machine's usage.
    * The uppermost 256 bytes are for display refresh, and 96 bytes before that are call stack.
    * The user program should start at address 0x200.
    */
    uint8_t memory[CHIP8_MEMORY_SIZE];

    /* Registers
    * CHIP-8 has 16 1 bytes registers named V0 to VF.
    * VF is a flag register for special purpose.
    * There are also two address registers which are 2 bytes long :
    *       I : Store memory address for CPU operations.
    *       PC: Program Counter register. Used for storing current reading memory address.
    */
    uint8_t V[CHIP8_REGISTER_SIZE];
    uint16_t I, PC;

    /* Stack
    * CHIP-8's stack is only used for storing return address when branching.
    * It has 16 level of nesting and one stack pointer used to record the current nesting level.
    */
    uint16_t stack[CHIP8_STACK_SIZE];
    uint16_t sp;

    /* Timer
    * CHIP-8 has two timers which will start counting in 60 Hz when the values are above 0.
    * Delay Timer: Using for game event.
    * Sound TImer: Using for sound event.
    */
    uint8_t delay_timer;
    uint8_t sound_timer;

    /* Input
    * CHIP-8 comes with hex keyboard.
    * The key ranges from 0 to F.
    */
    uint8_t key[CHIP8_KEY_SIZE];

    /* Graphic
    * CHIP-8 handles graphic in a 64*32 screen with totally 2048 pixels.
    */
    uint8_t screen[CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT];

    uint8_t draw_flag;
};
