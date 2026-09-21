#include "core/components.h"

#include "core/physics.h"

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(PhysicsBody);
ECS_COMPONENT_DECLARE(Lifetime);
ECS_COMPONENT_DECLARE(Input);
ECS_COMPONENT_DECLARE(GameCamera);
ECS_COMPONENT_DECLARE(Contacts);

// Deleting an entity has to take its rigid body with it. Hanging the teardown
// off the component means no system has to remember, and it works whether the
// entity was deleted one at a time or wiped out with the rest of a restart.
static void PhysicsBodyDtor(void *ptr, int32_t count, const ecs_type_info_t *info) {
  (void)info;
  PhysicsBody *bodies = ptr;
  for (int32_t i = 0; i < count; ++i) {
    PhysicsDestroyBody(bodies[i]);
    bodies[i].handle = 0;
  }
}

void CoreComponentsRegister(ecs_world_t *world) {
  ECS_COMPONENT_DEFINE(world, Position);
  ECS_COMPONENT_DEFINE(world, PhysicsBody);
  ECS_COMPONENT_DEFINE(world, Lifetime);
  ECS_COMPONENT_DEFINE(world, Input);
  ECS_COMPONENT_DEFINE(world, GameCamera);
  ECS_COMPONENT_DEFINE(world, Contacts);

  ecs_set_hooks(world, PhysicsBody, {.dtor = PhysicsBodyDtor});
}
