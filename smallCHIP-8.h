#ifndef SMALL_CHIP_8_HEADER
#define SMALL_CHIP_8_HEADER

/*
The MIT License

Copyright (c) 2025 Viktor Hugo C.M.G.

Permission is hereby granted, free of charge, to any person obtaining a copy 
    of this software and associated documentation files (the “Software”), to deal 
    in the Software without restriction, including without limitation the rights 
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
    copies of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in 
    all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
    THE SOFTWARE.

*/

/*
Resources used:
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.1
https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
*/

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SC8_ATTR_FORMAT(a, b) __attribute__((format(printf, a, b)))

#define SC8_LSB(val) ((val) & 1)
#define SC8_MSB(val) ((val) >> (sizeof(val)*8 - 1) & 1) // not portable blah blah blah I don't care

#define SC8_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define SC8_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SC8_CLAMP(value, min, max) SC8_MIN(SC8_MAX(value, min), max)

#define SC8_N(value)   ((value) & 0x000F)
#define SC8_NNN(value) ((value) & 0x0FFF)
#define SC8_Vx(value) (((value) & 0x0F00) >> 8)
#define SC8_Vy(value) (((value) & 0x00F0) >> 4)
#define SC8_KK(value) ((value) & 0x00FF)

// changing this value probably won't expand the memory,
// it's only here so I don't need to remember 4096 and type it out everywhere
#define MEMORY_SIZE 4096
// changing those won't have effect as well
#define SC8_W 64
#define SC8_H 32
typedef struct {
    uint8_t memory[MEMORY_SIZE];
    bool gfx[SC8_W * SC8_H];
    
    // The user should be aware of the draw flag and render the gfx array
    // properly to the screen whenever it's set.
    bool drawFlag;
    // The user should handle this array (since sc8 doesn't enforce any IO library),
    // handling this array means defining the sc8_updateKeyArray function.
    // The recomended layout is as follows:
    // +-+-+-+-+    +-+-+-+-+
    // |1|2|3|C|    |1|2|3|4|
    // +-+-+-+-+    +-+-+-+-+
    // |4|5|6|D|    |Q|W|E|R|
    // +-+-+-+-+ -> +-+-+-+-+
    // |7|8|9|E|    |A|S|D|F|
    // +-+-+-+-+    +-+-+-+-+
    // |A|0|B|F|    |Z|X|C|V|
    // +-+-+-+-+    +-+-+-+-+
    //
    // note: The index is the key value (for example,
    // if the key 2 was pressed, the key at index 0x2 should be set to true)
    // ```c
    // state.key[key_pressed] = true; // , as simple as that
    // ```
    // 
    // WARNING: NOT HANDLING THIS PROPERLY MAY AND WILL LEAD TO YOUR EMULATED PROGRAMS NOT WORKING
    //
    bool key[16];

    uint16_t opcode;

    union {
        uint8_t v[16];
        struct {
            uint8_t v0;
            uint8_t v1;
            uint8_t v2;
            uint8_t v3;
            uint8_t v4;
            uint8_t v5;
            uint8_t v6;
            uint8_t v7;
            uint8_t v8;
            uint8_t v9;
            uint8_t va;
            uint8_t vb;
            uint8_t vc;
            uint8_t vd;
            uint8_t ve;
            uint8_t vf;
        };
    };

    uint16_t i;
    uint16_t pc;

    uint8_t dt;
    uint8_t st;

    uint8_t stack[16];
    uint8_t sp;
} sc8_state;

// file hanlde
typedef void* sc8_fh;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// IO declarations (user defined)

sc8_fh sc8_fopen(const char *file_path, const char *mode);
void sc8_fclose(sc8_fh fh);
size_t sc8_fread(void *buffer, size_t size, size_t count, sc8_fh fh);
bool sc8_fnil(sc8_fh fh);

int sc8_errprintf(const char *format, ...) SC8_ATTR_FORMAT(1, 2);

// media IO declarations (user defined as well)

// the user should define this function for handling the key array
void sc8_updateKeyArray(sc8_state *state);
void sc8_beep(void);

// random generator

extern uint32_t sc8_xorRandState;
uint32_t sc8_xorRand();
#define sc8_defRand() sc8_xorRand() // you can modify this

// sc8 emulator

void sc8_init(sc8_state *state);

void sc8_loadRom(sc8_state *state, const uint8_t *rom, size_t rom_size);
typedef enum {
    sc8_loadFile_OK,
    sc8_loadFile_EmptyROM,
    sc8_loadFile_fopenError,
} sc8_LoadFileResult;
sc8_LoadFileResult sc8_loadFile(sc8_state *state, const char *file_path);

void sc8_loadRomPad(sc8_state *state, const uint8_t *rom, size_t rom_size, int padding);
sc8_LoadFileResult sc8_loadFilePad(sc8_state *state, const char *file_path, int padding);

bool sc8_step(sc8_state *state);

// define `SC8_NO_DEFAULT_FONTSET` to disable the default fontset (it's 8x5 pixels for char)
extern const uint8_t sc8_fontset[80];

#ifndef SC8_NO_DEFAULT_FONTSET
const uint8_t sc8_fontset[80] = { 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
#endif // SC8_NO_DEFAULT_FONTSET

#ifdef SC8_USE_STDIO
#include <stdarg.h>
#include <stdio.h>

sc8_fh sc8_fopen(const char *file_path, const char *mode) {
    return fopen(file_path, mode);
}
void sc8_fclose(sc8_fh fh) {
    fclose(fh);
}
size_t sc8_fread(void *buffer, size_t size, size_t count, sc8_fh fh) {
    return fread(buffer, size, count, fh);
}
bool sc8_fnil(sc8_fh fh) {
    return fh == NULL;
}

int sc8_errprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stderr, format, args);
    va_end(args);
    return ret;
}
#endif // SC8_USE_STDIO

#define SC8_IMPLEMENTATION
#ifdef SC8_IMPLEMENTATION
uint32_t sc8_xorRandState = 305419896;

uint32_t sc8_xorRand() {
    sc8_xorRandState ^= sc8_xorRandState << 13;
    sc8_xorRandState ^= sc8_xorRandState >> 17;
    sc8_xorRandState ^= sc8_xorRandState << 5;
    return sc8_xorRandState;
}

void sc8_init(sc8_state *state) {
    memset(state, 0, sizeof(sc8_state));
    state->pc = 512;

    // load fontset
    memcpy(state->memory, sc8_fontset, 80);
    // for(int i = 0; i < 80; i++) {
    //     state->memory[i] = sc8_fontset[i];
    // }
}

void sc8_loadRom(sc8_state *state, const uint8_t *rom, size_t rom_size) {
    assert((rom_size < (MEMORY_SIZE - 512)) && "The ROM size is greater than the maximum memory size");
    memcpy(state->memory + 512, rom, rom_size);
}

sc8_LoadFileResult sc8_loadFile(sc8_state *state, const char *file_path) {
    sc8_fh f = sc8_fopen(file_path, "rb");
    if(sc8_fnil(f)) {
        return sc8_loadFile_fopenError;
    }

    size_t rom_size = sc8_fread(state->memory + 512, 1, MEMORY_SIZE - 512, f);
    sc8_fclose(f);

    return rom_size == 0;
}

void sc8_loadRomPad(sc8_state *state, const uint8_t *rom, size_t rom_size, int padding) {
    assert((rom_size < (size_t)(MEMORY_SIZE - padding)) && "The ROM size is greater than the maximum memory size");
    memcpy(state->memory + padding, rom, rom_size);
}

sc8_LoadFileResult sc8_loadFilePad(sc8_state *state, const char *file_path, int padding) {
    sc8_fh f = sc8_fopen(file_path, "rb");
    if(sc8_fnil(f)) {
        return sc8_loadFile_fopenError;
    }

    size_t rom_size = sc8_fread(state->memory + padding, 1, MEMORY_SIZE - padding, f);
    sc8_fclose(f);

    return rom_size == 0;
}

bool sc8_step(sc8_state *state) {
    state->opcode = state->memory[state->pc] << 8 | state->memory[state->pc + 1];
    uint16_t opcode = state->opcode;

    sc8_updateKeyArray(state);

    bool unknown_opcode = false;
    switch(opcode & 0xF000) {
        case 0x0000: {
            switch(opcode & 0x000F) {
                case 0x0000: {
                    memset(state->gfx, 0, sizeof(state->gfx));
                    state->drawFlag = true;
                    state->pc += 2;
                } break;
                case 0x000E: {
                    state->pc = state->stack[state->sp--];
                    state->pc += 2;
                } break;
                default: {
                    sc8_errprintf("Unknown opcode: %04X\n", opcode);
                    unknown_opcode = true;
                    state->pc+=2;
                } break;
            }
        } break;
        case 0x1000: {
            state->pc = SC8_NNN(opcode);
        } break;
        case 0x2000: {
            state->stack[state->sp++] = state->pc;
            state->pc = SC8_NNN(opcode);
        } break;
        case 0x3000: {
            state->pc += (state->v[SC8_Vx(opcode)] == SC8_KK(opcode)) ? 4 : 2;
        } break;
        case 0x4000: {
            state->pc += (state->v[SC8_Vx(opcode)] != SC8_KK(opcode)) ? 4 : 2;
        } break;
        case 0x5000: {
            state->pc += (state->v[SC8_Vx(opcode)] == state->v[SC8_Vy(opcode)]) ? 4 : 2;
        } break;
        case 0x6000: {
            state->v[SC8_Vx(opcode)] = SC8_KK(opcode);
            state->pc += 2;
        } break;
        case 0x7000: {
            state->v[SC8_Vx(opcode)] += SC8_KK(opcode);
            state->pc += 2;
        } break;
        case 0x8000: {
            switch(opcode & 0x000F) {
                case 0x0000: {
                    state->v[SC8_Vx(opcode)] = SC8_Vy(opcode);
                } break;
                case 0x0001: {
                    state->v[SC8_Vx(opcode)] |= SC8_Vy(opcode);
                } break;
                case 0x0002: {
                    state->v[SC8_Vx(opcode)] &= SC8_Vy(opcode);
                } break;
                case 0x0003: {
                    state->v[SC8_Vx(opcode)] ^= SC8_Vy(opcode);
                } break;
                case 0x0004: {
                    const uint8_t x = state->v[SC8_Vx(opcode)], y = state->v[SC8_Vy(opcode)];
                    size_t result = x + y;
                    state->vf = (result > 255) ? 1 : 0;
                    state->v[SC8_Vx(opcode)] = result;
                } break;
                case 0x0005: {
                    const uint8_t x = state->v[SC8_Vx(opcode)], y = state->v[SC8_Vy(opcode)];
                    state->vf = (x > y) ? 1 : 0;
                    state->v[SC8_Vx(opcode)] = x - y;
                } break;
                case 0x0006: {
                    const uint8_t x = state->v[SC8_Vx(opcode)];
                    state->vf = SC8_LSB(x) ? 1 : 0;
                    state->v[SC8_Vx(opcode)] >>= 1;
                } break;
                case 0x0007: {
                    const uint8_t x = state->v[SC8_Vx(opcode)], y = state->v[SC8_Vy(opcode)];
                    state->vf = (y > x) ? 1 : 0;
                    state->v[SC8_Vx(opcode)] = y - x;
                } break;
                case 0x000E: {
                    const uint8_t x = state->v[SC8_Vx(opcode)];
                    state->vf = SC8_MSB(x) ? 1 : 0;
                    state->v[SC8_Vx(opcode)] <<= 1;
                } break;
                
                default: {
                    sc8_errprintf("Unknown opcode: %04X\n", opcode);
                    unknown_opcode = true;
                    state->pc+=2;
                } break;
            }
            state->pc += 2;
        } break;
        case 0x9000: {
            state->pc += (state->v[SC8_Vx(opcode)] != state->v[SC8_Vy(opcode)]) ? 4 : 2;
        } break;
        case 0xA000: {
            state->i = SC8_NNN(opcode);
            state->pc += 2;
        } break;
        case 0xB000: {
            state->pc = SC8_NNN(opcode) + state->v0;
        } break;
        case 0xC000: {
            state->v[SC8_Vx(opcode)] = (uint8_t)sc8_defRand() & SC8_KK(opcode);
            state->pc += 2;
        } break;
        case 0xD000: {
            const uint8_t x = state->v[SC8_Vx(opcode)], y = state->v[SC8_Vy(opcode)];
            uint8_t height = SC8_N(opcode);
            
            state->vf = 0;
            for(int irow = 0; irow < height; irow++) {
                uint8_t row = state->memory[0 + irow];
                for(int ipixel = 0; ipixel < 8; ipixel++) {
                    bool pixel = (row & (0x80 >> ipixel)) != 0;
                    const int index = (x + ipixel) * ((y + irow) * SC8_W);
                    state->vf = pixel && (state->gfx[index] != 0);
                    state->gfx[index] ^= pixel;
                }
            }

            state->drawFlag = true;
            state->pc += 2;
        } break;
        case 0xE000: {
            switch(opcode & 0x00FF) {
                case 0x009E: {
                    state->pc += 
                        (state->key[state->v[SC8_Vx(opcode)]]) ? 4 : 2;
                } break;
                case 0x00A1: {
                    state->pc += 
                        !(state->key[state->v[SC8_Vx(opcode)]]) ? 4 : 2;
                } break;
                default: {
                    sc8_errprintf("Unknown opcode: %04X\n", opcode);
                    unknown_opcode = true;
                    state->pc+=2;
                } break;
            }
        } break;
        case 0xF000: {
            switch(opcode & 0x00FF) {
                case 0x0007: {
                    state->v[SC8_Vx(opcode)] = state->dt;
                    state->pc += 2;
                } break;
                case 0x000A: {
                    for(;;) {
                        for(int key = 0; key < 16; key++) {
                            if(state->key[key]) {
                                state->v[SC8_Vx(opcode)] = key;
                                goto OPCODE_0x000A_EXIT;
                            }
                        }
                        sc8_updateKeyArray(state);
                    }
OPCODE_0x000A_EXIT:
                    state->pc += 2;
                } break;
                case 0x0015: {
                    state->dt = state->v[SC8_Vx(opcode)];
                    state->pc += 2;
                } break;
                case 0x0018: {
                    state->st = state->v[SC8_Vx(opcode)];
                    state->pc += 2;
                } break;
                case 0x001E: {
                    state->i += state->v[SC8_Vx(opcode)];
                    state->pc += 2;
                } break;
                case 0x0029: {
                    state->i = state->v[SC8_Vx(opcode)] * 5; // neat trick, take a look at the sc8_fontset
                                                             // array to understand it.
                    state->pc += 2;
                } break;
                case 0x0033: {
                    const uint8_t x = state->v[SC8_Vx(opcode)];
                    state->memory[state->i] = x / 100;
                    state->memory[state->i + 1] = (x / 10) % 10;
                    state->memory[state->i + 2] = (x % 100) % 10;
                    state->pc += 2;
                } break;
                case 0x0055: {
                    for(int i = 0; i < SC8_Vx(opcode); i++) {
                        state->memory[state->i + i] = state->v[i];
                    }
                    state->pc += 2;
                } break;
                case 0x0065: {
                    for(int i = 0; i < SC8_Vx(opcode); i++) {
                        state->v[i] = state->memory[state->i + i];
                    }
                    state->pc += 2;
                } break;
                case 0x00FF: {
                    // this instruction is just a repeat, basically exits the program
                } break;

                default: {
                    sc8_errprintf("Unknown opcode: %04X\n", opcode);
                    unknown_opcode = true;
                    state->pc+=2;
                } break;
            }
        } break;

        default: {
            sc8_errprintf("Unknown opcode: %04X\n", state->opcode);
            unknown_opcode = true;
            state->pc+=2;
        } break;
    }

    if(state->dt > 0) {
        state->dt--;
    }
    if(state->st > 0) {
        sc8_beep();
        state->st--;
    }

    return !unknown_opcode;
}

#undef SC8_IMPLEMENTATION
#endif // SC8_IMPLEMENTATION

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SMALL_CHIP_8_HEADER