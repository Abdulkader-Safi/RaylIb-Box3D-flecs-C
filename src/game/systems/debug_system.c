// Shows the AI its own working: where each enemy is going, and what it can
// see and hear. F1 toggles it.
//
// The route drawn here is not an illustration of the route. It is walked with
// the same function the enemies step along, so if the line goes through a wall
// then so do they.
#include "core/core.h"
#include "game/ai/nav.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/systems/systems.h"

static Color StateColor(AiState state) {
  switch (state) {
  case AI_CHASE: return COLOR_AI_CHASE;
  case AI_INVESTIGATE: return COLOR_AI_INVESTIGATE;
  case AI_PATROL: return COLOR_AI_PATROL;
  case AI_GUARD:
  default: return COLOR_AI_GUARD;
  }
}

// A stub sticking up out of the enemy, so its state is readable even when the
// route underneath it is short.
static void DrawStateMarker(Vec3 at, Color color) {
  Vec3 base = Vec3Make(at.x, 1.3f, at.z);
  Vec3 top = Vec3Make(at.x, 2.1f, at.z);
  GfxDrawCapsule(base, top, 0.10f, color);
}

static void DrawRoute(Vec3 from, NavGoal goal, Color color) {
  Vec3 points[DEBUG_MAX_ROUTE_POINTS];
  int count = NavRoutePoints(from, goal, DEBUG_ROUTE_HEIGHT, points, DEBUG_MAX_ROUTE_POINTS);
  if (count == 0) {
    return;
  }

  Vec3 previous = Vec3Make(from.x, DEBUG_ROUTE_HEIGHT, from.z);
  for (int i = 0; i < count; ++i) {
    GfxDrawLine(previous, points[i], color);
    previous = points[i];
  }
  // A ring on the last tile, so where the route ends is obvious.
  GfxDrawGroundCircle(previous, 0.45f, color);
}

static void AiDebugSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const Brain *brains = ecs_field(it, Brain, 1);
  const Senses *senses = ecs_field(it, Senses, 2);

  const DebugView *view = ecs_singleton_get(it->world, DebugView);
  if (!view->showAi) {
    return;
  }

  const PlayerTracker *tracker = ecs_singleton_get(it->world, PlayerTracker);
  const Alert *alert = ecs_singleton_get(it->world, Alert);

  // Where the room currently thinks the noise came from, and how far that
  // carries. Anything inside this ring is on its way over.
  if (alert->active) {
    Vec3 at = Vec3Make(alert->position.x, DEBUG_RANGE_HEIGHT, alert->position.z);
    GfxDrawGroundCircle(at, 0.7f, COLOR_AI_ALERT);
    GfxDrawGroundCircle(at, 1.1f, COLOR_AI_ALERT);
    GfxDrawLine(at, Vec3Make(at.x, 2.5f, at.z), COLOR_AI_ALERT);
  }

  for (int i = 0; i < it->count; ++i) {
    Vec3 at = positions[i].value;
    Color color = StateColor(brains[i].state);

    DrawStateMarker(at, color);

    float toPlayer = tracker->entity != 0
                         ? Vec3Length(Vec3Flat(Vec3Sub(tracker->position, at)))
                         : 1e9f;

    // What it could notice: the far ring is hearing, which goes through walls,
    // the near one is sight, which does not. Only drawn for enemies near
    // enough to be a threat, or the far side of the level fills up with arcs
    // belonging to something asleep in a corner.
    float furthest = senses[i].hearing > senses[i].sight ? senses[i].hearing : senses[i].sight;
    if (toPlayer <= furthest * 1.25f) {
      Vec3 ground = Vec3Make(at.x, DEBUG_RANGE_HEIGHT, at.z);
      GfxDrawGroundCircle(ground, senses[i].hearing, COLOR_AI_HEARING);
      GfxDrawGroundCircle(ground, senses[i].sight, COLOR_AI_SIGHT);
    }

    // What it actually notices right now. A line only appears when the enemy
    // genuinely has the player: in range, and with nothing in the way.
    if (tracker->entity != 0 && toPlayer <= senses[i].sight &&
        NavLineOfSight(at, tracker->position)) {
      GfxDrawLine(Vec3Make(at.x, 1.0f, at.z),
                  Vec3Make(tracker->position.x, 1.0f, tracker->position.z), COLOR_AI_SEEN);
    }

    switch (brains[i].state) {
    case AI_CHASE: DrawRoute(at, NAV_GOAL_PLAYER, color); break;
    case AI_INVESTIGATE: DrawRoute(at, NAV_GOAL_ALERT, color); break;
    default: break;
    }
  }
}

// The key that turns it on. Separate from the drawing because input is read
// long before anything is drawn.
static void DebugToggleSystem(ecs_iter_t *it) {
  const Input *input = ecs_singleton_get(it->world, Input);
  if (input->toggleDebug) {
    DebugView *view = ecs_singleton_get_mut(it->world, DebugView);
    view->showAi = !view->showAi;
  }
}

void DebugSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, DebugToggleSystem, PhaseTrack, 0);
  ECS_SYSTEM(world, AiDebugSystem, PhaseDraw3D, [in] Position, [in] Brain, [in] Senses, Enemy);
}
