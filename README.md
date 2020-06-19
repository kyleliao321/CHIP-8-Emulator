# CHIP8

A CHIP8 Emulator written in C++. To support cross-platform usage, I use SDL2 for Graphic & Audio programming. I documented the entire project as many detail as I can, hopefully it can help others to understand the hardware specification of CHIP8 and how to implement basic Graphic & Audio for it. 

The objection of this project is mainly to help myself get familiar with the features in C++ project, including the following items:

- Build System - Premake5
- Visual Studio Setting
- Cross-Platform Development
- Basic Audio Programming
- Precompiled Headers
- Documenting

## Setup Project
- All system required premake5 executable file in the root directory.
- The output exeutable file will be located in **/bin/\<configuration\>-\<system\>-\<platform\>/CHIP8/** directory. Ex: **/bin/Debug-linux-x86-64/CHIP8/**
### Windows
> Using premake5 to generate the project in visual studio
```shell
## Use Windows Command Prompt
premake5 vs2019
```

### Linux
- Requirements: SDL2 linux distribution.
> Install SDL2 on Linux and generate MakeFile
```shell
### Using any shell environment
sudo apt install libsdl2, libsdl2-dev
premake5 gmake
make
```

### OSX
- Not Support yet.