#include <stdlib.h>

#define SC8_USE_STDIO
#define SC8_IMPLEMENTATION
#include "../smallCHIP-8.h"

#define SDL_MAIN_HANDLED
#include "SDL3/SDL.h"

#ifdef _WIN32
#include <windows.h>
#endif

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

static SDL_AudioStream *beep_stream = NULL;
#define BEEP_FREQ 440
void sc8_beep(void) {
    static int sine_sample = 0;

    static float samples[725]; // I found 725 to be a sweet spot
    for(size_t i = 0; i < SDL_arraysize(samples); i++) {
        const float phase = sine_sample * BEEP_FREQ / 8000.0f;
        samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
        sine_sample++;
    }

    sine_sample %= 8000;

    SDL_PutAudioStreamData(beep_stream, samples, sizeof(samples));
}
#undef BEEP_FREQ

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
        return err;
    }

    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "SDL_Init has failed: %s", SDL_GetError());
        return 1;  
    }

    SDL_Window *w = SDL_CreateWindow("CHIP-8", SC8_W * PIXEL_SCALE, SC8_H * PIXEL_SCALE, 0);
    if(w == NULL) {
        SDL_Log("Error at creating window: %s", SDL_GetError());
        return 1;
    }
    SDL_Renderer *r = SDL_CreateRenderer(w, NULL);
    if(r == NULL) {
        SDL_Log("Error creating renderer: %s", SDL_GetError());
        return 1;
    }

    SDL_AudioSpec beep_stream_spec;
    beep_stream_spec.channels = 1;
    beep_stream_spec.format = SDL_AUDIO_F32;
    beep_stream_spec.freq = 8000;
    beep_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &beep_stream_spec, NULL, NULL
    );
    if(beep_stream == NULL) {
        SDL_Log("Failed to create beep audio stream: %s", SDL_GetError());
        return 1;
    }
    SDL_ResumeAudioStreamDevice(beep_stream);

    quit = false;
    while(!quit) {
        sc8_step(&state);

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT)
                quit = true;
        }

        if(state.drawFlag) {
            SDL_SetRenderDrawColor(r, 0x18, 0x18, 0x18, 255);
            SDL_RenderClear(r);
            
            SDL_SetRenderDrawColor(r, 90, 255, 90, 255);
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
            }
            state.drawFlag = false;
            SDL_RenderPresent(r);
        }

        SDL_Delay(64);
    }

    SDL_DestroyWindow(w);
    SDL_DestroyRenderer(r);
    SDL_Quit();

    return 0;
}
