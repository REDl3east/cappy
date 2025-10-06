#include "SDL3/SDL.h"
#include "SDL3_ttf/SDL_ttf.h"

#include "camera.h"
#include "capture.h"
#include "config.h"

typedef enum app_state_t {
  APP_MOVE_STATE,
  APP_COLOR_STATE,
  APP_DRAW_STATE,
  APP_FLASHLIGHT_STATE,
} app_state_t;

typedef struct app_t {
  SDL_Window* window;
  SDL_Renderer* renderer;
  capture_t capture;
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

  bool recompute_text;
  SDL_Texture* text_texture;

  app_state_t state;
} app_t;

SDL_AppResult app_init(app_t* app, int argc, char** argv);
SDL_AppResult app_event(app_t* app, SDL_Event* event);
SDL_AppResult app_iterate(app_t* app);
void app_quit(app_t* app, SDL_AppResult result);

bool app_event_move(app_t* app, SDL_Event* event);
void app_iterate_move(app_t* app);
bool app_event_color(app_t* app, SDL_Event* event);
void app_iterate_color(app_t* app);

#ifndef MIN
  #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
  #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif