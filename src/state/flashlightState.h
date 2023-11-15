#ifndef _FLASHLIGHT_STATE_H
#define _FLASHLIGHT_STATE_H


#include "cappyMachine.h"

DEFINE_STATE(FlashlightState, CappyMachine) {
  DEFINE_STATE_INNER(FlashlightState, CappyMachine);

public:
  FlashlightState();
  ~FlashlightState();

private:
  void zoom(float in);
  bool update();

  float size             = 300.0f;
  float zooming          = false;
  float zoom_in          = false;
  float zoom_amount      = 150.0f;
  float zoom_ms          = 25.0f;
  float zoom_tick        = 0.0f;
  float zoom_elapsed     = 0.0f;
  float zoom_size_per_ms = 0.0f;
};

#endif