#pragma once

#include "pch.h"
#include "CHIP8.h"

class Window
{
public:
    Window(const std::string& name, unsigned int w, unsigned int h);
    ~Window();

    void Connect(CHIP8* chip8);

    void Draw();
private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_texture;

    CHIP8* m_chip8;
};