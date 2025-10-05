#include "main.h"

SDL_AppResult SDL_AppInit(void** app_ptr, int argc, char** argv) {
  *app_ptr = (app_t*)SDL_calloc(sizeof(app_t), 1);
  if (*app_ptr == NULL) return SDL_APP_FAILURE;
  app_t* app = (app_t*)*app_ptr;

  app->width  = 1440;
  app->height = 810;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("c-color-convert - playground", app->width, app->height, SDL_WINDOW_HIDDEN, &app->window, &app->renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_ShowWindow(app->window);

  return app_init(app, argc, argv);
}

SDL_AppResult SDL_AppEvent(void* app_ptr, SDL_Event* event) {
  return app_event((app_t*)app_ptr, event);
}

SDL_AppResult SDL_AppIterate(void* app_ptr) {
  app_t* app = (app_t*)app_ptr;

  SDL_AppResult result = app_iterate(app);
  if (result == SDL_APP_SUCCESS || result == SDL_APP_FAILURE) return result;

  SDL_RenderPresent(app->renderer);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_ptr, SDL_AppResult result) {
  app_t* app = (app_t*)app_ptr;

  if (app != NULL) {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
  }

  app_quit((app_t*)app_ptr, result);
  SDL_free(app);
}