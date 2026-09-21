// What happens when something runs out of health. An enemy pays out and is
// removed; the player ends the run but stays on screen.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/systems/systems.h"

static void EnemyDeathSystem(ecs_iter_t *it) {
  const Health *health = ecs_field(it, Health, 0);
  GameState *state = ecs_singleton_get_mut(it->world, GameState);

  for (int i = 0; i < it->count; ++i) {
    if (health[i].current > 0) {
      continue;
    }
    state->score += SCORE_PER_KILL;
    // flecs holds the delete until the end of the frame, and the rigid body
    // goes with the entity when it lands.
    ecs_delete(it->world, it->entities[i]);
  }
}

static void PlayerDeathSystem(ecs_iter_t *it) {
  const Health *health = ecs_field(it, Health, 0);
  GameState *state = ecs_singleton_get_mut(it->world, GameState);

  for (int i = 0; i < it->count; ++i) {
    if (health[i].current <= 0) {
      // The player entity survives so the camera still has something to look
      // at and the game over panel can read the final numbers.
      state->over = true;
    }
  }
}

void DeathSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, EnemyDeathSystem, PhaseCleanup, [in] Health, Enemy);
  ECS_SYSTEM(world, PlayerDeathSystem, PhaseCleanup, [in] Health, Player);
}
