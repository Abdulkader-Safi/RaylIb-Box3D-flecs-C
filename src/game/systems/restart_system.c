// R, the pad's start button, or the button on the game over panel all arrive
// here as one flag, and all mean the same thing: wipe the board and start over.
#include "core/core.h"
#include "game/components/components.h"
#include "game/spawn/spawn.h"
#include "game/systems/systems.h"

static void RestartSystem(ecs_iter_t *it) {
  const Input *input = ecs_singleton_get(it->world, Input);
  if (input->restart) {
    SpawnFreshRun(it->world);
  }
}

void RestartSystemRegister(ecs_world_t *world) {
  // Immediate, because deleting the old run has to happen now: a queued
  // delete_with would fire after the replacement arena and player exist and
  // take those down as well.
  //
  // First in PhaseSpawn, so the wave system that follows sees the fresh world
  // rather than the one being thrown away.
  SystemRegisterImmediate(world, "RestartSystem", PhaseSpawn, RestartSystem, NULL);
}
