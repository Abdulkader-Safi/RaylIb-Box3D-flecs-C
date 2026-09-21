// Whether the simulation is running.
//
// Pausing is a framework concern because the framework owns the things that
// would otherwise keep moving: the physics step and the lifetime countdown. A
// game sets the flag, and anything time-based checks it.
#ifndef CORE_CLOCK_H
#define CORE_CLOCK_H

#include "flecs.h"

#include <stdbool.h>

typedef struct Clock {
  bool paused;
} Clock;

extern ECS_COMPONENT_DECLARE(Clock);

void ClockRegister(ecs_world_t *world);
void ClockSetPaused(ecs_world_t *world, bool paused);
bool ClockIsPaused(const ecs_world_t *world);

#endif // CORE_CLOCK_H
