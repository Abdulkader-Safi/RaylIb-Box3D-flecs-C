// Enemies hurt the player by standing next to it.
//
// This is distance and a cooldown rather than a collision event on purpose. A
// contact fires once, when two bodies start touching, and an enemy leaning on
// the player never stops touching, so it would only ever land a single hit.
#include "core/core.h"
#include "game/components/components.h"
#include "game/systems/systems.h"

static void BiteSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  Bite *bites = ecs_field(it, Bite, 1);

  const GameState *state = ecs_singleton_get(it->world, GameState);
  const PlayerTracker *tracker = ecs_singleton_get(it->world, PlayerTracker);

  Health *playerHealth = NULL;
  if (!state->over && tracker->entity != 0) {
    playerHealth = ecs_get_mut(it->world, tracker->entity, Health);
  }

  for (int i = 0; i < it->count; ++i) {
    if (bites[i].cooldown > 0.0f) {
      bites[i].cooldown -= it->delta_time;
      continue;
    }
    if (playerHealth == NULL || playerHealth->current <= 0) {
      continue;
    }
    if (Vec3Length(Vec3Flat(Vec3Sub(tracker->position, positions[i].value))) > bites[i].range) {
      continue;
    }

    playerHealth->current -= bites[i].damage;
    if (playerHealth->current < 0) {
      playerHealth->current = 0;
    }
    bites[i].cooldown = bites[i].interval;
  }
}

void BiteSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, BiteSystem, PhaseLogic, [in] Position, Bite, Enemy);
}
