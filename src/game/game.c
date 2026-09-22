#include "game/game.h"

#include "game/components/components.h"
#include "game/levels/levels.h"
#include "game/spawn/spawn.h"
#include "game/systems/systems.h"

// What the game is, minus how it looks.
static void RegisterSimulation(ecs_world_t *world) {
  // Components first: a system's query is parsed when it is registered, and it
  // resolves component names by looking them up in the world.
  GameComponentsRegister(world);

  // Then the systems. Each one picks its own phase, so this order only decides
  // ties inside a single phase. One of those matters: the mode system shares
  // PhaseSpawn with the spawners, and may rebuild the whole level, so it goes
  // first and the spawners see the board they are actually topping up.
  ModeSystemRegister(world);
  SpawnerSystemRegister(world);
  PlayerSystemRegister(world);
  WeaponSystemRegister(world);
  AiSystemRegister(world);
  BiteSystemRegister(world);
  PickupSystemRegister(world);
  ExitSystemRegister(world);
  DoorSystemRegister(world);
  CombatSystemRegister(world);
  DeathSystemRegister(world);
}

void GameRegister(ecs_world_t *world) {
  RegisterSimulation(world);

  CameraSystemRegister(world);
  RenderSystemRegister(world);
  HudSystemRegister(world);

  // The title screen is the starting state, with the first level standing
  // behind it so there is something to look at.
  LevelLoad(world, 0);
}

void GameRegisterHeadless(ecs_world_t *world) {
  RegisterSimulation(world);
  LevelLoad(world, 0);
}
