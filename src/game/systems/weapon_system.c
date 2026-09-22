// Ages every weapon's cooldown and fires when the trigger is held and the
// weapon is ready.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/spawn/spawn.h"
#include "game/systems/systems.h"

// One frame cannot produce more shots than there are weapons, and there is one
// weapon. The cap is here so the loop below can finish before anything spawns.
#define MAX_SHOTS_PER_FRAME 16

typedef struct Shot {
  Vec3 muzzle;
  Vec3 direction;
  Weapon weapon;
} Shot;

static void WeaponFireSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const Aim *aims = ecs_field(it, Aim, 1);
  Weapon *weapons = ecs_field(it, Weapon, 2);

  const Input *input = ecs_singleton_get(it->world, Input);
  const GameState *state = ecs_singleton_get(it->world, GameState);
  const PlayerTracker *tracker = ecs_singleton_get(it->world, PlayerTracker);

  const Powerups *powerups =
      tracker->entity != 0 ? ecs_get(it->world, tracker->entity, Powerups) : NULL;
  bool rapid = powerups != NULL && powerups->rapidFire > 0.0f;

  Shot shots[MAX_SHOTS_PER_FRAME];
  int shotCount = 0;

  for (int i = 0; i < it->count; ++i) {
    if (weapons[i].cooldown > 0.0f) {
      weapons[i].cooldown -= it->delta_time;
    }
    if (state->mode != MODE_PLAYING || !input->fire || state->ignoreFireUntilRelease ||
        weapons[i].cooldown > 0.0f) {
      continue;
    }
    if (shotCount == MAX_SHOTS_PER_FRAME) {
      break;
    }

    // The bullet starts clear of the shooter. Born any closer and it collides
    // with the body that fired it on the very first step.
    float reach = PLAYER_RADIUS + weapons[i].bulletRadius + 0.25f;
    shots[shotCount++] = (Shot){
        .muzzle = Vec3Add(positions[i].value, Vec3Scale(aims[i].direction, reach)),
        .direction = aims[i].direction,
        .weapon = weapons[i],
    };
    weapons[i].cooldown = rapid ? POWERUP_RAPID_INTERVAL : weapons[i].interval;
  }

  // Spawning happens after the loop. This system is immediate, so a spawn
  // takes effect at once, and doing that mid-iteration would be changing the
  // world out from under the fields read above.
  for (int i = 0; i < shotCount; ++i) {
    SpawnBullet(it->world, shots[i].muzzle, shots[i].direction, &shots[i].weapon);
  }

  // Shooting tells the room where you are. It is the main way a player gives
  // themselves away, and the main reason to stop shooting.
  if (shotCount > 0) {
    Alert *alert = ecs_singleton_get_mut(it->world, Alert);
    alert->position = shots[shotCount - 1].muzzle;
    alert->timer = ALERT_SECONDS;
    alert->active = true;
  }
}

void WeaponSystemRegister(ecs_world_t *world) {
  SystemRegisterImmediate(world, "WeaponFireSystem", PhaseLogic, WeaponFireSystem,
                          "[in] Position, [in] Aim, Weapon, Player");
}
