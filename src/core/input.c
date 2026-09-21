#include "core/input.h"

#include "raymath.h"

#define GAMEPAD 0
#define STICK_DEADZONE 0.2f

// Drops stick noise around centre and keeps the remaining range full length.
static Vector2 ApplyDeadzone(Vector2 stick) {
  float length = Vector2Length(stick);
  if (length < STICK_DEADZONE) {
    return (Vector2){0.0f, 0.0f};
  }
  float scaled = (length - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
  return Vector2Scale(Vector2Scale(stick, 1.0f / length), fminf(scaled, 1.0f));
}

static Vector2 ReadMove(void) {
  Vector2 move = {0.0f, 0.0f};
  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1.0f;
  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1.0f;
  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.y += 1.0f;
  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.y -= 1.0f;

  if (move.x != 0.0f || move.y != 0.0f) {
    return Vector2Normalize(move);
  }

  if (IsGamepadAvailable(GAMEPAD)) {
    Vector2 stick = {GetGamepadAxisMovement(GAMEPAD, GAMEPAD_AXIS_LEFT_X),
                     -GetGamepadAxisMovement(GAMEPAD, GAMEPAD_AXIS_LEFT_Y)};
    return ApplyDeadzone(stick);
  }
  return move;
}

// Where the mouse ray crosses the horizontal plane the player stands on. That
// plane, not the ground, keeps the crosshair level with the muzzle.
static Vector3 MouseAimPoint(Camera3D camera, Vector3 playerPosition) {
  Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
  if (fabsf(ray.direction.y) < 1e-4f) {
    return Vector3Add(playerPosition, (Vector3){0.0f, 0.0f, 1.0f});
  }
  float distance = (playerPosition.y - ray.position.y) / ray.direction.y;
  if (distance < 0.0f) {
    return Vector3Add(playerPosition, (Vector3){0.0f, 0.0f, 1.0f});
  }
  return Vector3Add(ray.position, Vector3Scale(ray.direction, distance));
}

InputState InputRead(Camera3D camera, Vector3 playerPosition) {
  InputState input = {0};
  input.move = ReadMove();
  input.restart = IsKeyPressed(KEY_R);

  bool usingGamepad = false;
  if (IsGamepadAvailable(GAMEPAD)) {
    Vector2 aimStick = ApplyDeadzone((Vector2){GetGamepadAxisMovement(GAMEPAD, GAMEPAD_AXIS_RIGHT_X),
                                               -GetGamepadAxisMovement(GAMEPAD, GAMEPAD_AXIS_RIGHT_Y)});
    if (aimStick.x != 0.0f || aimStick.y != 0.0f) {
      usingGamepad = true;
      input.aimPoint = Vector3Add(playerPosition, (Vector3){aimStick.x * 10.0f, 0.0f, -aimStick.y * 10.0f});
      input.firing = true; // Pushing the right stick is the shoot gesture.
    }
    input.restart = input.restart || IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_MIDDLE_RIGHT);
  }

  if (!usingGamepad) {
    input.aimPoint = MouseAimPoint(camera, playerPosition);
    input.firing = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE);
  }
  return input;
}
