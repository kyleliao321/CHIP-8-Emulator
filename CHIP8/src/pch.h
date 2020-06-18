#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>
#include <bitset>
#include <thread>
#include <stdlib.h>
#include <queue>

#ifdef _WIN64
#include <Windows.h>
#include <SDL.h>
#elif __linux__
#include <SDL2/SDL.h>
#endif