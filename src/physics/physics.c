#include "physics/physics.h"

#include "core/config.h"

static b3ShapeDef MakeShapeDef(uint64_t category, uint64_t mask, float friction) {
  b3ShapeDef shapeDef = b3DefaultShapeDef();
  shapeDef.baseMaterial = b3DefaultSurfaceMaterial();
  shapeDef.baseMaterial.friction = friction;
  shapeDef.baseMaterial.restitution = 0.0f;
  shapeDef.filter.categoryBits = category;
  shapeDef.filter.maskBits = mask;
  // Every game shape reports contacts. At a few hundred bodies the bookkeeping
  // is free, and it keeps the hit rules in one switch in game.c.
  shapeDef.enableContactEvents = true;
  return shapeDef;
}

void PhysicsInit(Physics *physics) {
  b3WorldDef worldDef = b3DefaultWorldDef();
  worldDef.gravity = (b3Vec3){0.0f, PHYSICS_GRAVITY, 0.0f};
  physics->worldId = b3CreateWorld(&worldDef);
  physics->accumulator = 0.0f;
}

void PhysicsShutdown(Physics *physics) {
  b3DestroyWorld(physics->worldId);
  physics->worldId = (b3WorldId){0};
}

void PhysicsStep(Physics *physics, float dt, ContactCallback onContact, void *context) {
  physics->accumulator += dt;
  if (physics->accumulator > PHYSICS_MAX_ACCUMULATED) {
    physics->accumulator = PHYSICS_MAX_ACCUMULATED;
  }

  while (physics->accumulator >= PHYSICS_TIME_STEP) {
    b3World_Step(physics->worldId, PHYSICS_TIME_STEP, PHYSICS_SUB_STEPS);
    physics->accumulator -= PHYSICS_TIME_STEP;

    if (onContact == NULL) {
      continue;
    }

    // Events only describe the step that just ran, so they are drained here
    // rather than after the loop. Otherwise a hit during a catch-up step is lost.
    b3ContactEvents events = b3World_GetContactEvents(physics->worldId);
    for (int i = 0; i < events.beginCount; ++i) {
      b3ContactBeginTouchEvent *event = events.beginEvents + i;
      void *ownerA = b3Body_GetUserData(b3Shape_GetBody(event->shapeIdA));
      void *ownerB = b3Body_GetUserData(b3Shape_GetBody(event->shapeIdB));
      onContact(ownerA, ownerB, context);
    }
  }
}

b3BodyId PhysicsCreateStaticBox(Physics *physics, Vector3 center, Vector3 halfExtents,
                                uint64_t category, uint64_t mask, void *userData) {
  b3BodyDef bodyDef = b3DefaultBodyDef();
  bodyDef.type = b3_staticBody;
  bodyDef.position = PhysicsToB3(center);
  bodyDef.userData = userData;
  b3BodyId bodyId = b3CreateBody(physics->worldId, &bodyDef);

  b3BoxHull hull = b3MakeBoxHull(halfExtents.x, halfExtents.y, halfExtents.z);
  b3ShapeDef shapeDef = MakeShapeDef(category, mask, 0.7f);
  b3CreateHullShape(bodyId, &shapeDef, &hull.base);
  return bodyId;
}

b3BodyId PhysicsCreateCharacter(Physics *physics, Vector3 center, float radius, float halfHeight,
                                uint64_t category, uint64_t mask, void *userData) {
  b3BodyDef bodyDef = b3DefaultBodyDef();
  bodyDef.type = b3_dynamicBody;
  bodyDef.position = PhysicsToB3(center);
  bodyDef.userData = userData;
  bodyDef.enableSleep = false;
  // Locking rotation is what separates a character from a barrel: it absorbs
  // shoves from the crowd without spinning or falling flat.
  bodyDef.motionLocks.angularX = true;
  bodyDef.motionLocks.angularY = true;
  bodyDef.motionLocks.angularZ = true;
  b3BodyId bodyId = b3CreateBody(physics->worldId, &bodyDef);

  b3Capsule capsule = {
      .center1 = {0.0f, -halfHeight, 0.0f},
      .center2 = {0.0f, halfHeight, 0.0f},
      .radius = radius,
  };
  // Low friction so the crowd slides around obstacles instead of sticking.
  b3ShapeDef shapeDef = MakeShapeDef(category, mask, 0.15f);
  b3CreateCapsuleShape(bodyId, &shapeDef, &capsule);
  return bodyId;
}

b3BodyId PhysicsCreateProjectile(Physics *physics, Vector3 center, Vector3 velocity, float radius,
                                 uint64_t category, uint64_t mask, void *userData) {
  b3BodyDef bodyDef = b3DefaultBodyDef();
  bodyDef.type = b3_dynamicBody;
  bodyDef.position = PhysicsToB3(center);
  bodyDef.linearVelocity = PhysicsToB3(velocity);
  bodyDef.userData = userData;
  bodyDef.gravityScale = 0.0f;
  bodyDef.enableSleep = false;
  bodyDef.isBullet = true;
  b3BodyId bodyId = b3CreateBody(physics->worldId, &bodyDef);

  b3Sphere sphere = {.center = {0.0f, 0.0f, 0.0f}, .radius = radius};
  b3ShapeDef shapeDef = MakeShapeDef(category, mask, 0.0f);
  b3CreateSphereShape(bodyId, &shapeDef, &sphere);
  return bodyId;
}

Vector3 PhysicsBodyPosition(b3BodyId bodyId) {
  return PhysicsFromB3(b3Body_GetPosition(bodyId));
}

void PhysicsDriveHorizontal(b3BodyId bodyId, Vector3 velocity) {
  b3Vec3 current = b3Body_GetLinearVelocity(bodyId);
  b3Body_SetLinearVelocity(bodyId, (b3Vec3){velocity.x, current.y, velocity.z});
}
