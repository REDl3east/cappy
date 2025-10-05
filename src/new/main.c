#include "main.h"

#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL_main.h"

SDL_AppResult app_init(app_t* app, int argc, char** argv) {
  
  return SDL_APP_CONTINUE;
}

SDL_AppResult app_event(app_t* app, SDL_Event* event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.key == SDLK_Q) {
      return SDL_APP_SUCCESS;
    }
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult app_iterate(app_t* app) {
  SDL_SetRenderDrawColor(app->renderer, 255, 128, 0, 255);
  SDL_RenderClear(app->renderer);

  return SDL_APP_CONTINUE;
}

void app_quit(app_t* app, SDL_AppResult result) {
  // free anything allocated within app variable.
  if (app != NULL) {
  }
}

