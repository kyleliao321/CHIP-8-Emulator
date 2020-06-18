#pragma once

#include "pch.h"
#include "EventHandler.h"

//  CHIP-8's keypad   -- mapping --> Computer Keyboard
// שÝשששששששÞשששששששÞשששששששÞשששששששß                 שÝשששששששÞשששששששÞשששששששÞשששששששß
// שר 1 שר 2 שר 3 שר C שר                 שר 1 שר 2 שר 3 שר 4 שר
// שאשששששששבשששששששבשששששששבשששששששג                 שאשששששששבשששששששבשששששששבשששששששג
// שר 4 שר 5 שר 6 שר D שר                 שר Q שר W שר E שר R שר    
// שאשששששששבשששששששבשששששששבשששששששג                 שאשששששששבשששששששבשששששששבשששששששג
// שר 7 שר 8 שר 9 שר E שר                 שר A שר S שר D שר F שר
// שאשששששששבשששששששבשששששששבשששששששג                 שאשששששששבשששששששבשששששששבשששששששג
// שר A שר 0 שר B שר F שר                 שר Z שר X שר C שר V שר
// שדשששששששהשששששששהשששששששהשששששששו                 שדשששששששהשששששששהשששששששהשששששששו

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