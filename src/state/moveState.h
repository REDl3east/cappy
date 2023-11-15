#ifndef _MOVE_STATE_H
#define _MOVE_STATE_H

#include "cappyMachine.h"

DEFINE_STATE(MoveState, CappyMachine) {
  DEFINE_STATE_INNER(MoveState, CappyMachine);

public:
  MoveState();
};

#endif