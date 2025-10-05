#include "SDL3/SDL.h"
#include "SDL3_ttf/SDL_ttf.h"

#include "camera.h"
#include "capture.h"
#include "config.h"

typedef struct app_t {
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* capture_texture;
  config_t config;
  camera_t camera;
  TTF_Font* font;
  SDL_Surface* icon;
  SDL_Cursor* move_cursor;
  SDL_Cursor* default_cursor;

  int current_x;
  int current_y;
  int current_w;
  int current_h;

  float last_x;
  float last_y;

  bool grid_enabled;
} app_t;

SDL_AppResult app_init(app_t* app, int argc, char** argv);
SDL_AppResult app_event(app_t* app, SDL_Event* event);
SDL_AppResult app_iterate(app_t* app);
void app_quit(app_t* app, SDL_AppResult result);

#ifndef MIN
  #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
  #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif