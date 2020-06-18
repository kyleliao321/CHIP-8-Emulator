#include "pch.h"

#include "CHIP8.h"
#include "Window.h"
#include "EventHandler.h"

int main(int argc, char* argv[])
{
    char file[100];

#ifdef _WIN64
    // In windows system, using Windows File System API to select file.
    OPENFILENAMEA ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = file;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = "All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (!GetOpenFileNameA(&ofn))
        exit(-1);

#elif __linux__
    // In linux system, use command line to select file.
    if (argc != 2)
    {
        std::cout << "Usage : ./CHIP8-Emulator <File Path>";
        exit(1);
    }

    file = argv[1];
#endif

    // Initialize SDL for graphic and audio.
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    CHIP8 chip8;

    int w = 1024;
    int h = 512;

    Window window("CHP8 Game", w, h);
    window.Connect(&chip8);

    EventHandler eventHandler;
    eventHandler.Connect(&chip8);

    chip8.Load(file);

    while (true)
    {
        auto start = std::chrono::high_resolution_clock::now();

        chip8.EmulateCycle();
        eventHandler.HandleEvent();
        window.Draw();

        auto elapsed = std::chrono::high_resolution_clock::now() - start;

        long long delt_t = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

        // Since the timer counts in 60 Hz, put thread holds for 0.013ms each cycle.
        if (delt_t < CHIP8_MICROSECOND_PER_CYCLE)
            std::this_thread::sleep_for(std::chrono::microseconds(CHIP8_MICROSECOND_PER_CYCLE - delt_t));
    }

    return 0;
}