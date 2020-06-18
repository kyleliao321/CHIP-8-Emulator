#pragma once

#include "pch.h"
#include "EventHandler.h"

//  CHIP-8's keypad   -- mapping --> Computer Keyboard
// ����������������������������������                 ����������������������������������
// �� 1 �� 2 �� 3 �� C ��                 �� 1 �� 2 �� 3 �� 4 ��
// ����������������������������������                 ����������������������������������
// �� 4 �� 5 �� 6 �� D ��                 �� Q �� W �� E �� R ��    
// ����������������������������������                 ����������������������������������
// �� 7 �� 8 �� 9 �� E ��                 �� A �� S �� D �� F ��
// ����������������������������������                 ����������������������������������
// �� A �� 0 �� B �� F ��                 �� Z �� X �� C �� V ��
// ����������������������������������                 ����������������������������������

uint8_t EventHandler::m_keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

EventHandler::EventHandler()
{
    // Nothing to setup.
};

EventHandler::~EventHandler()
{
    // Nothing to deconstruct.
};

void EventHandler::Connect(CHIP8* chip8)
{
    m_chip8 = chip8;
};

void EventHandler::HandleEvent()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) exit(0);

        // When given key is hit down, it should record 1 in CHPI8's key set.
        if (e.type == SDL_KEYDOWN) {
            // Allow user to exit the program by pressing ESC.
            if (e.key.keysym.sym == SDLK_ESCAPE)
                exit(0);

            for (int i = 0; i < CHIP8_KEY_SIZE; ++i) {
                if (e.key.keysym.sym == m_keymap[i]) {
                    m_chip8->key[i] = 1;
                }
            }
        }

        // When given key is release, it should set back to 0 in CHIP8's key set.
        if (e.type == SDL_KEYUP) {
            for (int i = 0; i < CHIP8_KEY_SIZE; ++i) {
                if (e.key.keysym.sym == m_keymap[i]) {
                    m_chip8->key[i] = 0;
                }
            }
        }
    }
}