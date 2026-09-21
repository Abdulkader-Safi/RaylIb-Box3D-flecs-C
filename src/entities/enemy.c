#include "entities/enemy.h"

#include "raymath.h"

#include <string.h>

void EnemyPoolInit(EnemyPool *pool) {
  memset(pool, 0, sizeof(*pool));
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    pool->items[i].entity.kind = ENTITY_ENEMY;
    pool->items[i].entity.index = i;
  }
}

void EnemyPoolClear(EnemyPool *pool) {
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    if (pool->items[i].alive) {
      b3DestroyBody(pool->items[i].bodyId);
      pool->items[i].alive = false;
    }
  }
  pool->aliveCount = 0;
}

void EnemySpawn(EnemyPool *pool, Physics *physics, Vector3 position) {
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    Enemy *enemy = pool->items + i;
    if (enemy->alive) {
      continue;
    }
    enemy->alive = true;
    enemy->health = ENEMY_MAX_HEALTH;
    enemy->touchCooldown = 0.0f;
    enemy->bodyId = PhysicsCreateCharacter(physics, position, ENEMY_RADIUS, ENEMY_HALF_HEIGHT,
                                           CATEGORY_ENEMY,
                                           CATEGORY_ARENA | CATEGORY_PLAYER | CATEGORY_ENEMY |
                                               CATEGORY_BULLET,
                                           &enemy->entity);
    pool->aliveCount += 1;
    return;
  }
}

void EnemyPoolUpdate(EnemyPool *pool, Player *player, float dt) {
  Vector3 playerPosition = PlayerPosition(player);
  float biteRange = PLAYER_RADIUS + ENEMY_RADIUS + 0.35f;

  for (int i = 0; i < MAX_ENEMIES; ++i) {
    Enemy *enemy = pool->items + i;
    if (!enemy->alive) {
      continue;
    }

    if (enemy->touchCooldown > 0.0f) {
      enemy->touchCooldown -= dt;
    }

    Vector3 position = EnemyPosition(enemy);
    Vector3 toPlayer = {playerPosition.x - position.x, 0.0f, playerPosition.z - position.z};
    float distance = Vector3Length(toPlayer);
    if (distance > 1e-4f) {
      Vector3 direction = Vector3Scale(toPlayer, 1.0f / distance);
      PhysicsDriveHorizontal(enemy->bodyId, Vector3Scale(direction, ENEMY_SPEED));
    }

    // Contact events fire once per touch, but an enemy leaning on the player
    // never stops touching, so proximity plus a cooldown is the honest test.
    if (distance <= biteRange && enemy->touchCooldown <= 0.0f && PlayerIsAlive(player)) {
      PlayerDamage(player, ENEMY_TOUCH_DAMAGE);
      enemy->touchCooldown = ENEMY_TOUCH_INTERVAL;
    }
  }
}

void EnemyDamage(Enemy *enemy, int amount, Vector3 knockback) {
  if (!enemy->alive) {
    return;
  }
  enemy->health -= amount;
  b3Body_ApplyLinearImpulseToCenter(enemy->bodyId, PhysicsToB3(knockback), true);
}

int EnemyPoolReap(EnemyPool *pool) {
  int killed = 0;
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    Enemy *enemy = pool->items + i;
    if (!enemy->alive || enemy->health > 0) {
      continue;
    }
    b3DestroyBody(enemy->bodyId);
    enemy->alive = false;
    pool->aliveCount -= 1;
    killed += 1;
  }
  return killed;
}

Vector3 EnemyPosition(const Enemy *enemy) { return PhysicsBodyPosition(enemy->bodyId); }
