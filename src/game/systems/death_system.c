// What happens when something runs out of health. An enemy pays out in score
// and coins; the player ends the level.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/spawn/spawn.h"
#include "game/systems/systems.h"

#define MAX_DROPS_PER_FRAME 32

static void EnemyDeathSystem(ecs_iter_t *it) {
  const Health *health = ecs_field(it, Health, 0);
  const Position *positions = ecs_field(it, Position, 1);
  const Loot *loot = ecs_field(it, Loot, 2);

  GameState *state = ecs_singleton_get_mut(it->world, GameState);

  // Where to scatter coins, gathered while iterating and spawned after, for
  // the same reason the spawner does it: this system changes the world on the
  // spot and must not do that while reading its own fields.
  Vec3 drops[MAX_DROPS_PER_FRAME];
  int dropCount = 0;

  for (int i = 0; i < it->count; ++i) {
    if (health[i].current > 0) {
      continue;
    }

    state->score += loot[i].score;
    for (int coin = 0; coin < loot[i].coins && dropCount < MAX_DROPS_PER_FRAME; ++coin) {
      Vec3 at = positions[i].value;
      at.x += RandomFloat(-COIN_SCATTER, COIN_SCATTER);
      at.z += RandomFloat(-COIN_SCATTER, COIN_SCATTER);
      at.y = 0.6f;
      drops[dropCount++] = at;
    }

    // flecs holds the delete until the end of the frame, and the rigid body
    // goes with the entity when it lands.
    ecs_delete(it->world, it->entities[i]);
  }

  for (int i = 0; i < dropCount; ++i) {
    SpawnPickup(it->world, drops[i], PICKUP_COIN, 1);
  }
}

static void PlayerDeathSystem(ecs_iter_t *it) {
  const Health *health = ecs_field(it, Health, 0);
  GameState *state = ecs_singleton_get_mut(it->world, GameState);

  for (int i = 0; i < it->count; ++i) {
    if (health[i].current <= 0 && state->mode == MODE_PLAYING) {
      // The player entity survives so the camera still has something to look
      // at and the screen can show where the run ended.
      state->over = true;
      state->mode = MODE_GAME_OVER;
    }
  }
}

void DeathSystemRegister(ecs_world_t *world) {
  // Immediate, because a death drops coins, and a queued spawn would not exist
  // until after the entity that paid for it was already gone.
  SystemRegisterImmediate(world, "EnemyDeathSystem", PhaseCleanup, EnemyDeathSystem,
                          "[in] Health, [in] Position, [in] Loot, Enemy");
  ECS_SYSTEM(world, PlayerDeathSystem, PhaseCleanup, [in] Health, Player);
}
