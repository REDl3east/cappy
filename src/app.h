#ifndef _APP_H_
#define _APP_H_

#include "SDL3/SDL.h"
#include "SDL3_ttf/SDL_ttf.h"

#include "camera.h"
#include "capture.h"
#include "config.h"
#include "state/state.h"

typedef struct app_t {
  SDL_Window* window;
  SDL_Renderer* renderer;
  capture_t capture;
  SDL_Texture* capture_texture;
  config_t config;
  camera_t camera;
  float camera_scale_to_change;
  TTF_Font* font;
  SDL_Surface* icon;

  SDL_Cursor* move_cursor;
  SDL_Cursor* default_cursor;
  SDL_Cursor* crosshair_cursor;
  SDL_Cursor* ns_cursor;
  SDL_Cursor* ew_cursor;
  SDL_Cursor* nwse_cursor;
  SDL_Cursor* nesw_cursor;
  SDL_Cursor* wait_cursor;

  int current_x;
  int current_y;
  int current_w;
  int current_h;
  float last_x;
  float last_y;
  bool grid_enabled;

  bool recompute_text;
  SDL_Texture* text_texture;

  flashlight_zoom_t flashlight_zoom;

  bool drawing;
  SDL_FPoint start_crop;
  SDL_FPoint end_crop;
  crop_resize_selection_t resize_selection;

  int using_file;

  int save_file_event;

  app_state_t state;
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

#endif // _APP_H_
