// How every entity in this game is built. One function per thing, so the full
// recipe for a player or an enemy is in one place you can read top to bottom.
#ifndef GAME_SPAWN_H
#define GAME_SPAWN_H

#include "core/core.h"
#include "game/components/components.h"

// Floor and four walls.
void SpawnArena(ecs_world_t *world);

ecs_entity_t SpawnPlayer(ecs_world_t *world, Vec3 position);
ecs_entity_t SpawnEnemy(ecs_world_t *world, Vec3 position);
ecs_entity_t SpawnBullet(ecs_world_t *world, Vec3 origin, Vec3 direction, const Weapon *weapon);

// A point just inside the wall for an enemy to walk in from.
Vec3 SpawnPointOnArenaEdge(float height);

// Clears whatever is left of a run and builds a fresh one.
void SpawnFreshRun(ecs_world_t *world);

#endif // GAME_SPAWN_H
