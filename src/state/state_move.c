#include "app.h"
#include "state.h"

bool app_event_move(app_t* app, SDL_Event* event) {
  return false;
}

void app_iterate_move(app_t* app) {
  camera_update(&app->camera);
}