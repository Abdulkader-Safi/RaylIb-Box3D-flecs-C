// Coins, health, powerups and the keycard. They drift toward the player once
// they are close, then vanish into whatever they do.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/systems/systems.h"

static void CollectOne(ecs_world_t *world, const Pickup *pickup, GameState *state,
                       Health *playerHealth, Powerups *powerups) {
  switch (pickup->kind) {
  case PICKUP_COIN:
    state->coins += pickup->amount;
    state->score += SCORE_PER_COIN * pickup->amount;
    break;

  case PICKUP_HEALTH:
    playerHealth->current += pickup->amount;
    if (playerHealth->current > playerHealth->max) {
      playerHealth->current = playerHealth->max;
    }
    break;

  case PICKUP_RAPID_FIRE:
    powerups->rapidFire = POWERUP_RAPID_SECONDS;
    break;

  case PICKUP_SHIELD:
    powerups->shield = POWERUP_SHIELD_SECONDS;
    break;

  case PICKUP_KEYCARD:
    state->hasKeycard = true;
    break;
  }
}

static void PickupSystem(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  const Pickup *pickups = ecs_field(it, Pickup, 1);

  GameState *state = ecs_singleton_get_mut(it->world, GameState);
  const PlayerTracker *tracker = ecs_singleton_get(it->world, PlayerTracker);
  if (state->mode != MODE_PLAYING || tracker->entity == 0) {
    return;
  }

  Health *playerHealth = ecs_get_mut(it->world, tracker->entity, Health);
  Powerups *powerups = ecs_get_mut(it->world, tracker->entity, Powerups);
  if (playerHealth == NULL || powerups == NULL) {
    return;
  }

  for (int i = 0; i < it->count; ++i) {
    Vec3 toPlayer = Vec3Flat(Vec3Sub(tracker->position, positions[i].value));
    float distance = Vec3Length(toPlayer);

    if (distance <= PICKUP_COLLECT_RANGE) {
      CollectOne(it->world, pickups + i, state, playerHealth, powerups);
      ecs_delete(it->world, it->entities[i]);
      continue;
    }

    // Drawn in rather than walked over. It makes clearing a room feel like it
    // paid out, without asking the player to hoover up every last coin.
    if (distance < PICKUP_MAGNET_RANGE && distance > 1e-4f) {
      Vec3 step = Vec3Scale(toPlayer, PICKUP_MAGNET_SPEED * it->delta_time / distance);
      positions[i].value = Vec3Add(positions[i].value, step);
    }
  }
}

void PickupSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, PickupSystem, PhaseLogic, Position, [in] Pickup);
}
