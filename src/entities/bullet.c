#include "entities/bullet.h"

#include "raymath.h"

#include <string.h>

void BulletPoolInit(BulletPool *pool) {
  memset(pool, 0, sizeof(*pool));
  for (int i = 0; i < MAX_BULLETS; ++i) {
    pool->items[i].entity.kind = ENTITY_BULLET;
    pool->items[i].entity.index = i;
  }
}

void BulletFire(BulletPool *pool, Physics *physics, Vector3 origin, Vector3 direction) {
  for (int i = 0; i < MAX_BULLETS; ++i) {
    Bullet *bullet = pool->items + i;
    if (bullet->alive) {
      continue;
    }
    bullet->alive = true;
    bullet->timeToLive = BULLET_LIFETIME;
    bullet->bodyId = PhysicsCreateProjectile(physics, origin, Vector3Scale(direction, BULLET_SPEED),
                                             BULLET_RADIUS, CATEGORY_BULLET,
                                             CATEGORY_ARENA | CATEGORY_ENEMY, &bullet->entity);
    pool->aliveCount += 1;
    return;
  }
  // Pool full: the shot is dropped. With MAX_BULLETS well above fire rate times
  // lifetime this cannot happen in normal play.
}

void BulletPoolUpdate(BulletPool *pool, float dt) {
  for (int i = 0; i < MAX_BULLETS; ++i) {
    Bullet *bullet = pool->items + i;
    if (!bullet->alive) {
      continue;
    }
    bullet->timeToLive -= dt;
    if (bullet->timeToLive <= 0.0f) {
      b3DestroyBody(bullet->bodyId);
      bullet->alive = false;
      pool->aliveCount -= 1;
    }
  }
}

void BulletMarkSpent(Bullet *bullet) {
  if (bullet->alive) {
    bullet->timeToLive = 0.0f;
  }
}

Vector3 BulletVelocity(const Bullet *bullet) {
  return PhysicsFromB3(b3Body_GetLinearVelocity(bullet->bodyId));
}
