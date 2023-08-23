#include <iostream>
#include <memory>

#include "SDL3/SDL.h"
#include "capture.h"

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture) {
  std::shared_ptr<SDL_Surface> surface = std::shared_ptr<SDL_Surface>(SDL_CreateSurfaceFrom(capture.pixels, capture.width, capture.height, capture.stride * 3, SDL_PIXELFORMAT_RGB24), SDL_DestroySurface);
  return std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer.get(), surface.get()), SDL_DestroyTexture);
}

int main() {
  Capture capture;
  if (!capture.capture()) {
    std::cerr << "Failed to capture screen!\n";
    return 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to init SDL!\n";
    return 1;
  }

  std::shared_ptr<SDL_Window> window = std::shared_ptr<SDL_Window>(SDL_CreateWindow("Cappy", capture.width, capture.height, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL), SDL_DestroyWindow);
  if (!window) {
    std::cerr << "Failed to create window!\n";
    return 1;
  }

  std::shared_ptr<SDL_Renderer> renderer = std::shared_ptr<SDL_Renderer>(SDL_CreateRenderer(window.get(), NULL, SDL_RENDERER_ACCELERATED), SDL_DestroyRenderer);
  if (!renderer) {
    std::cerr << "Failed to create renderer!\n";
    return 1;
  }

  std::shared_ptr<SDL_Texture> texture = create_capture_texture(renderer, capture);

  if (!texture) {
    std::cerr << "Failed to create capture texture!\n";
    return 1;
  }

  SDL_Event event;
  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:{
          quit = true;
          break;
        }
        case SDL_EVENT_KEY_DOWN:{
          if(event.key.keysym.sym == SDLK_q){
            quit = true;
            break;
          }
          break;
        }
      }
    }

    SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);
    SDL_RenderClear(renderer.get());

    SDL_RenderTexture(renderer.get(), texture.get(), NULL, NULL);

    SDL_RenderPresent(renderer.get());
  }

  return 0;
}
