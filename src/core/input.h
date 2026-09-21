// Keyboard, mouse and gamepad folded into the Input singleton once per frame.
// Game systems read Input and never ask raylib what a key is doing.
#ifndef CORE_INPUT_H
#define CORE_INPUT_H

#include "core/components.h"
#include "flecs.h"

void InputRegister(ecs_world_t *world);

// Where the player is aiming, as a world point on the plane through `origin`.
// Works the same whether the aim came from a mouse or a stick, which is the
// only reason a game system can ignore the difference.
Vec3 InputAimPoint(ecs_world_t *world, Vec3 origin);

#endif // CORE_INPUT_H
