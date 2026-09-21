#include "game/spawn/spawn.h"

#include "game/config.h"

// Spawning happens with deferring switched off.
//
// While a system runs, flecs queues changes and applies them at a later sync
// point. For a spawn that means the entity id comes back straight away but its
// components do not land until afterwards, so a bullet could collide on its
// first physics step while its Damage component was still in the queue. Doing
// the work immediately keeps a spawned entity whole the moment it exists.
//
// ecs_defer_suspend asserts unless something is actually being deferred, so
// the state is checked first. That also makes these nest: an inner spawn
// called from SpawnFreshRun sees deferring already off and leaves it alone.
static bool SpawnBegin(ecs_world_t *world) {
  if (!ecs_is_deferred(world)) {
    return false;
  }
  ecs_defer_suspend(world);
  return true;
}

static void SpawnEnd(ecs_world_t *world, bool suspended) {
  if (suspended) {
    ecs_defer_resume(world);
  }
}

// A body plus the position it writes into. Every physical thing starts here.
static ecs_entity_t SpawnPhysical(ecs_world_t *world, const BodyDesc *desc) {
  ecs_entity_t entity = ecs_new(world);
  ecs_add_id(world, entity, Spawned);
  ecs_set(world, entity, Position, {desc->position});
  // The body needs the entity id, so it is built after the entity and attached
  // once. Setting PhysicsBody twice would leak the first body.
  PhysicsBody body = PhysicsCreateBody(entity, desc);
  ecs_set_ptr(world, entity, PhysicsBody, &body);
  return entity;
}

static void SpawnWall(ecs_world_t *world, Vec3 center, Vec3 halfExtents) {
  BodyDesc desc = {
      .kind = BODY_STATIC_BOX,
      .position = center,
      .halfExtents = halfExtents,
      .friction = 0.7f,
      .category = CATEGORY_ARENA,
      .mask = CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_BULLET,
  };
  ecs_entity_t wall = SpawnPhysical(world, &desc);
  ecs_set(world, wall, BoxVisual,
          {.size = Vec3Scale(halfExtents, 2.0f), .color = COLOR_WALL, .wireframe = true});
}

void SpawnArena(ecs_world_t *world) {
  bool suspended = SpawnBegin(world);
  const float half = ARENA_HALF_EXTENT;
  const float floorHalf = ARENA_FLOOR_HALF_THICKNESS;

  // The floor's top surface sits at y = 0, so every spawn height in the game is
  // measured from zero.
  BodyDesc floorDesc = {
      .kind = BODY_STATIC_BOX,
      .position = Vec3Make(0.0f, -floorHalf, 0.0f),
      .halfExtents = Vec3Make(half, floorHalf, half),
      .friction = 0.7f,
      .category = CATEGORY_ARENA,
      .mask = CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_BULLET,
  };
  ecs_entity_t floor = SpawnPhysical(world, &floorDesc);
  // Drawn 2cm lower than it collides, so the grid lines at y = 0 do not fight
  // with the floor surface for the same pixels.
  ecs_set(world, floor, Position, {Vec3Make(0.0f, -floorHalf - 0.02f, 0.0f)});
  ecs_set(world, floor, BoxVisual,
          {.size = Vec3Make(half * 2.0f, floorHalf * 2.0f, half * 2.0f), .color = COLOR_FLOOR});

  const float wallY = ARENA_WALL_HEIGHT * 0.5f;
  const float offset = half + ARENA_WALL_THICKNESS * 0.5f;
  const float thin = ARENA_WALL_THICKNESS * 0.5f;

  SpawnWall(world, Vec3Make(0.0f, wallY, -offset), Vec3Make(half, wallY, thin));
  SpawnWall(world, Vec3Make(0.0f, wallY, offset), Vec3Make(half, wallY, thin));
  SpawnWall(world, Vec3Make(-offset, wallY, 0.0f), Vec3Make(thin, wallY, half));
  SpawnWall(world, Vec3Make(offset, wallY, 0.0f), Vec3Make(thin, wallY, half));
  SpawnEnd(world, suspended);
}

ecs_entity_t SpawnPlayer(ecs_world_t *world, Vec3 position) {
  bool suspended = SpawnBegin(world);
  BodyDesc desc = {
      .kind = BODY_CHARACTER,
      .position = position,
      .radius = PLAYER_RADIUS,
      .halfHeight = PLAYER_HALF_HEIGHT,
      // Low friction so the player slides along a wall instead of catching on it.
      .friction = 0.15f,
      .category = CATEGORY_PLAYER,
      .mask = CATEGORY_ARENA | CATEGORY_ENEMY,
  };
  ecs_entity_t player = SpawnPhysical(world, &desc);

  ecs_add_id(world, player, Player);
  ecs_set(world, player, Health, {PLAYER_MAX_HEALTH, PLAYER_MAX_HEALTH});
  ecs_set(world, player, MoveSpeed, {PLAYER_SPEED});
  ecs_set(world, player, Aim, {Vec3Make(0.0f, 0.0f, 1.0f)});
  ecs_set(world, player, Weapon,
          {
              .interval = WEAPON_INTERVAL,
              .cooldown = 0.0f,
              .damage = WEAPON_DAMAGE,
              .knockback = WEAPON_KNOCKBACK,
              .bulletSpeed = BULLET_SPEED,
              .bulletRadius = BULLET_RADIUS,
              .bulletLifetime = BULLET_LIFETIME,
          });
  ecs_set(world, player, CapsuleVisual,
          {.radius = PLAYER_RADIUS, .halfHeight = PLAYER_HALF_HEIGHT, .color = COLOR_PLAYER});
  SpawnEnd(world, suspended);
  return player;
}

ecs_entity_t SpawnEnemy(ecs_world_t *world, Vec3 position) {
  bool suspended = SpawnBegin(world);
  BodyDesc desc = {
      .kind = BODY_CHARACTER,
      .position = position,
      .radius = ENEMY_RADIUS,
      .halfHeight = ENEMY_HALF_HEIGHT,
      .friction = 0.15f,
      .category = CATEGORY_ENEMY,
      .mask = CATEGORY_ARENA | CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_BULLET,
  };
  ecs_entity_t enemy = SpawnPhysical(world, &desc);

  ecs_add_id(world, enemy, Enemy);
  ecs_set(world, enemy, Health, {ENEMY_MAX_HEALTH, ENEMY_MAX_HEALTH});
  ecs_set(world, enemy, MoveSpeed, {ENEMY_SPEED});
  ecs_set(world, enemy, Bite,
          {
              .interval = ENEMY_BITE_INTERVAL,
              .cooldown = 0.0f,
              .damage = ENEMY_BITE_DAMAGE,
              .range = PLAYER_RADIUS + ENEMY_RADIUS + 0.35f,
          });
  ecs_set(world, enemy, CapsuleVisual,
          {.radius = ENEMY_RADIUS, .halfHeight = ENEMY_HALF_HEIGHT, .color = COLOR_ENEMY});
  SpawnEnd(world, suspended);
  return enemy;
}

ecs_entity_t SpawnBullet(ecs_world_t *world, Vec3 origin, Vec3 direction, const Weapon *weapon) {
  bool suspended = SpawnBegin(world);
  BodyDesc desc = {
      .kind = BODY_PROJECTILE,
      .position = origin,
      .velocity = Vec3Scale(direction, weapon->bulletSpeed),
      .radius = weapon->bulletRadius,
      .friction = 0.0f,
      .category = CATEGORY_BULLET,
      .mask = CATEGORY_ARENA | CATEGORY_ENEMY,
  };
  ecs_entity_t bullet = SpawnPhysical(world, &desc);

  ecs_add_id(world, bullet, Bullet);
  ecs_set(world, bullet, Damage, {weapon->damage, weapon->knockback});
  ecs_set(world, bullet, Lifetime, {weapon->bulletLifetime});
  ecs_set(world, bullet, SphereVisual, {.radius = weapon->bulletRadius, .color = COLOR_BULLET});
  SpawnEnd(world, suspended);
  return bullet;
}

Vec3 SpawnPointOnArenaEdge(float height) {
  float radius = ARENA_HALF_EXTENT - ENEMY_SPAWN_MARGIN;
  float angle = RandomAngle();
  return Vec3Make(cosf(angle) * radius, height, sinf(angle) * radius);
}

void SpawnFreshRun(ecs_world_t *world) {
  // Deferring matters twice as much here: a queued delete_with would run after
  // the new arena and player exist and wipe those out too.
  bool suspended = SpawnBegin(world);

  // Every entity the game creates carries Spawned, so one call clears the board
  // and the PhysicsBody destructor takes each rigid body down with it.
  ecs_delete_with(world, Spawned);

  ecs_singleton_set(world, GameState, {.score = 0, .wave = 0, .waveBreak = 0.0f, .over = false});

  SpawnArena(world);
  SpawnPlayer(world, Vec3Make(0.0f, PLAYER_HALF_HEIGHT + PLAYER_RADIUS + 0.05f, 0.0f));

  SpawnEnd(world, suspended);
}
