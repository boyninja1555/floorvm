#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <SDL3/SDL_events.h>
#include "machine.h"
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
        printf("FPS = %i\n", FPS);
        printf("Screen = %ix%i\n", SCREEN_WIDTH, SCREEN_HEIGHT);
        printf("Registers count = %i\n", REGISTERS_COUNT);
        printf("Start positions = Program:%i, VRAM:%i, User:%i\n", PROGRAM_START, VRAM_START, USER_START);
        return 0;
    }

    Machine machine = machine_init();
    machine_reset(&machine);
    if (renderer_init(16) != MS_OK)
    {
        fprintf(stderr, "Unable to initialize renderer!");
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file)
    {
        fprintf(stderr, "ROM does not exist!\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size > PROGRAM_SIZE)
    {
        fprintf(stderr, "ROM too large (%ld bytes), must be %i bytes!\n", size, PROGRAM_SIZE);
        fclose(file);
        return 1;
    }

    fread(machine.ram, sizeof(byte), PROGRAM_SIZE, file);
    fclose(file);

    byte running = 1;
    while (running && machine.status == MS_OK)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = 0;
        }

        machine_step(&machine);
        renderer_frame(&machine);
    }

    renderer_cleanup();
    return 0;
}
