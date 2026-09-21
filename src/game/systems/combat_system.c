// Turns the collisions the physics step produced into damage.
//
// The framework hands over a flat list of entity pairs. Whether a pair means
// anything is entirely this system's business: a pair matters when one side
// deals damage and the other has health.
#include "core/core.h"
#include "game/components/components.h"
#include "game/systems/systems.h"

// Applies one side of a pair. Called twice per contact, once each way round,
// so the order the physics engine reported the two shapes in does not matter.
static void ResolveOneWay(ecs_world_t *world, ecs_entity_t attacker, ecs_entity_t target) {
  Damage *damage = ecs_get_mut(world, attacker, Damage);
  // No damage to give, or it was already given. A bullet that reaches two
  // enemies inside one physics step shows up twice in this list, and the
  // delete below is queued rather than immediate, so the entity is still here
  // and still armed the second time round.
  if (damage == NULL || damage->amount <= 0) {
    return;
  }

  Health *health = ecs_get_mut(world, target, Health);
  if (health != NULL && health->current > 0) {
    health->current -= damage->amount;

    // Shove the target along the line the bullet was travelling. The bullet's
    // own mass is far too small to do this through the solver.
    const PhysicsBody *body = ecs_get(world, target, PhysicsBody);
    const PhysicsBody *attackerBody = ecs_get(world, attacker, PhysicsBody);
    if (body != NULL && attackerBody != NULL) {
      Vec3 heading = Vec3Normalize(PhysicsGetVelocity(*attackerBody));
      PhysicsApplyImpulse(*body, Vec3Scale(heading, damage->knockback));
    }
  }

  // Spent either way: a bullet that reaches a wall is just as finished as one
  // that reaches an enemy. Disarming it is what makes one bullet one hit.
  damage->amount = 0;
  ecs_delete(world, attacker);
}

static void CombatSystem(ecs_iter_t *it) {
  const Contacts *contacts = ecs_singleton_get(it->world, Contacts);

  for (int i = 0; i < contacts->count; ++i) {
    ecs_entity_t a = contacts->items[i].a;
    ecs_entity_t b = contacts->items[i].b;
    if (!ecs_is_alive(it->world, a) || !ecs_is_alive(it->world, b)) {
      continue;
    }
    ResolveOneWay(it->world, a, b);
    ResolveOneWay(it->world, b, a);
  }
}

void CombatSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, CombatSystem, PhasePostPhysics, 0);
}
