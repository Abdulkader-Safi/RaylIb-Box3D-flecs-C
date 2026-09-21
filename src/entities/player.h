#ifndef PLAYER_H
#define PLAYER_H

#include "core/input.h"
#include "entities/entity.h"
#include "physics/physics.h"

typedef struct Player {
  Entity entity; // First member: the body points its user data at this.
  b3BodyId bodyId;
  int health;
  float fireCooldown;
  Vector3 aimDirection; // Unit vector on the horizontal plane.
} Player;

void PlayerSpawn(Player *player, Physics *physics, Vector3 position);

// Applies movement and aim for this frame and ages the weapon cooldown.
void PlayerUpdate(Player *player, const InputState *input, float dt);

// True once per shot the weapon is ready to take, and starts the next cooldown.
bool PlayerTryFire(Player *player);

void PlayerDamage(Player *player, int amount);
Vector3 PlayerPosition(const Player *player);
Vector3 PlayerMuzzlePoint(const Player *player);
bool PlayerIsAlive(const Player *player);

#endif // PLAYER_H
