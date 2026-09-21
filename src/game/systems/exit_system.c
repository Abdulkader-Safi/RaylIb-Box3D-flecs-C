// Standing on the exit finishes the level.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/systems/systems.h"

#define EXIT_REACH 1.2f

static void ExitSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);

  GameState *state = ecs_singleton_get_mut(it->world, GameState);
  const PlayerTracker *tracker = ecs_singleton_get(it->world, PlayerTracker);
  if (state->mode != MODE_PLAYING || tracker->entity == 0) {
    return;
  }

  for (int i = 0; i < it->count; ++i) {
    if (Vec3Length(Vec3Flat(Vec3Sub(tracker->position, positions[i].value))) > EXIT_REACH) {
      continue;
    }
    state->score += LEVEL_COMPLETE_BONUS;
    state->mode = MODE_LEVEL_CLEARED;
    return;
  }
}

void ExitSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, ExitSystem, PhaseLogic, [in] Position, Exit);
}
