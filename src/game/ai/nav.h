// Getting around the level without walking into it.
//
// Every enemy wants the same thing, a route to the player, so the route is
// worked out once for the whole grid rather than once per enemy. One breadth
// first sweep fills in how many tiles each square is from the player, and an
// enemy then only has to look at its neighbours and step to the lowest one.
// Twenty enemies cost the same as one.
//
// A second sweep does the same from the last place anything was heard, which
// is where enemies go when they have lost the player but have not forgotten.
#ifndef GAME_NAV_H
#define GAME_NAV_H

#include "core/core.h"
#include "game/levels/levels.h"

typedef enum NavGoal {
  NAV_GOAL_PLAYER,
  NAV_GOAL_ALERT,
} NavGoal;

// Called when a level is built. Clears both fields.
void NavSetLevel(const Level *level);

// Recomputes what is walkable and both distance fields. Cheap enough to call
// several times a second: a full grid is a few thousand tiles.
void NavRebuild(bool doorsOpen, Vec3 playerPosition, bool alertActive, Vec3 alertPosition);

// Unit direction for the next step toward the goal, false when there is no
// route at all. The step aims at the centre of the next tile rather than
// straight at the goal, which is what keeps bodies off the corners.
bool NavDirectionToward(Vec3 from, NavGoal goal, Vec3 *outDirection);

// Whether one point can see another. Walls and shut doors block it; low cover
// does not, because it is low.
bool NavLineOfSight(Vec3 from, Vec3 to);

// Walks the route out into world points, for drawing it. Returns how many
// were written. The first point is the next tile, not where you are standing.
int NavRoutePoints(Vec3 from, NavGoal goal, float height, Vec3 *points, int maxPoints);

// How far the goal is by the route rather than through the walls, in metres.
// Negative when it cannot be reached.
float NavRouteDistance(Vec3 from, NavGoal goal);

#endif // GAME_NAV_H
