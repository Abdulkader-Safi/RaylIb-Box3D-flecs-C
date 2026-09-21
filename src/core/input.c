#include "core/input.h"

#include "core/gfx.h"
#include "core/phases.h"
#include "raylib.h"

#define PAD 0
#define STICK_DEADZONE 0.2f
// Aiming with the right stick also pulls the trigger, so it takes a deliberate
// push to count. A worn stick that rests a little off centre clears the
// deadzone on its own, and at that point the game fires forever and ignores
// the mouse.
#define STICK_AIM_THRESHOLD 0.5f

// Drops stick noise around centre and stretches what is left back to full
// range, so a barely pushed stick reads as zero and a fully pushed one as one.
static Vec2 ApplyDeadzone(Vec2 stick) {
  float length = Vec2Length(stick);
  if (length < STICK_DEADZONE) {
    return VEC2_ZERO;
  }
  float scaled = (length - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
  return Vec2Scale(Vec2Scale(stick, 1.0f / length), MathClamp(scaled, 0.0f, 1.0f));
}

static Vec2 ReadMove(void) {
  Vec2 move = VEC2_ZERO;
  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1.0f;
  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1.0f;
  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.y += 1.0f;
  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.y -= 1.0f;

  if (move.x != 0.0f || move.y != 0.0f) {
    return Vec2Normalize(move);
  }
  if (IsGamepadAvailable(PAD)) {
    return ApplyDeadzone(Vec2Make(GetGamepadAxisMovement(PAD, GAMEPAD_AXIS_LEFT_X),
                                  -GetGamepadAxisMovement(PAD, GAMEPAD_AXIS_LEFT_Y)));
  }
  return move;
}

static void InputPollSystem(ecs_iter_t *it) {
  Input *input = ecs_singleton_get_mut(it->world, Input);

  input->move = ReadMove();
  input->restart = IsKeyPressed(KEY_R);

  input->pause = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P);
  input->confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_KP_ENTER);
  input->menuUp = IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
  input->menuDown = IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN);
  input->menuLeft = IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT);
  input->menuRight = IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT);
  input->quit = WindowShouldClose();
  input->aimIsStick = false;
  input->aimStick = VEC2_ZERO;

  if (IsGamepadAvailable(PAD)) {
    Vec2 stick = ApplyDeadzone(Vec2Make(GetGamepadAxisMovement(PAD, GAMEPAD_AXIS_RIGHT_X),
                                        -GetGamepadAxisMovement(PAD, GAMEPAD_AXIS_RIGHT_Y)));
    if (Vec2Length(stick) >= STICK_AIM_THRESHOLD) {
      input->aimIsStick = true;
      input->aimStick = stick;
    }
    input->pause = input->pause || IsGamepadButtonPressed(PAD, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    input->confirm = input->confirm || IsGamepadButtonPressed(PAD, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    input->menuUp = input->menuUp || IsGamepadButtonPressed(PAD, GAMEPAD_BUTTON_LEFT_FACE_UP);
    input->menuDown = input->menuDown || IsGamepadButtonPressed(PAD, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    input->menuLeft = input->menuLeft || IsGamepadButtonPressed(PAD, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    input->menuRight = input->menuRight || IsGamepadButtonPressed(PAD, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
  }

  input->aimScreen = GetMousePosition();
  // Pushing the right stick is the shoot gesture on a pad, the way twin stick
  // games have always done it.
  input->fire = input->aimIsStick || IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE);
}

void InputRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, InputPollSystem, PhaseInput, 0);
}

Vec3 InputAimPoint(ecs_world_t *world, Vec3 origin) {
  const Input *input = ecs_singleton_get(world, Input);
  if (input->aimIsStick) {
    // A stick gives a direction, so the point is placed a fixed distance out
    // along it. Anything past the player reads the same once normalised.
    return Vec3Add(origin, Vec3Make(input->aimStick.x * 10.0f, 0.0f, -input->aimStick.y * 10.0f));
  }
  return GfxScreenPointOnPlane(world, input->aimScreen, origin.y);
}
