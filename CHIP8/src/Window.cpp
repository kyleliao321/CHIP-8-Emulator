#pragma once

#include "pch.h"
#include "Window.h"

Window::Window(const std::string& name, unsigned int w, unsigned int h)
    : m_window(nullptr), m_renderer(nullptr), m_texture(nullptr), m_chip8(nullptr)
{
    m_window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h, SDL_WINDOW_SHOWN
    );

    if (m_window == NULL)
    {
        printf("SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    SDL_RenderSetLogicalSize(m_renderer, w, h);

    // Create a texture to draw as a pixel.
    // Each pixel will be 64*32 on screen.
    m_texture = SDL_CreateTexture(m_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        64, 32);
};

Window::~Window()
{
    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyTexture(m_texture);
};

void Window::Connect(CHIP8* chip8)
{
    m_chip8 = chip8;
};

void Window::Draw()
{
    if (m_chip8 == nullptr)
    {
        std::cout << "Render Error : Window Cannot find CHIP" << std::endl;
        exit(1);
    }

    // Pixels buffer for rendering.
    // Using 4 bytes to represent color.
    uint32_t pixels[CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT];

    if (m_chip8->draw_flag) {
        m_chip8->draw_flag = 0;

        for (int i = 0; i < CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT; ++i) {
            uint32_t pixel = static_cast<uint32_t>(m_chip8->screen[i]);
            // White == 0xFFFFFFFF ; Black == 0xFF000000;
            //
            // if pixel == 1 => Draw White block on screen.
            // else          => Draw Black block on screen.
            pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
        }
        
        // SDL has to know to decode data inside pixels buffer.
        // Since there should be 64 pixel on each row and each pixel is 4 bytes long,
        // the pitch value should set to 64 * sizeof(Uint32).
        SDL_UpdateTexture(m_texture, NULL, pixels, 64 * sizeof(Uint32));
        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
        SDL_RenderPresent(m_renderer);
    }
}