#include "core/physics.h"

#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/math_functions.h"
#include "core/phases.h"

// Physics advances in fixed slices so behaviour does not drift with frame rate.
#define PHYSICS_STEP (1.0f / 60.0f)
#define PHYSICS_SUB_STEPS 4
#define PHYSICS_MAX_CATCHUP 0.25f
#define PHYSICS_GRAVITY (-24.0f)

// One world for the process. A second one would need this to become a
// singleton component, which is a change worth making only if it is ever real.
static b3WorldId g_worldId;
static float g_accumulator;

static inline b3Vec3 ToB3(Vec3 v) { return (b3Vec3){v.x, v.y, v.z}; }
static inline Vec3 FromB3(b3Vec3 v) { return (Vec3){v.x, v.y, v.z}; }

// b3BodyId is three small fields. Packing it into one integer is what keeps
// Box3D out of the headers game code reads.
static inline uint64_t PackBody(b3BodyId id) {
  return (uint64_t)(uint32_t)id.index1 | ((uint64_t)id.world0 << 32) |
         ((uint64_t)id.generation << 48);
}

static inline b3BodyId UnpackBody(uint64_t handle) {
  b3BodyId id;
  id.index1 = (int32_t)(uint32_t)(handle & 0xffffffffu);
  id.world0 = (uint16_t)((handle >> 32) & 0xffffu);
  id.generation = (uint16_t)((handle >> 48) & 0xffffu);
  return id;
}

static b3ShapeDef MakeShapeDef(const BodyDesc *desc) {
  b3ShapeDef shapeDef = b3DefaultShapeDef();
  shapeDef.baseMaterial = b3DefaultSurfaceMaterial();
  shapeDef.baseMaterial.friction = desc->friction;
  shapeDef.baseMaterial.restitution = 0.0f;
  shapeDef.filter.categoryBits = desc->category;
  shapeDef.filter.maskBits = desc->mask;
  // Every body reports contacts. At this scale the bookkeeping is free, and it
  // means a game system can react to any collision without asking first.
  shapeDef.enableContactEvents = true;
  return shapeDef;
}

PhysicsBody PhysicsCreateBody(ecs_entity_t owner, const BodyDesc *desc) {
  b3BodyDef bodyDef = b3DefaultBodyDef();
  bodyDef.position = ToB3(desc->position);
  // The entity id rides along on the body, which is how a contact between two
  // shapes becomes a collision between two entities.
  bodyDef.userData = (void *)(uintptr_t)owner;

  switch (desc->kind) {
  case BODY_STATIC_BOX:
    bodyDef.type = b3_staticBody;
    break;
  case BODY_CHARACTER:
    bodyDef.type = b3_dynamicBody;
    bodyDef.enableSleep = false;
    bodyDef.motionLocks.angularX = true;
    bodyDef.motionLocks.angularY = true;
    bodyDef.motionLocks.angularZ = true;
    break;
  case BODY_PROJECTILE:
    bodyDef.type = b3_dynamicBody;
    bodyDef.enableSleep = false;
    bodyDef.gravityScale = 0.0f;
    bodyDef.isBullet = true;
    bodyDef.linearVelocity = ToB3(desc->velocity);
    break;
  }

  b3BodyId bodyId = b3CreateBody(g_worldId, &bodyDef);
  b3ShapeDef shapeDef = MakeShapeDef(desc);

  switch (desc->kind) {
  case BODY_STATIC_BOX: {
    b3BoxHull hull =
        b3MakeBoxHull(desc->halfExtents.x, desc->halfExtents.y, desc->halfExtents.z);
    b3CreateHullShape(bodyId, &shapeDef, &hull.base);
    break;
  }
  case BODY_CHARACTER: {
    b3Capsule capsule = {
        .center1 = {0.0f, -desc->halfHeight, 0.0f},
        .center2 = {0.0f, desc->halfHeight, 0.0f},
        .radius = desc->radius,
    };
    b3CreateCapsuleShape(bodyId, &shapeDef, &capsule);
    break;
  }
  case BODY_PROJECTILE: {
    b3Sphere sphere = {.center = {0.0f, 0.0f, 0.0f}, .radius = desc->radius};
    b3CreateSphereShape(bodyId, &shapeDef, &sphere);
    break;
  }
  }

  return (PhysicsBody){PackBody(bodyId)};
}

void PhysicsDestroyBody(PhysicsBody body) {
  if (body.handle == 0) {
    return;
  }
  b3BodyId bodyId = UnpackBody(body.handle);
  if (b3Body_IsValid(bodyId)) {
    b3DestroyBody(bodyId);
  }
}

Vec3 PhysicsGetPosition(PhysicsBody body) {
  return FromB3(b3Body_GetPosition(UnpackBody(body.handle)));
}

Vec3 PhysicsGetVelocity(PhysicsBody body) {
  return FromB3(b3Body_GetLinearVelocity(UnpackBody(body.handle)));
}

void PhysicsDriveHorizontal(PhysicsBody body, Vec3 velocity) {
  b3BodyId bodyId = UnpackBody(body.handle);
  b3Vec3 current = b3Body_GetLinearVelocity(bodyId);
  b3Body_SetLinearVelocity(bodyId, (b3Vec3){velocity.x, current.y, velocity.z});
}

void PhysicsApplyImpulse(PhysicsBody body, Vec3 impulse) {
  b3Body_ApplyLinearImpulseToCenter(UnpackBody(body.handle), ToB3(impulse), true);
}

// Reads the contacts Box3D buffered for the step that just ran and appends them
// to this frame's list as entity pairs.
static void CollectContacts(ecs_world_t *world, Contacts *contacts) {
  b3ContactEvents events = b3World_GetContactEvents(g_worldId);
  for (int i = 0; i < events.beginCount; ++i) {
    if (contacts->count >= MAX_CONTACTS_PER_FRAME) {
      return; // Losing the tail of a freak frame beats writing past the array.
    }
    b3ContactBeginTouchEvent *event = events.beginEvents + i;
    ecs_entity_t a = (ecs_entity_t)(uintptr_t)b3Body_GetUserData(b3Shape_GetBody(event->shapeIdA));
    ecs_entity_t b = (ecs_entity_t)(uintptr_t)b3Body_GetUserData(b3Shape_GetBody(event->shapeIdB));
    if (!ecs_is_alive(world, a) || !ecs_is_alive(world, b)) {
      continue;
    }
    contacts->items[contacts->count++] = (Contact){a, b};
  }
}

static void PhysicsStepSystem(ecs_iter_t *it) {
  Contacts *contacts = ecs_singleton_ensure(it->world, Contacts);
  contacts->count = 0;

  g_accumulator += it->delta_time;
  if (g_accumulator > PHYSICS_MAX_CATCHUP) {
    g_accumulator = PHYSICS_MAX_CATCHUP;
  }

  while (g_accumulator >= PHYSICS_STEP) {
    b3World_Step(g_worldId, PHYSICS_STEP, PHYSICS_SUB_STEPS);
    g_accumulator -= PHYSICS_STEP;
    // Contact events describe only the step that just ran, so they are drained
    // inside the loop. Draining after it would lose every hit that happened
    // during a catch-up step.
    CollectContacts(it->world, contacts);
  }
}

// Copies body positions into Position so that nothing downstream, drawing
// included, has to know a physics engine exists.
static void PhysicsSyncSystem(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  const PhysicsBody *bodies = ecs_field(it, PhysicsBody, 1);

  for (int i = 0; i < it->count; ++i) {
    positions[i].value = PhysicsGetPosition(bodies[i]);
  }
}

void PhysicsRegister(ecs_world_t *world) {
  b3WorldDef worldDef = b3DefaultWorldDef();
  worldDef.gravity = (b3Vec3){0.0f, PHYSICS_GRAVITY, 0.0f};
  g_worldId = b3CreateWorld(&worldDef);
  g_accumulator = 0.0f;

  ecs_singleton_set(world, Contacts, {0});

  ECS_SYSTEM(world, PhysicsStepSystem, PhasePhysics, 0);
  ECS_SYSTEM(world, PhysicsSyncSystem, PhaseSync, Position, [in] PhysicsBody);
}

void PhysicsShutdown(void) {
  b3DestroyWorld(g_worldId);
  g_worldId = (b3WorldId){0};
}
