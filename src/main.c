#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include "util.h"
#include "machine.h"
#include "input.h"
#include "renderer.h"

int main(const int argc, const char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Please specify a program ROM file!\n\tUsage: %s <file.from>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], ":-dbgfo") == 0)
    {
        printf("Target FPS = %i\n", TARGET_FPS);
        printf("Screen = %ix%i\n", SCREEN_WIDTH, SCREEN_HEIGHT);
        printf("Registers count = %i\n", REGISTERS_COUNT);
        printf("RAM section sizes = Program:%i, VRAM:%i, Input:%i, User:%i\n", PROGRAM_SIZE, VRAM_SIZE, INPUT_SIZE, USER_SIZE);
        printf("RAM start positions = Program:%i, VRAM:%i, Input:%i, User:%i\n", PROGRAM_START, VRAM_START, INPUT_START, USER_START);
        return 0;
    }

    Machine machine = machine_init();
    machine_reset(&machine);
    if (renderer_init(16) != MS_OK)
    {
        fprintf(stderr, "Unable to initialize renderer!");
        return 1;
    }

    program_romfile(argv[1]);
    if (program_loadrom(&machine) == 1)
        return 1;

    byte running = 1;
    Uint64 next_frame_time = SDL_GetTicksNS();
    while (running && machine.status == MS_OK)
    {
        Uint64 frame_start = SDL_GetTicksNS();
        SDL_Event event;
        while (SDL_PollEvent(&event))
            if (event.type == SDL_EVENT_QUIT)
                running = 0;

        for (int i = 0; i < CYCLES_PER_FRAME; i++)
        {
            input_poll(&machine);
            machine_step(&machine);
            if (machine.status != MS_OK)
                break;
        }

        renderer_frame(&machine);
        next_frame_time += NS_PER_FRAME;
        Uint64 current_time = SDL_GetTicksNS();
        if (next_frame_time > current_time)
            SDL_DelayNS(next_frame_time - current_time);
        else
            next_frame_time = current_time;
    }

    renderer_cleanup();
    return 0;
}
