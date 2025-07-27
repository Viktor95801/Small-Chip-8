# Small Chip-8

Header-only stb-style lib.
Implements Chip-8 + a waiting 0xF0FF instruction.
Very barebones, it doesn't depend on any of the C stdlib so you need to provide your own IO + rendering (you can use `test/sc8_renderer.c` as an emulator and if you're rolling your own with this lib, you can define the SC8_USE_STDIO macro to use STDIO for IO).

## TODO

- [ ] Solve GFX bugs (WHAT THE HECK IS ACTUALLY WRONG?????)
- [x] Example SDL3 Renderer:
  - [x] Implement rendering to screen
  - [x] Implement proper beep
