#pragma once

#include "pch.h"
#include "CHIP8.h"

class EventHandler
{
public:
    EventHandler();
    ~EventHandler();

    void Connect(CHIP8* chip8);

    void HandleEvent();
private:
    static uint8_t m_keymap[16];

    CHIP8 *m_chip8;
};