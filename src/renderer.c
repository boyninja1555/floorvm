#include "renderer.h"
#include <SDL3/SDL.h>

static SDL_Window *window = NULL;
static SDL_Renderer *sdl_renderer = NULL;
static SDL_Texture *texture = NULL;

static int scale = 1;

byte renderer_init(int gui_scale)
{
    scale = gui_scale;
    if (!SDL_Init(SDL_INIT_VIDEO))
        return MS_KO;

    window = SDL_CreateWindow("FloorVM", SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale, 0);
    if (!window)
        return MS_KO;

    sdl_renderer = SDL_CreateRenderer(window, NULL);
    if (!sdl_renderer)
        return MS_KO;

    SDL_SetRenderVSync(sdl_renderer, 1);
    texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!texture)
        return MS_KO;

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    return MS_OK;
}

void renderer_frame(const Machine *machine)
{
    uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        byte green = machine->ram[VRAM_START + i] / 2 + 20;
        pixels[i] = (0 << 24) |     // Red
                    (green << 16) | // Green
                    (0 << 8) |      // Blue
                    0xFF;           // Alpha
    }

    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(sdl_renderer);
    SDL_RenderTexture(sdl_renderer, texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);
}

void renderer_cleanup()
{
    if (texture)
        SDL_DestroyTexture(texture);

    if (sdl_renderer)
        SDL_DestroyRenderer(sdl_renderer);

    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();
}
