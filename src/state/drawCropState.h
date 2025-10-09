#ifndef _DRAW_CROP_STATE_H
#define _DRAW_CROP_STATE_H

#include "cappyMachine.h"

enum class ResizeSelection {
  NONE,
  CENTER,
  N,
  E,
  S,
  W,
  NE,
  SE,
  SW,
  NW,
};

DEFINE_STATE(DrawCropState, CappyMachine) {
  DEFINE_STATE_INNER(DrawCropState, CappyMachine);

public:
  DrawCropState(float x, float y);

private:
  SDL_FPoint start_crop;
  SDL_FPoint end_crop;
  bool drawing                            = true;
  static constexpr float resize_rect_size = 15.0f;
  ResizeSelection resize_selection        = ResizeSelection::NONE;
  std::shared_ptr<SDL_Cursor> crosshair_cursor;
  std::shared_ptr<SDL_Cursor> ns_cursor;
  std::shared_ptr<SDL_Cursor> ew_cursor;
  std::shared_ptr<SDL_Cursor> nwse_cursor;
  std::shared_ptr<SDL_Cursor> nesw_cursor;
  std::shared_ptr<SDL_Cursor> move_cursor;

};

#endif