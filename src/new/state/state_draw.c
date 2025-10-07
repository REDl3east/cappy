#include "app.h"
#include "state.h"

bool app_event_draw(app_t* app, SDL_Event* event) {
  return false;
}

void app_iterate_draw(app_t* app) {
  SDL_FRect r = {0, 0, 250, 250};
  SDL_SetRenderDrawColor(app->renderer, 255, 128, 0, 255);
  SDL_RenderRect(app->renderer, &r);
}