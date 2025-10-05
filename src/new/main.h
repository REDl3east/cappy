#include "SDL3/SDL.h"

#include "capture.h"
#include "config.h"

typedef struct app_t {
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* capture_texture;
  config_t config;
} app_t;

SDL_AppResult app_init(app_t* app, int argc, char** argv);
SDL_AppResult app_event(app_t* app, SDL_Event* event);
SDL_AppResult app_iterate(app_t* app);
void app_quit(app_t* app, SDL_AppResult result);