// Counts the enemies still standing and, when there are none, walks the next
// wave in from the edge of the arena after a short breather.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/spawn/spawn.h"
#include "game/systems/systems.h"

static void WaveSystem(ecs_iter_t *it) {
  GameState *state = ecs_singleton_get_mut(it->world, GameState);
  if (state->over) {
    return;
  }
  if (ecs_count(it->world, Enemy) > 0) {
    return;
  }

  state->waveBreak -= it->delta_time;
  if (state->waveBreak > 0.0f) {
    return;
  }

  state->wave += 1;
  int count = WAVE_FIRST_COUNT + (state->wave - 1) * WAVE_COUNT_STEP;
  float spawnHeight = ENEMY_HALF_HEIGHT + ENEMY_RADIUS + 0.1f;
  for (int i = 0; i < count; ++i) {
    SpawnEnemy(it->world, SpawnPointOnArenaEdge(spawnHeight));
  }
  state->waveBreak = WAVE_BREAK_SECONDS;
}

void WaveSystemRegister(ecs_world_t *world) {
  // Immediate, so a spawned enemy is complete before the physics step that
  // first moves it.
  SystemRegisterImmediate(world, "WaveSystem", PhaseSpawn, WaveSystem, NULL);
}
