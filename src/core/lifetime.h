// Anything with a Lifetime counts down and is deleted when it runs out.
#ifndef CORE_LIFETIME_H
#define CORE_LIFETIME_H

#include "flecs.h"

void LifetimeRegister(ecs_world_t *world);

#endif // CORE_LIFETIME_H
