// A locked door is a wall until the player is carrying a keycard, and then it
// is not there any more.
#include "core/core.h"
#include "game/components/components.h"
#include "game/systems/systems.h"

static void DoorSystem(ecs_iter_t *it) {
  const GameState *state = ecs_singleton_get(it->world, GameState);
  if (!state->hasKeycard) {
    return;
  }

  for (int i = 0; i < it->count; ++i) {
    // Deleting takes the rigid body with it, so the doorway opens for the
    // crowd as well as for the player.
    ecs_delete(it->world, it->entities[i]);
  }
}

void DoorSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, DoorSystem, PhaseCleanup, [in] Door);
}
