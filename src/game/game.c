#include "game/game.h"

#include "game/components/components.h"
#include "game/spawn/spawn.h"
#include "game/systems/systems.h"

// What the game is, minus how it looks.
static void RegisterSimulation(ecs_world_t *world) {
  // Components first: a system's query is parsed when it is registered, and it
  // resolves component names by looking them up in the world.
  GameComponentsRegister(world);

  // Then the systems. Each one picks its own phase, so this order only decides
  // ties inside a single phase. There is one that matters: restart shares
  // PhaseSpawn with waves, and has to wipe the board before waves refill it.
  RestartSystemRegister(world);
  WaveSystemRegister(world);
  PlayerSystemRegister(world);
  WeaponSystemRegister(world);
  ChaseSystemRegister(world);
  BiteSystemRegister(world);
  CombatSystemRegister(world);
  DeathSystemRegister(world);
}

void GameRegister(ecs_world_t *world) {
  RegisterSimulation(world);

  CameraSystemRegister(world);
  RenderSystemRegister(world);
  HudSystemRegister(world);

  // Finally something to play with.
  SpawnFreshRun(world);
}

void GameRegisterHeadless(ecs_world_t *world) {
  RegisterSimulation(world);
  SpawnFreshRun(world);
}
