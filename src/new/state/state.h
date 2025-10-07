#ifndef _STATE_H_
#define _STATE_H_

typedef struct flashlight_zoom_t {
  float size;
  float zooming;
  bool zoom_in;
  float zoom_amount;
  float zoom_ms;
  float zoom_tick;
  float zoom_elapsed;
  float zoom_size_per_ms;
} flashlight_zoom_t;

typedef enum app_state_t {
  APP_MOVE_STATE,
  APP_COLOR_STATE,
  APP_DRAW_STATE,
  APP_FLASHLIGHT_STATE,
  APP_STATE_COUNT
} app_state_t;

typedef struct app_t app_t;

bool app_event_move(app_t* app, SDL_Event* event);
void app_iterate_move(app_t* app);
bool app_event_color(app_t* app, SDL_Event* event);
void app_iterate_color(app_t* app);
bool app_event_flashlight(app_t* app, SDL_Event* event);
void app_iterate_flashlight(app_t* app);
bool app_event_draw(app_t* app, SDL_Event* event);
void app_iterate_draw(app_t* app);



#endif // _STATE_H_