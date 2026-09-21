// Spawners keep a level populated. Each one tops the room up to its own share
// of enemies and then waits, so a level stays dangerous without the count
// running away while the player takes their time.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/spawn/spawn.h"
#include "game/systems/systems.h"

#define SPAWNER_MAX_PENDING 8

static void SpawnerSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  Spawner *spawners = ecs_field(it, Spawner, 1);

  const GameState *state = ecs_singleton_get(it->world, GameState);
  if (state->mode != MODE_PLAYING) {
    return;
  }

  int alive = ecs_count(it->world, Enemy);

  // Collected first, spawned after the loop. This system is immediate, so a
  // spawn lands at once, and doing that mid-iteration would move the ground
  // under the fields read above.
  Vec3 pending[SPAWNER_MAX_PENDING];
  EnemyTier tiers[SPAWNER_MAX_PENDING];
  int pendingCount = 0;

  for (int i = 0; i < it->count; ++i) {
    if (spawners[i].cooldown > 0.0f) {
      spawners[i].cooldown -= it->delta_time;
      continue;
    }
    if (alive >= spawners[i].maxAlive * it->count || pendingCount == SPAWNER_MAX_PENDING) {
      continue;
    }

    Vec3 at = positions[i].value;
    at.y = ENEMY_MEDIUM_RADIUS + 0.6f;
    pending[pendingCount] = at;
    tiers[pendingCount] = spawners[i].tier;
    pendingCount += 1;
    alive += 1;
    spawners[i].cooldown = spawners[i].interval;
  }

  for (int i = 0; i < pendingCount; ++i) {
    SpawnEnemy(it->world, pending[i], tiers[i]);
  }
}

void SpawnerSystemRegister(ecs_world_t *world) {
  SystemRegisterImmediate(world, "SpawnerSystem", PhaseSpawn, SpawnerSystem,
                          "[in] Position, Spawner");
}
