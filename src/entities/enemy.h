// A fixed pool of chasers. They steer at the player and let the solver sort out
// the shoving when they arrive together.
#ifndef ENEMY_H
#define ENEMY_H

#include "core/config.h"
#include "entities/entity.h"
#include "entities/player.h"
#include "physics/physics.h"

typedef struct Enemy {
  Entity entity; // First member: the body points its user data at this.
  b3BodyId bodyId;
  int health;
  float touchCooldown;
  bool alive;
} Enemy;

typedef struct EnemyPool {
  Enemy items[MAX_ENEMIES];
  int aliveCount;
} EnemyPool;

void EnemyPoolInit(EnemyPool *pool);
void EnemyPoolClear(EnemyPool *pool);

void EnemySpawn(EnemyPool *pool, Physics *physics, Vector3 position);

// Steers every enemy at the player and bites when it gets close enough.
void EnemyPoolUpdate(EnemyPool *pool, Player *player, float dt);

// Subtracts health and shoves the body along knockback.
void EnemyDamage(Enemy *enemy, int amount, Vector3 knockback);

// Removes enemies whose health ran out. Returns how many died this frame.
int EnemyPoolReap(EnemyPool *pool);

Vector3 EnemyPosition(const Enemy *enemy);

#endif // ENEMY_H
