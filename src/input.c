#include "input.h"
#include <SDL3/SDL.h>
#include "util.h"

void input_poll(Machine *machine)
{
    const bool *state = SDL_GetKeyboardState(NULL);
    u8 input_byte = 0;

    if (state[SDL_SCANCODE_ESCAPE])
    {
        machine_reset(machine);
        program_loadrom(machine);
        return;
    }

    if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W])
        input_byte |= BTN_UP;

    if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S])
        input_byte |= BTN_DOWN;

    if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A])
        input_byte |= BTN_LEFT;

    if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D])
        input_byte |= BTN_RIGHT;

    if (state[SDL_SCANCODE_A])
        input_byte |= BTN_A;

    if (state[SDL_SCANCODE_D])
        input_byte |= BTN_D;

    if (state[SDL_SCANCODE_RETURN])
        input_byte |= BTN_SELECT;

    if (state[SDL_SCANCODE_BACKSPACE])
        input_byte |= BTN_START;

    machine->ram[INPUT_START] = input_byte;
}
