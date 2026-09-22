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
// the state is checked first. That also makes these nest.
bool SpawnBegin(ecs_world_t *world) {
  if (!ecs_is_deferred(world)) {
    return false;
  }
  ecs_defer_suspend(world);
  return true;
}

void SpawnEnd(ecs_world_t *world, bool suspended) {
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

// Something you can see but never touch. Floors are drawn this way; a single
// slab underneath does the actual holding up.
static ecs_entity_t SpawnDecoration(ecs_world_t *world, Vec3 centre, Vec3 size, Color color) {
  ecs_entity_t entity = ecs_new(world);
  ecs_add_id(world, entity, Spawned);
  ecs_set(world, entity, Position, {centre});
  ecs_set(world, entity, BoxVisual, {.size = size, .color = color});
  return entity;
}

ecs_entity_t SpawnBlock(ecs_world_t *world, Vec3 centre, Vec3 size, Color color, bool solid) {
  bool suspended = SpawnBegin(world);
  ecs_entity_t block;

  if (solid) {
    BodyDesc desc = {
        .kind = BODY_STATIC_BOX,
        .position = centre,
        .halfExtents = Vec3Scale(size, 0.5f),
        .friction = 0.7f,
        .category = CATEGORY_ARENA,
        .mask = CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_BULLET,
    };
    block = SpawnPhysical(world, &desc);
    ecs_add_id(world, block, Wall);
    ecs_set(world, block, BoxVisual, {.size = size, .color = color, .wireframe = true});
  } else {
    block = SpawnDecoration(world, centre, size, color);
  }

  SpawnEnd(world, suspended);
  return block;
}

ecs_entity_t SpawnFloorSlab(ecs_world_t *world, Vec3 centre, Vec3 size) {
  bool suspended = SpawnBegin(world);
  BodyDesc desc = {
      .kind = BODY_STATIC_BOX,
      .position = centre,
      .halfExtents = Vec3Scale(size, 0.5f),
      .friction = 0.7f,
      .category = CATEGORY_ARENA,
      .mask = CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_BULLET,
  };
  ecs_entity_t slab = SpawnPhysical(world, &desc);
  SpawnEnd(world, suspended);
  return slab;
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
  ecs_set(world, player, Powerups, {0});
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

// Everything that separates one enemy from another, in one table.
typedef struct TierStats {
  float radius;
  int health;
  float speed;
  int damage;
  int coins;
  Color color;
  float sight;
  float hearing;
  float memory;
  bool wanders; // Light ones pace about; the heavy ones hold their ground.
} TierStats;

static TierStats StatsFor(EnemyTier tier) {
  switch (tier) {
  case ENEMY_LIGHT:
    // Jumpy. Hears a long way, gives up quickly, never stands still.
    return (TierStats){ENEMY_LIGHT_RADIUS,  ENEMY_LIGHT_HEALTH, ENEMY_LIGHT_SPEED,
                       ENEMY_LIGHT_DAMAGE,  ENEMY_LIGHT_COINS,  COLOR_ENEMY_LIGHT,
                       ENEMY_LIGHT_SIGHT,   ENEMY_LIGHT_HEARING, ENEMY_LIGHT_MEMORY,
                       true};
  case ENEMY_HEAVY:
    // Half deaf and slow, but it does not forget, and it guards its patch.
    return (TierStats){ENEMY_HEAVY_RADIUS,  ENEMY_HEAVY_HEALTH, ENEMY_HEAVY_SPEED,
                       ENEMY_HEAVY_DAMAGE,  ENEMY_HEAVY_COINS,  COLOR_ENEMY_HEAVY,
                       ENEMY_HEAVY_SIGHT,   ENEMY_HEAVY_HEARING, ENEMY_HEAVY_MEMORY,
                       false};
  case ENEMY_MEDIUM:
  default:
    // The middle of everything, and the best pair of eyes.
    return (TierStats){ENEMY_MEDIUM_RADIUS,  ENEMY_MEDIUM_HEALTH, ENEMY_MEDIUM_SPEED,
                       ENEMY_MEDIUM_DAMAGE,  ENEMY_MEDIUM_COINS,  COLOR_ENEMY_MEDIUM,
                       ENEMY_MEDIUM_SIGHT,   ENEMY_MEDIUM_HEARING, ENEMY_MEDIUM_MEMORY,
                       true};
  }
}

ecs_entity_t SpawnEnemy(ecs_world_t *world, Vec3 position, EnemyTier tier) {
  bool suspended = SpawnBegin(world);
  TierStats stats = StatsFor(tier);

  BodyDesc desc = {
      .kind = BODY_CHARACTER,
      .position = position,
      .radius = stats.radius,
      .halfHeight = stats.radius,
      .friction = 0.15f,
      .category = CATEGORY_ENEMY,
      .mask = CATEGORY_ARENA | CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_BULLET,
  };
  ecs_entity_t enemy = SpawnPhysical(world, &desc);

  ecs_add_id(world, enemy, Enemy);
  ecs_set(world, enemy, Health, {stats.health, stats.health});
  ecs_set(world, enemy, MoveSpeed, {stats.speed});
  ecs_set(world, enemy, Loot, {.tier = tier, .coins = stats.coins, .score = SCORE_PER_KILL});
  ecs_set(world, enemy, Bite,
          {
              .interval = ENEMY_BITE_INTERVAL,
              .cooldown = 0.0f,
              .damage = stats.damage,
              .range = PLAYER_RADIUS + stats.radius + 0.35f,
          });
  ecs_set(world, enemy, Senses,
          {.sight = stats.sight, .hearing = stats.hearing, .memory = stats.memory});
  ecs_set(world, enemy, Brain,
          {
              .state = stats.wanders ? AI_PATROL : AI_GUARD,
              .wandering = stats.wanders,
              .wanderTarget = position,
              // Staggered so a room full of them does not think in lockstep.
              .repathTimer = RandomFloat(0.0f, 1.0f),
          });
  ecs_set(world, enemy, CapsuleVisual,
          {.radius = stats.radius, .halfHeight = stats.radius, .color = stats.color});

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

// The exit is a marker, not an obstacle: no body, just somewhere to stand.
ecs_entity_t SpawnExit(ecs_world_t *world, Vec3 position) {
  bool suspended = SpawnBegin(world);
  ecs_entity_t exit = ecs_new(world);
  ecs_add_id(world, exit, Spawned);
  ecs_add_id(world, exit, Exit);
  ecs_set(world, exit, Position, {position});
  ecs_set(world, exit, BoxVisual,
          {.size = Vec3Make(TILE_SIZE * 0.9f, 0.1f, TILE_SIZE * 0.9f), .color = COLOR_EXIT});
  SpawnEnd(world, suspended);
  return exit;
}

// A solid block until the player is carrying a keycard, at which point the
// door system deletes it.
ecs_entity_t SpawnDoor(ecs_world_t *world, Vec3 position) {
  bool suspended = SpawnBegin(world);
  Vec3 size = Vec3Make(TILE_SIZE, ARENA_WALL_HEIGHT, TILE_SIZE);
  BodyDesc desc = {
      .kind = BODY_STATIC_BOX,
      .position = position,
      .halfExtents = Vec3Scale(size, 0.5f),
      .friction = 0.7f,
      .category = CATEGORY_ARENA,
      .mask = CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_BULLET,
  };
  ecs_entity_t door = SpawnPhysical(world, &desc);
  ecs_set(world, door, Door, {.locked = true});
  ecs_set(world, door, BoxVisual, {.size = size, .color = COLOR_DOOR, .wireframe = true});
  SpawnEnd(world, suspended);
  return door;
}

ecs_entity_t SpawnSpawner(ecs_world_t *world, Vec3 position, EnemyTier tier) {
  bool suspended = SpawnBegin(world);
  ecs_entity_t spawner = ecs_new(world);
  ecs_add_id(world, spawner, Spawned);
  ecs_set(world, spawner, Position, {position});
  ecs_set(world, spawner, Spawner,
          {
              .tier = tier,
              .interval = SPAWNER_INTERVAL,
              // Staggered, so a level with four spawners does not empty them
              // all into the room on the same frame.
              .cooldown = RandomFloat(0.5f, SPAWNER_INTERVAL),
              .maxAlive = SPAWNER_MAX_ALIVE,
          });
  ecs_set(world, spawner, BoxVisual,
          {.size = Vec3Make(1.0f, 0.2f, 1.0f), .color = COLOR_SPAWNER, .wireframe = true});
  SpawnEnd(world, suspended);
  return spawner;
}

static Color PickupColor(PickupKind kind) {
  switch (kind) {
  case PICKUP_HEALTH: return COLOR_HEALTH;
  case PICKUP_RAPID_FIRE: return COLOR_RAPID;
  case PICKUP_SHIELD: return COLOR_SHIELD;
  case PICKUP_KEYCARD: return COLOR_KEYCARD;
  case PICKUP_COIN:
  default: return COLOR_COIN;
  }
}

ecs_entity_t SpawnPickup(ecs_world_t *world, Vec3 position, PickupKind kind, int amount) {
  bool suspended = SpawnBegin(world);
  ecs_entity_t pickup = ecs_new(world);
  ecs_add_id(world, pickup, Spawned);
  ecs_set(world, pickup, Position, {position});
  ecs_set(world, pickup, Pickup, {.kind = kind, .amount = amount});

  // Pickups have no rigid body. They are drawn where they are and collected by
  // distance, which keeps them out of the way of the crowd shoving about.
  if (kind == PICKUP_COIN) {
    ecs_set(world, pickup, SphereVisual, {.radius = PICKUP_RADIUS, .color = COLOR_COIN});
  } else {
    ecs_set(world, pickup, BoxVisual,
            {.size = Vec3Make(0.6f, 0.6f, 0.6f), .color = PickupColor(kind), .wireframe = true});
  }
  if (kind == PICKUP_KEYCARD) {
    ecs_add_id(world, pickup, Keycard);
  }

  SpawnEnd(world, suspended);
  return pickup;
}
