#include "world/arena.h"

#include "core/config.h"
#include "raymath.h"

#define FLOOR_HALF_THICKNESS 0.5f

void ArenaBuild(Arena *arena, Physics *physics) {
  arena->entity.kind = ENTITY_ARENA;
  arena->entity.index = -1;
  arena->halfExtent = ARENA_HALF_EXTENT;

  float half = arena->halfExtent;
  uint64_t everything = CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_BULLET;

  // Floor top sits at y = 0, so every spawn height is measured from zero.
  arena->floorId = PhysicsCreateStaticBox(physics, (Vector3){0.0f, -FLOOR_HALF_THICKNESS, 0.0f},
                                          (Vector3){half, FLOOR_HALF_THICKNESS, half}, CATEGORY_ARENA,
                                          everything, &arena->entity);

  float wallY = ARENA_WALL_HEIGHT * 0.5f;
  float offset = half + ARENA_WALL_THICKNESS * 0.5f;
  Vector3 centers[4] = {
      {0.0f, wallY, -offset},
      {0.0f, wallY, offset},
      {-offset, wallY, 0.0f},
      {offset, wallY, 0.0f},
  };
  Vector3 extents[4] = {
      {half, wallY, ARENA_WALL_THICKNESS * 0.5f},
      {half, wallY, ARENA_WALL_THICKNESS * 0.5f},
      {ARENA_WALL_THICKNESS * 0.5f, wallY, half},
      {ARENA_WALL_THICKNESS * 0.5f, wallY, half},
  };

  for (int i = 0; i < 4; ++i) {
    arena->wallIds[i] = PhysicsCreateStaticBox(physics, centers[i], extents[i], CATEGORY_ARENA,
                                               everything, &arena->entity);
  }
}

Vector3 ArenaEdgeSpawnPoint(const Arena *arena, float height) {
  float radius = arena->halfExtent - ENEMY_SPAWN_MARGIN;
  float angle = (float)GetRandomValue(0, 359) * DEG2RAD;
  return (Vector3){cosf(angle) * radius, height, sinf(angle) * radius};
}
