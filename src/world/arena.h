// The playfield: a static floor and four static walls that keep everything in.
#ifndef ARENA_H
#define ARENA_H

#include "entities/entity.h"
#include "physics/physics.h"

typedef struct Arena {
  Entity entity; // First member: bodies point their user data at this.
  b3BodyId floorId;
  b3BodyId wallIds[4];
  float halfExtent;
} Arena;

void ArenaBuild(Arena *arena, Physics *physics);

// A point just inside the wall, used to drop enemies in from the edge.
Vector3 ArenaEdgeSpawnPoint(const Arena *arena, float height);

#endif // ARENA_H
