#include "core/lifetime.h"

#include "core/components.h"
#include "core/phases.h"

static void LifetimeSystem(ecs_iter_t *it) {
  Lifetime *lifetimes = ecs_field(it, Lifetime, 0);

  for (int i = 0; i < it->count; ++i) {
    lifetimes[i].remaining -= it->delta_time;
    if (lifetimes[i].remaining <= 0.0f) {
      // flecs holds deletes until the end of the frame, so this is safe in the
      // middle of iterating, and the body goes with the entity.
      ecs_delete(it->world, it->entities[i]);
    }
  }
}

void LifetimeRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, LifetimeSystem, PhaseCleanup, Lifetime);
}
