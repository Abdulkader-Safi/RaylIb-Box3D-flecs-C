#include "render/render.h"

#include "core/config.h"
#include "core/game.h"
#include "raymath.h"

static const Color COLOR_FLOOR = {26, 30, 42, 255};
static const Color COLOR_GRID = {44, 52, 70, 255};
static const Color COLOR_WALL = {58, 68, 92, 255};
static const Color COLOR_PLAYER = {90, 200, 255, 255};
static const Color COLOR_ENEMY = {235, 84, 84, 255};
static const Color COLOR_BULLET = {255, 214, 102, 255};
static const Color COLOR_AIM = {255, 255, 255, 110};

static Vector3 CameraOffset(void) {
  return (Vector3){0.0f, CAMERA_HEIGHT, CAMERA_BACK_OFFSET};
}

void RendererInit(Renderer *renderer, Vector3 target) {
  renderer->camera.position = Vector3Add(target, CameraOffset());
  renderer->camera.target = target;
  renderer->camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  renderer->camera.fovy = 50.0f;
  renderer->camera.projection = CAMERA_PERSPECTIVE;
}

void RendererFollow(Renderer *renderer, Vector3 target, float dt) {
  // Frame rate independent easing: the same fraction of the remaining gap is
  // closed per second whatever the frame time.
  float blend = 1.0f - expf(-CAMERA_FOLLOW_RATE * dt);
  renderer->camera.target = Vector3Lerp(renderer->camera.target, target, blend);
  renderer->camera.position = Vector3Add(renderer->camera.target, CameraOffset());
}

static void DrawArena(const Arena *arena) {
  float half = arena->halfExtent;
  // The slab is dropped 2cm so the grid at y = 0 never z-fights with its top.
  DrawCubeV((Vector3){0.0f, -0.52f, 0.0f}, (Vector3){half * 2.0f, 1.0f, half * 2.0f}, COLOR_FLOOR);
  DrawGrid((int)(half * 2.0f), 1.0f);

  float wallY = ARENA_WALL_HEIGHT * 0.5f;
  float offset = half + ARENA_WALL_THICKNESS * 0.5f;
  Vector3 spans[4] = {
      {half * 2.0f, ARENA_WALL_HEIGHT, ARENA_WALL_THICKNESS},
      {half * 2.0f, ARENA_WALL_HEIGHT, ARENA_WALL_THICKNESS},
      {ARENA_WALL_THICKNESS, ARENA_WALL_HEIGHT, half * 2.0f},
      {ARENA_WALL_THICKNESS, ARENA_WALL_HEIGHT, half * 2.0f},
  };
  Vector3 centers[4] = {
      {0.0f, wallY, -offset},
      {0.0f, wallY, offset},
      {-offset, wallY, 0.0f},
      {offset, wallY, 0.0f},
  };
  for (int i = 0; i < 4; ++i) {
    DrawCubeV(centers[i], spans[i], COLOR_WALL);
    DrawCubeWiresV(centers[i], spans[i], COLOR_GRID);
  }
}

static void DrawCharacter(Vector3 center, float radius, float halfHeight, Color color) {
  Vector3 bottom = {center.x, center.y - halfHeight, center.z};
  Vector3 top = {center.x, center.y + halfHeight, center.z};
  DrawCapsule(bottom, top, radius, 12, 8, color);
}

static void DrawPlayer(const Player *player) {
  Vector3 position = PlayerPosition(player);
  DrawCharacter(position, PLAYER_RADIUS, PLAYER_HALF_HEIGHT, COLOR_PLAYER);

  // Muzzle stub so the aim direction reads at a glance.
  Vector3 barrelStart = Vector3Add(position, Vector3Scale(player->aimDirection, PLAYER_RADIUS));
  Vector3 barrelEnd = Vector3Add(position, Vector3Scale(player->aimDirection, PLAYER_RADIUS + 0.7f));
  DrawCapsule(barrelStart, barrelEnd, 0.12f, 8, 4, COLOR_BULLET);

  // Aim line along the floor, stopping at the wall.
  Vector3 floorStart = {position.x, 0.02f, position.z};
  Vector3 floorEnd = Vector3Add(floorStart, Vector3Scale(player->aimDirection, 14.0f));
  DrawLine3D(floorStart, floorEnd, COLOR_AIM);
}

static void DrawEnemies(const EnemyPool *pool) {
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    const Enemy *enemy = pool->items + i;
    if (!enemy->alive) {
      continue;
    }
    // Fades toward white as it takes damage.
    float wear = 1.0f - (float)enemy->health / (float)ENEMY_MAX_HEALTH;
    Color color = COLOR_ENEMY;
    color.g = (unsigned char)(color.g + (unsigned char)(wear * 120.0f));
    color.b = (unsigned char)(color.b + (unsigned char)(wear * 120.0f));
    DrawCharacter(EnemyPosition(enemy), ENEMY_RADIUS, ENEMY_HALF_HEIGHT, color);
  }
}

static void DrawBullets(const BulletPool *pool) {
  for (int i = 0; i < MAX_BULLETS; ++i) {
    const Bullet *bullet = pool->items + i;
    if (!bullet->alive) {
      continue;
    }
    DrawSphere(PhysicsBodyPosition(bullet->bodyId), BULLET_RADIUS, COLOR_BULLET);
  }
}

void RendererDrawScene(const Renderer *renderer, const struct Game *game) {
  (void)renderer;
  DrawArena(&game->arena);
  DrawEnemies(&game->enemies);
  DrawBullets(&game->bullets);
  if (PlayerIsAlive(&game->player)) {
    DrawPlayer(&game->player);
  }
}
