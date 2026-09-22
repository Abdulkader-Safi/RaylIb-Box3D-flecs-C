// What the enemies know, and what they do about it.
//
// Nothing here reads the player's position and walks at it. An enemy only acts
// on what it can see or has heard, and when it does move it follows the routes
// in nav.c, so it goes through the doorway rather than into the wall beside it.
//
// The four states are a loop, not a hierarchy:
//
//   guard or patrol  ->  sees or hears something  ->  chase
//   chase            ->  loses sight              ->  investigate
//   investigate      ->  finds nothing in time    ->  guard or patrol
#include "core/core.h"
#include "game/ai/nav.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/systems/systems.h"

// The routes every enemy shares, rebuilt on a clock rather than per enemy.
static void RebuildRoutes(ecs_world_t *world) {
  const PlayerTracker *tracker = ecs_singleton_get(world, PlayerTracker);
  const GameState *state = ecs_singleton_get(world, GameState);
  const Alert *alert = ecs_singleton_get(world, Alert);
  if (tracker->entity == 0) {
    return;
  }
  NavRebuild(state->hasKeycard, tracker->position, alert->active, alert->position);
}

// The one place an enemy is allowed to learn anything about the player.
static bool CanSee(const Senses *senses, Vec3 from, Vec3 playerPosition) {
  if (Vec3Length(Vec3Flat(Vec3Sub(playerPosition, from))) > senses->sight) {
    return false;
  }
  return NavLineOfSight(from, playerPosition);
}

static bool CanHear(const Senses *senses, Vec3 from, const Alert *alert) {
  if (!alert->active) {
    return false;
  }
  return Vec3Length(Vec3Flat(Vec3Sub(alert->position, from))) <= senses->hearing;
}

// Somewhere reachable to wander to, picked by walking the route away from
// nothing in particular. Failing that, it stays put.
static Vec3 PickWanderTarget(Vec3 from) {
  float angle = RandomAngle();
  float reach = RandomFloat(4.0f, 10.0f);
  return Vec3Add(from, Vec3Make(cosf(angle) * reach, 0.0f, sinf(angle) * reach));
}

static void Drive(PhysicsBody body, Vec3 direction, float speed) {
  PhysicsDriveHorizontal(body, Vec3Scale(direction, speed));
}

// Follows the shared route to the goal. Close in, it steers straight at the
// target instead: the grid is two metres wide and walking its centres looks
// stilted when you are already in the same room.
static bool MoveAlong(PhysicsBody body, Vec3 from, Vec3 target, NavGoal goal, float speed,
                      bool targetVisible) {
  Vec3 straight = Vec3Flat(Vec3Sub(target, from));
  float distance = Vec3Length(straight);

  if (targetVisible && distance < AI_DIRECT_RANGE && distance > 1e-4f) {
    Drive(body, Vec3Scale(straight, 1.0f / distance), speed);
    return true;
  }

  Vec3 direction;
  if (NavDirectionToward(from, goal, &direction)) {
    Drive(body, direction, speed);
    return true;
  }

  // No route at all. Standing still beats grinding into a wall.
  Drive(body, VEC3_ZERO, 0.0f);
  return false;
}

static void AiSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const PhysicsBody *bodies = ecs_field(it, PhysicsBody, 1);
  const MoveSpeed *speeds = ecs_field(it, MoveSpeed, 2);
  const Senses *senses = ecs_field(it, Senses, 3);
  Brain *brains = ecs_field(it, Brain, 4);

  const GameState *state = ecs_singleton_get(it->world, GameState);
  const PlayerTracker *tracker = ecs_singleton_get(it->world, PlayerTracker);
  Alert *alert = ecs_singleton_get_mut(it->world, Alert);

  // Outside of play the crowd stops where it stands, so a pause or a game over
  // screen is something you read rather than something still moving.
  if (state->mode != MODE_PLAYING) {
    for (int i = 0; i < it->count; ++i) {
      Drive(bodies[i], VEC3_ZERO, 0.0f);
    }
    return;
  }

  if (alert->active) {
    alert->timer -= it->delta_time;
    if (alert->timer <= 0.0f) {
      alert->active = false;
    }
  }

  if (tracker->entity == 0) {
    return;
  }

  for (int i = 0; i < it->count; ++i) {
    Vec3 at = positions[i].value;
    Brain *brain = brains + i;
    bool visible = CanSee(senses + i, at, tracker->position);

    if (visible) {
      brain->state = AI_CHASE;
      brain->alertTimer = senses[i].memory;
      // Seeing the player is worth telling the room about. This is how one
      // enemy spotting you pulls the others in without any of them cheating.
      alert->position = tracker->position;
      alert->timer = ALERT_SECONDS;
      alert->active = true;
    } else if (CanHear(senses + i, at, alert)) {
      if (brain->state != AI_INVESTIGATE && brain->state != AI_CHASE) {
        brain->state = AI_INVESTIGATE;
      }
      brain->alertTimer = senses[i].memory;
    } else if (brain->alertTimer > 0.0f) {
      // Lost sight but not interest. Go to where it was last worth going.
      brain->state = AI_INVESTIGATE;
      brain->alertTimer -= it->delta_time;
    } else {
      brain->state = brain->wandering ? AI_PATROL : AI_GUARD;
    }

    switch (brain->state) {
    case AI_CHASE:
      MoveAlong(bodies[i], at, tracker->position, NAV_GOAL_PLAYER, speeds[i].value, true);
      break;

    case AI_INVESTIGATE:
      if (!alert->active ||
          !MoveAlong(bodies[i], at, alert->position, NAV_GOAL_ALERT, speeds[i].value, false)) {
        brain->alertTimer = 0.0f;
        Drive(bodies[i], VEC3_ZERO, 0.0f);
      }
      break;

    case AI_PATROL: {
      float speed = speeds[i].value * AI_PATROL_SPEED_SCALE;
      Vec3 toTarget = Vec3Flat(Vec3Sub(brain->wanderTarget, at));
      if (Vec3Length(toTarget) < AI_PATROL_REACH) {
        brain->wanderTarget = PickWanderTarget(at);
        Drive(bodies[i], VEC3_ZERO, 0.0f);
        break;
      }
      // Wandering has no shared route, so it walks at its target and gives up
      // on it when a wall gets in the way.
      Drive(bodies[i], Vec3Normalize(toTarget), speed);
      brain->repathTimer -= it->delta_time;
      if (brain->repathTimer <= 0.0f) {
        brain->repathTimer = 1.0f;
        if (!NavLineOfSight(at, brain->wanderTarget)) {
          brain->wanderTarget = PickWanderTarget(at);
        }
      }
      break;
    }

    case AI_GUARD:
    default:
      Drive(bodies[i], VEC3_ZERO, 0.0f);
      break;
    }
  }
}

// Rebuilding the routes is one job for the whole level, so it is its own
// system rather than something the first enemy happens to do.
static void NavSystem(ecs_iter_t *it) {
  GameState *state = ecs_singleton_get_mut(it->world, GameState);
  if (state->mode != MODE_PLAYING) {
    return;
  }

  state->navTimer -= it->delta_time;
  if (state->navTimer > 0.0f) {
    return;
  }
  state->navTimer = AI_REPATH_INTERVAL;
  RebuildRoutes(it->world);
}

void AiSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, NavSystem, PhaseTrack, 0);
  ECS_SYSTEM(world, AiSystem, PhaseLogic, [in] Position, [in] PhysicsBody, [in] MoveSpeed,
             [in] Senses, Brain, Enemy);
}
