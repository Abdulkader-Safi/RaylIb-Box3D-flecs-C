// Every enemy walks straight at the player. There is no path finding and none
// is needed: the solver does the hard part, shoving the crowd apart so they
// squeeze around each other instead of stacking into one column.
#include "core/core.h"
#include "game/components/components.h"
#include "game/systems/systems.h"

static void ChaseSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const PhysicsBody *bodies = ecs_field(it, PhysicsBody, 1);
  const MoveSpeed *speeds = ecs_field(it, MoveSpeed, 2);

  const PlayerTracker *tracker = ecs_singleton_get(it->world, PlayerTracker);
  const GameState *state = ecs_singleton_get(it->world, GameState);
  if (tracker->entity == 0) {
    return;
  }

  // Outside of play the crowd stops where it stands, so a pause or a game over
  // screen is something you read rather than something still moving.
  if (state->mode != MODE_PLAYING) {
    for (int i = 0; i < it->count; ++i) {
      PhysicsDriveHorizontal(bodies[i], VEC3_ZERO);
    }
    return;
  }

  for (int i = 0; i < it->count; ++i) {
    Vec3 toPlayer = Vec3Flat(Vec3Sub(tracker->position, positions[i].value));
    float distance = Vec3Length(toPlayer);
    if (distance > 1e-4f) {
      PhysicsDriveHorizontal(bodies[i], Vec3Scale(toPlayer, speeds[i].value / distance));
    }
  }
}

void ChaseSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, ChaseSystem, PhaseLogic, [in] Position, [in] PhysicsBody, [in] MoveSpeed,
             Enemy);
}
