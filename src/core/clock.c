#include "core/clock.h"

ECS_COMPONENT_DECLARE(Clock);

void ClockRegister(ecs_world_t *world) {
  ECS_COMPONENT_DEFINE(world, Clock);
  ecs_singleton_set(world, Clock, {.paused = false});
}

void ClockSetPaused(ecs_world_t *world, bool paused) {
  ecs_singleton_get_mut(world, Clock)->paused = paused;
}

bool ClockIsPaused(const ecs_world_t *world) {
  return ecs_singleton_get(world, Clock)->paused;
}
