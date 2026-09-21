// The player: publish where it is, then drive it from this frame's input.
#include "core/core.h"
#include "game/components/components.h"
#include "game/systems/systems.h"

// Runs in PhaseTrack, before anything that wants to know about the player.
// Writing it to a singleton once is what lets chasing, biting, the camera and
// the HUD stay ignorant of how players are stored.
static void PlayerTrackSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const Health *health = ecs_field(it, Health, 1);

  PlayerTracker *tracker = ecs_singleton_get_mut(it->world, PlayerTracker);
  if (it->count == 0) {
    tracker->entity = 0;
    return;
  }

  tracker->entity = it->entities[0];
  tracker->position = positions[0].value;
  tracker->health = health[0].current;
  tracker->maxHealth = health[0].max;
}

static void PlayerControlSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const PhysicsBody *bodies = ecs_field(it, PhysicsBody, 1);
  const MoveSpeed *speeds = ecs_field(it, MoveSpeed, 2);
  Aim *aims = ecs_field(it, Aim, 3);

  const Input *input = ecs_singleton_get(it->world, Input);
  const GameState *state = ecs_singleton_get(it->world, GameState);

  for (int i = 0; i < it->count; ++i) {
    // Screen up is negative Z, so pushing forward moves that way.
    Vec3 velocity = VEC3_ZERO;
    if (!state->over) {
      velocity = Vec3Make(input->move.x * speeds[i].value, 0.0f, -input->move.y * speeds[i].value);
    }
    PhysicsDriveHorizontal(bodies[i], velocity);

    Vec3 toAim = Vec3Flat(Vec3Sub(InputAimPoint(it->world, positions[i].value), positions[i].value));
    // Aim only changes when there is somewhere to point. Resting the crosshair
    // on the player would otherwise spin it at random.
    if (Vec3LengthSquared(toAim) > 1e-6f) {
      aims[i].direction = Vec3Normalize(toAim);
    }
  }
}

void PlayerSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, PlayerTrackSystem, PhaseTrack, [in] Position, [in] Health, Player);
  ECS_SYSTEM(world, PlayerControlSystem, PhaseLogic, [in] Position, [in] PhysicsBody,
             [in] MoveSpeed, Aim, Player);
}
