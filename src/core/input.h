// Turns keyboard, mouse and gamepad into one frame of intent. Nothing else in
// the game asks raylib what a key is doing.
#ifndef INPUT_H
#define INPUT_H

#include "raylib.h"

#include <stdbool.h>

typedef struct InputState {
  Vector2 move;     // Left stick: x is right, y is forward. Length <= 1.
  Vector3 aimPoint; // Right stick or mouse, resolved onto the player's plane.
  bool firing;
  bool restart;
} InputState;

InputState InputRead(Camera3D camera, Vector3 playerPosition);

#endif // INPUT_H
