// How every entity in this game is built. One function per thing, so the full
// recipe for a player or an enemy is in one place you can read top to bottom.
#ifndef GAME_SPAWN_H
#define GAME_SPAWN_H

#include "core/core.h"
#include "game/components/components.h"

// Spawning runs with deferring switched off. These bracket a spawn so the
// entity is whole the moment it exists, and they nest, so a spawn called from
// inside another one leaves the state alone. Level building uses them too.
bool SpawnBegin(ecs_world_t *world);
void SpawnEnd(ecs_world_t *world, bool suspended);

ecs_entity_t SpawnPlayer(ecs_world_t *world, Vec3 position);
ecs_entity_t SpawnEnemy(ecs_world_t *world, Vec3 position, EnemyTier tier);
ecs_entity_t SpawnBullet(ecs_world_t *world, Vec3 origin, Vec3 direction, const Weapon *weapon);

// A solid box for walls and cover, or a visual one for floors.
ecs_entity_t SpawnBlock(ecs_world_t *world, Vec3 centre, Vec3 size, Color color, bool solid);
ecs_entity_t SpawnFloorSlab(ecs_world_t *world, Vec3 centre, Vec3 size);

ecs_entity_t SpawnExit(ecs_world_t *world, Vec3 position);
ecs_entity_t SpawnDoor(ecs_world_t *world, Vec3 position);
ecs_entity_t SpawnSpawner(ecs_world_t *world, Vec3 position, EnemyTier tier);
ecs_entity_t SpawnPickup(ecs_world_t *world, Vec3 position, PickupKind kind, int amount);

#endif // GAME_SPAWN_H
