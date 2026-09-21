// Registering a system that is allowed to change the world on the spot.
//
// While ecs_progress runs, flecs puts the world in readonly mode so systems can
// iterate their components safely. Creating or deleting an entity is queued and
// applied at a later sync point, which is the right default and wrong for
// spawning: a bullet created that way would have a rigid body colliding in the
// same physics step while its Damage component was still sitting in the queue.
//
// A system marked immediate runs on its own with readonly mode lifted, so the
// spawn helpers in game/spawn can build an entity whole before returning.
//
// Use it only for systems that spawn or wipe entities. Everything else should
// stay on the normal queue.
#ifndef CORE_SYSTEM_H
#define CORE_SYSTEM_H

#include "flecs.h"

// `terms` is a query expression such as "[in] Position, Player", or NULL for a
// system that iterates nothing and only reads singletons.
void SystemRegisterImmediate(ecs_world_t *world, const char *name, ecs_entity_t phase,
                             ecs_iter_action_t callback, const char *terms);

#endif // CORE_SYSTEM_H
