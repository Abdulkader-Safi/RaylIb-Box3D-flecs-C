#include "entities/player.h"

#include "core/config.h"
#include "raymath.h"

void PlayerSpawn(Player *player, Physics *physics, Vector3 position) {
  player->entity.kind = ENTITY_PLAYER;
  player->entity.index = -1;
  player->health = PLAYER_MAX_HEALTH;
  player->fireCooldown = 0.0f;
  player->aimDirection = (Vector3){0.0f, 0.0f, 1.0f};
  player->bodyId = PhysicsCreateCharacter(physics, position, PLAYER_RADIUS, PLAYER_HALF_HEIGHT,
                                          CATEGORY_PLAYER, CATEGORY_ARENA | CATEGORY_ENEMY,
                                          &player->entity);
}

void PlayerUpdate(Player *player, const InputState *input, float dt) {
  if (player->fireCooldown > 0.0f) {
    player->fireCooldown -= dt;
  }

  // Screen up is -Z, so forward input maps to negative Z.
  Vector3 velocity = {input->move.x * PLAYER_SPEED, 0.0f, -input->move.y * PLAYER_SPEED};
  PhysicsDriveHorizontal(player->bodyId, velocity);

  Vector3 position = PlayerPosition(player);
  Vector3 toAim = {input->aimPoint.x - position.x, 0.0f, input->aimPoint.z - position.z};
  if (Vector3LengthSqr(toAim) > 1e-6f) {
    player->aimDirection = Vector3Normalize(toAim);
  }
}

bool PlayerTryFire(Player *player) {
  if (player->fireCooldown > 0.0f) {
    return false;
  }
  player->fireCooldown = PLAYER_FIRE_INTERVAL;
  return true;
}

void PlayerDamage(Player *player, int amount) {
  player->health -= amount;
  if (player->health < 0) {
    player->health = 0;
  }
}

Vector3 PlayerPosition(const Player *player) { return PhysicsBodyPosition(player->bodyId); }

Vector3 PlayerMuzzlePoint(const Player *player) {
  float reach = PLAYER_RADIUS + BULLET_RADIUS + 0.25f;
  return Vector3Add(PlayerPosition(player), Vector3Scale(player->aimDirection, reach));
}

bool PlayerIsAlive(const Player *player) { return player->health > 0; }
