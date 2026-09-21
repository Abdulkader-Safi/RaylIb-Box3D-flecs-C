// The rigid body world, wrapped so game code never sees Box3D.
//
// Bodies are created from a description, addressed by an opaque PhysicsBody
// handle, and destroyed automatically when their entity is deleted. Collisions
// arrive as entity pairs in the Contacts singleton, not as engine events.
#ifndef CORE_PHYSICS_H
#define CORE_PHYSICS_H

#include "core/components.h"
#include "core/math.h"
#include "flecs.h"

#include <stdint.h>

typedef enum BodyKind {
  // Never moves. Floors, walls, anything scenery.
  BODY_STATIC_BOX,
  // An upright capsule with rotation locked, for anything that walks. It gets
  // shoved by the crowd without tipping over or spinning.
  BODY_CHARACTER,
  // A weightless sphere with continuous collision on, so a fast shot cannot
  // pass through a wall between two steps.
  BODY_PROJECTILE,
} BodyKind;

typedef struct BodyDesc {
  BodyKind kind;
  Vec3 position;
  Vec3 velocity;     // Projectiles only.
  Vec3 halfExtents;  // Static boxes only.
  float radius;      // Characters and projectiles.
  float halfHeight;  // Characters: half the straight part of the capsule.
  float friction;
  uint64_t category; // What this body is, as a single bit.
  uint64_t mask;     // What it is willing to collide with.
} BodyDesc;

// Sets up the world and registers the stepping and syncing systems.
void PhysicsRegister(ecs_world_t *world);
void PhysicsShutdown(void);

// The owner entity is stored on the body, so a collision can name it.
PhysicsBody PhysicsCreateBody(ecs_entity_t owner, const BodyDesc *desc);
void PhysicsDestroyBody(PhysicsBody body);

Vec3 PhysicsGetPosition(PhysicsBody body);
Vec3 PhysicsGetVelocity(PhysicsBody body);

// Replaces horizontal velocity and leaves the vertical part to gravity. This is
// how a character is driven: direct control that the solver still gets to
// resolve against walls and other bodies.
void PhysicsDriveHorizontal(PhysicsBody body, Vec3 velocity);
void PhysicsApplyImpulse(PhysicsBody body, Vec3 impulse);

#endif // CORE_PHYSICS_H
