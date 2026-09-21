// A fixed pool of projectiles. Slots are reused, so firing never allocates.
#ifndef BULLET_H
#define BULLET_H

#include "core/config.h"
#include "entities/entity.h"
#include "physics/physics.h"

typedef struct Bullet {
  Entity entity; // First member: the body points its user data at this.
  b3BodyId bodyId;
  float timeToLive;
  bool alive;
} Bullet;

typedef struct BulletPool {
  Bullet items[MAX_BULLETS];
  int aliveCount;
} BulletPool;

void BulletPoolInit(BulletPool *pool);

void BulletFire(BulletPool *pool, Physics *physics, Vector3 origin, Vector3 direction);

// Ages every bullet and removes the ones that expired or were marked spent.
void BulletPoolUpdate(BulletPool *pool, float dt);

// Marks a bullet for removal at the end of the frame. Safe to call twice.
void BulletMarkSpent(Bullet *bullet);

Vector3 BulletVelocity(const Bullet *bullet);

#endif // BULLET_H
