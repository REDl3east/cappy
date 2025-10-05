#include "main.h"

#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL_main.h"

SDL_Texture* create_capture_texture(SDL_Renderer* renderer, capture_t* capture);

SDL_AppResult app_init(app_t* app, int argc, char** argv) {
  capture_t capture = {0};
  if (!capture_screen(&capture)) {
    SDL_Log("Failed to capture screen!");
    return SDL_APP_FAILURE;
  }

  if (!config_init(&app->config)) {
    SDL_Log("Failed to initialize config!");
    return SDL_APP_FAILURE;
  }

  config_print(&app->config);

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("Failed to init SDL!");
    return SDL_APP_FAILURE;
  }

  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Cappy");
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, capture.width);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, capture.height);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 0);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 0);

  if (app->config.window_fullscreen) {
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, 1);
  } else {
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, 1);
  }

  app->window = SDL_CreateWindowWithProperties(props);
  if (app->window == NULL) {
    SDL_Log("Failed to create window!");
    SDL_DestroyProperties(props);
    return SDL_APP_FAILURE;
  }
  SDL_DestroyProperties(props);

  app->renderer = SDL_CreateRenderer(app->window, NULL);
  if (app->renderer == NULL) {
    SDL_Log("Failed to create renderer!");
    return SDL_APP_FAILURE;
  }

  SDL_SetRenderVSync(app->renderer, 1);
  SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);

  app->capture_texture = create_capture_texture(app->renderer, &capture);
  if (app->capture_texture == NULL) {
    SDL_Log("Failed to create capture texture %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  capture_free(&capture);

  SDL_SetTextureScaleMode(app->capture_texture, SDL_SCALEMODE_NEAREST);

  // if (TTF_Init() < 0) {
  //   SDL_Log("Failed to init TTF!");
  //   return 1;
  // }

  // TTF_Font* font = TTF_OpenFontIO(SDL_IOFromConstMem(advanced_pixel_7, sizeof(advanced_pixel_7)), SDL_TRUE, 36);
  // if (!font) {
  //   SDL_Log("Failed to load font: %s", TTF_GetError());
  //   return 1;
  // }

  // std::shared_ptr<SDL_Surface> icon = std::shared_ptr<SDL_Surface>(SDL_CreateSurfaceFrom(icon_data, ICON_WIDTH, ICON_HEIGHT, ICON_WIDTH * 4, SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
  // SDL_SetWindowIcon(window.get(), icon.get());

  // CameraSmooth camera;
  // auto machine = CappyMachine::make(config, renderer, capture, texture, camera, font);
  // machine->set_state<MoveState>();

  // // setting bounds in capture, if width or height is <= 0
  // // then it sets the crop to the capture width or height.
  // if (config.window_pre_crop[0] < 0) config.window_pre_crop[0] = 0;
  // if (config.window_pre_crop[1] < 0) config.window_pre_crop[1] = 0;
  // if (config.window_pre_crop[2] <= 0) config.window_pre_crop[2] = capture.width;
  // if (config.window_pre_crop[3] <= 0) config.window_pre_crop[3] = capture.height;

  // if (config.window_pre_crop[0] > capture.width) config.window_pre_crop[0] = capture.width;
  // if (config.window_pre_crop[1] > capture.height) config.window_pre_crop[1] = capture.height;
  // if (config.window_pre_crop[2] > capture.width) config.window_pre_crop[2] = capture.width;
  // if (config.window_pre_crop[3] > capture.height) config.window_pre_crop[3] = capture.height;

  // // set x and y to top left most point
  // // and calculate width and height
  // machine->current_x = std::min(config.window_pre_crop[0], config.window_pre_crop[2]);
  // machine->current_y = std::min(config.window_pre_crop[1], config.window_pre_crop[3]);
  // machine->current_w = std::abs(config.window_pre_crop[2] - config.window_pre_crop[0]);
  // machine->current_h = std::abs(config.window_pre_crop[3] - config.window_pre_crop[1]);

  // std::shared_ptr<SDL_Cursor> move_cursor = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL), SDL_DestroyCursor);
  // SDL_Cursor* default_cursor              = SDL_GetDefaultCursor();

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
    SDL_DestroyTexture(app->capture_texture);
  }
}

SDL_Texture* create_capture_texture(SDL_Renderer* renderer, capture_t* capture) {
  if (renderer == NULL || capture == NULL || capture->pixels == NULL) return NULL;

  SDL_Surface* surface = SDL_CreateSurfaceFrom(capture->width, capture->height, SDL_PIXELFORMAT_RGB24, capture->pixels, capture->stride * 3);
  if (surface == NULL) {
    SDL_Log("Failed to create surface from capture pixels: %s", SDL_GetError());
    return NULL;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    SDL_Log("Failed to create texture from surface: %s", SDL_GetError());
    SDL_DestroySurface(surface);
    return NULL;
  }

  SDL_DestroySurface(surface);
  return texture;
}