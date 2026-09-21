// Thin wrapper over the Box3D world: fixed stepping, body factories for the
// three shapes the game needs, and collision filtering in one place.
#ifndef PHYSICS_H
#define PHYSICS_H

#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/math_functions.h"
#include "raylib.h"

#include <stdint.h>

// One bit per kind of thing. A shape's category says what it is, its mask says
// what it is willing to touch.
enum CollisionCategory {
  CATEGORY_ARENA = 1 << 0,
  CATEGORY_PLAYER = 1 << 1,
  CATEGORY_ENEMY = 1 << 2,
  CATEGORY_BULLET = 1 << 3,
};

typedef struct Physics {
  b3WorldId worldId;
  float accumulator;
} Physics;

// Called once per contact that started during a step, with the user data of
// both bodies. Either pointer may be NULL.
typedef void (*ContactCallback)(void *ownerA, void *ownerB, void *context);

void PhysicsInit(Physics *physics);
void PhysicsShutdown(Physics *physics);

// Advances the world in fixed slices and reports the contacts that began in
// each one. Do not destroy bodies from inside the callback, mark them instead.
void PhysicsStep(Physics *physics, float dt, ContactCallback onContact, void *context);

b3BodyId PhysicsCreateStaticBox(Physics *physics, Vector3 center, Vector3 halfExtents,
                                uint64_t category, uint64_t mask, void *userData);

// An upright capsule with rotation locked, which is what a walking character
// wants: it leans on nothing and never tips over.
b3BodyId PhysicsCreateCharacter(Physics *physics, Vector3 center, float radius, float halfHeight,
                                uint64_t category, uint64_t mask, void *userData);

// A weightless sphere launched at a fixed velocity, flagged for continuous
// collision so it cannot tunnel through a wall at high speed.
b3BodyId PhysicsCreateProjectile(Physics *physics, Vector3 center, Vector3 velocity, float radius,
                                 uint64_t category, uint64_t mask, void *userData);

Vector3 PhysicsBodyPosition(b3BodyId bodyId);

// Replaces horizontal velocity while leaving the vertical component to gravity.
void PhysicsDriveHorizontal(b3BodyId bodyId, Vector3 velocity);

static inline Vector3 PhysicsFromB3(b3Vec3 v) { return (Vector3){v.x, v.y, v.z}; }
static inline b3Vec3 PhysicsToB3(Vector3 v) { return (b3Vec3){v.x, v.y, v.z}; }

#endif // PHYSICS_H
