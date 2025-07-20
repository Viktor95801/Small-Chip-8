#include <stdlib.h>

#define SC8_USE_STDIO
#define SC8_IMPLEMENTATION
#include "../smallCHIP-8.h"

#define SDL_MAIN_HANDLED
#include "SDL3/SDL.h"

sc8_state state; // OH NO EVIL GLOBAL STATTE NONONONONO YOU CAN"T NOT DO THAT NOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
bool quit;

// Map SDL_Keycode to CHIP-8 key index (example mapping)
static int mapKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_Q: return 0x4;
        case SDLK_W: return 0x5;
        case SDLK_E: return 0x6;
        case SDLK_R: return 0xD;
        case SDLK_A: return 0x7;
        case SDLK_S: return 0x8;
        case SDLK_D: return 0x9;
        case SDLK_F: return 0xE;
        case SDLK_Z: return 0xA;
        case SDLK_X: return 0x0;
        case SDLK_C: return 0xB;
        case SDLK_V: return 0xF;
        default:     return -1; // Not a CHIP-8 key
    }
}

void sc8_updateKeyArray(sc8_state *state) {
    SDL_Event events[16];
    int num = SDL_PeepEvents(events, 16, SDL_PEEKEVENT, SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);

    for(int i = 0; i < num; i++) {
        SDL_Event event = events[16];
        int key = mapKey(event.key.key);
        if(key >= 0 && key <= 16) {
            state->key[key] = event.type == SDL_EVENT_KEY_DOWN;
        }
    }
}
void sc8_beep(void) {
    printf("pretend to beep\n");
}

#define PIXEL_SCALE 10

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "Expected usage: %s <ROM file path>\n", argv[0]);
        return 1;
    }

    sc8_init(&state);
    int err = sc8_loadFile(&state, argv[1]);
    if(err != sc8_loadFile_OK) {
        fprintf(stderr, "Error loading file, code: %d\n", err);
        //return err;
    }

    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "SDL_Init has failed: %s", SDL_GetError());
        return 1;  
    }

    SDL_Window *w = SDL_CreateWindow("CHIP-8", SC8_W * PIXEL_SCALE, SC8_H * PIXEL_SCALE, 0);
    SDL_Renderer *r = SDL_CreateRenderer(w, NULL);

    quit = false;
    while(!quit) {
        sc8_step(&state);

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT)
                quit = true;
        }

        if(state.drawFlag) {
            SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
            SDL_RenderClear(r);
            printf("pretend to render\n");
            
            SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
            for(int y = 0; y < SC8_H; y++) {
                for(int x = 0; x < SC8_W; x++) {
                    if(state.gfx[y * SC8_W + x]) {
                        SDL_FRect rect = {
                            x * PIXEL_SCALE, y * PIXEL_SCALE,
                            PIXEL_SCALE, PIXEL_SCALE
                        };
                        SDL_RenderFillRect(r, &rect);
                    }
                }
                state.drawFlag = false;
                SDL_RenderPresent(r);
            }

        }

        SDL_Delay(64);
    }

    SDL_DestroyWindow(w);
    SDL_DestroyRenderer(r);
    SDL_Quit();

    return 0;
}
