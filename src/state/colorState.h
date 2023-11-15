#ifndef _COLOR_STATE_H
#define _COLOR_STATE_H

#include "cappyMachine.h"

DEFINE_STATE(ColorState, CappyMachine) {
  DEFINE_STATE_INNER(ColorState, CappyMachine);

public:
private:
  float panel_width  = 275.0f;
  float panel_offset = 50.0f;

  std::shared_ptr<SDL_Surface> text_surface;
  std::shared_ptr<SDL_Texture> text_texture;
  bool recompute_text = true;
};

#endif