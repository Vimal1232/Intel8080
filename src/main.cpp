#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <_types/_uint16_t.h>
#include <_types/_uint32_t.h>
#include <_types/_uint8_t.h>

#include <Memory.hpp>
#include <cpu.hpp>
#include <iostream>

int SCREEN_WIDTH = 224;
int SCREEN_HEIGHT = 256;

SDL_Window *gWindow = NULL;
SDL_Renderer *Render = NULL;
SDL_Texture *Texture = NULL;
uint32_t *Pixels = NULL;

bool init() {
  bool success = true;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cout << "SDL not initialized: " << SDL_GetError() << std::endl;
    success = false;
  } else {
    gWindow = SDL_CreateWindow("Space Invaders", SCREEN_WIDTH * 2,
                               SCREEN_HEIGHT * 2, 0);
    if (gWindow == NULL) {
      std::cout << "Window Not Created: " << SDL_GetError() << std::endl;
      success = false;
    } else {
      Render = SDL_CreateRenderer(gWindow, NULL);
      if (Render == NULL) {
        std::cout << "Renderer could not be created: " << SDL_GetError()
                  << std::endl;
        success = false;
      } else {
        Texture = SDL_CreateTexture(Render, SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
                                    SCREEN_HEIGHT);

        if (Texture == NULL) {
          std::cout << "Texture could not be created: " << SDL_GetError()
                    << std::endl;
          success = false;
        } else {
          Pixels = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];
        }
      }
    }
  }

  return success;
}

void UpdateWindow(Memory &mem) {
  uint16_t vram = 0x2400;

  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      int byteIndex = ((x * SCREEN_HEIGHT) + y) / 8;
      int bit = y % 8;
      uint8_t byte = mem.read(vram + byteIndex);
      bool On = (byte >> bit) & 1;
      Pixels[(SCREEN_HEIGHT - 1 - y) * SCREEN_WIDTH + x] =
          On ? 0xFFFFFFFF : 0xFF000000;
    }
  }

  SDL_UpdateTexture(Texture, NULL, Pixels, SCREEN_WIDTH * sizeof(uint32_t));
  SDL_SetRenderDrawColor(Render, 0, 0, 0, 255);
  SDL_RenderClear(Render);
  SDL_RenderTexture(Render, Texture, NULL, NULL);
  SDL_RenderPresent(Render);
}

int main() {
  Memory mem;
  mem.Load_ROM();
  CPU cpu(mem);

  if (!init()) {
    std::cout << "Failed to INitalise" << std::endl;
    return -1;
  }

  bool quit = false;
  int interrupt_toggle = 0;

  while (!quit) {
    uint32_t frameStart = SDL_GetTicks();

    for (int i = 0; i < 33333; i++) {
      cpu.cycle();
    }

    if (cpu.intrruptsEn) {
      if (interrupt_toggle == 0) {
        cpu.interrupt(0xCF);
      } else {
        cpu.interrupt(0xD7);
      }
      interrupt_toggle ^= 1;
    }

    UpdateWindow(mem);
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        quit = true;
      }
    }
    SDL_Delay(1000 / 60);
  }
  if (Pixels) {
    delete[] Pixels;
    Pixels = NULL;
  }
  if (Texture) {
    SDL_DestroyTexture(Texture);
    Texture = NULL;
  }
  if (Render) {
    SDL_DestroyRenderer(Render);
    Render = NULL;
  }
  if (gWindow) {
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
  }

  SDL_Quit();
}