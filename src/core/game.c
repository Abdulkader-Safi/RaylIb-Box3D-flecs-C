#include "core/game.h"

#include "core/config.h"
#include "raymath.h"
#include "render/hud.h"

#include <string.h>

static void StartNextWave(Game *game) {
  game->wave += 1;
  int count = WAVE_FIRST_COUNT + (game->wave - 1) * WAVE_COUNT_STEP;
  if (count > MAX_ENEMIES) {
    count = MAX_ENEMIES;
  }

  float spawnHeight = ENEMY_HALF_HEIGHT + ENEMY_RADIUS + 0.1f;
  for (int i = 0; i < count; ++i) {
    EnemySpawn(&game->enemies, &game->physics, ArenaEdgeSpawnPoint(&game->arena, spawnHeight));
  }
  game->waveBreakTimer = WAVE_BREAK_SECONDS;
}

// One half of a contact, seen from the bullet's side. The other ordering is
// handled by the second call in OnContact.
static void ResolveBulletHit(Game *game, const Entity *bulletEntity, const Entity *other) {
  Bullet *bullet = game->bullets.items + bulletEntity->index;
  if (!bullet->alive || bullet->timeToLive <= 0.0f) {
    return; // Already spent this frame, a second hit costs nothing.
  }

  if (other != NULL && other->kind == ENTITY_ENEMY) {
    Enemy *enemy = game->enemies.items + other->index;
    Vector3 knockback = Vector3Scale(Vector3Normalize(BulletVelocity(bullet)), BULLET_KNOCKBACK);
    EnemyDamage(enemy, BULLET_DAMAGE, knockback);
  }
  BulletMarkSpent(bullet);
}

static void OnContact(void *ownerA, void *ownerB, void *context) {
  Game *game = (Game *)context;
  const Entity *a = (const Entity *)ownerA;
  const Entity *b = (const Entity *)ownerB;

  if (a != NULL && a->kind == ENTITY_BULLET) {
    ResolveBulletHit(game, a, b);
  }
  if (b != NULL && b->kind == ENTITY_BULLET) {
    ResolveBulletHit(game, b, a);
  }
}

void GameInit(Game *game) {
  memset(game, 0, sizeof(*game));

  PhysicsInit(&game->physics);
  ArenaBuild(&game->arena, &game->physics);
  EnemyPoolInit(&game->enemies);
  BulletPoolInit(&game->bullets);

  Vector3 start = {0.0f, PLAYER_HALF_HEIGHT + PLAYER_RADIUS + 0.05f, 0.0f};
  PlayerSpawn(&game->player, &game->physics, start);
  RendererInit(&game->renderer, start);

  game->state = GAME_PLAYING;
  game->score = 0;
  game->wave = 0;
  StartNextWave(game);
}

void GameShutdown(Game *game) {
  // Destroying the world releases every body, so the pools only need their
  // bookkeeping cleared.
  PhysicsShutdown(&game->physics);
  memset(&game->enemies, 0, sizeof(game->enemies));
  memset(&game->bullets, 0, sizeof(game->bullets));
}

void GameUpdate(Game *game, float dt) {
  InputState input = InputRead(game->renderer.camera, PlayerPosition(&game->player));
  GameSimulate(game, &input, dt);
}

void GameSimulate(Game *game, const InputState *input, float dt) {
  // Restart works at any time, which is what the on screen hint promises.
  if (input->restart || game->restartRequested) {
    GameShutdown(game);
    GameInit(game);
    return;
  }
  if (game->state == GAME_OVER) {
    return;
  }

  PlayerUpdate(&game->player, input, dt);
  if (input->firing && PlayerTryFire(&game->player)) {
    BulletFire(&game->bullets, &game->physics, PlayerMuzzlePoint(&game->player),
               game->player.aimDirection);
  }

  EnemyPoolUpdate(&game->enemies, &game->player, dt);

  // Hits are recorded during the step and acted on here, so no body is ever
  // destroyed while Box3D is still reading it.
  PhysicsStep(&game->physics, dt, OnContact, game);

  BulletPoolUpdate(&game->bullets, dt);
  game->score += EnemyPoolReap(&game->enemies) * SCORE_PER_KILL;

  if (game->enemies.aliveCount == 0) {
    game->waveBreakTimer -= dt;
    if (game->waveBreakTimer <= 0.0f) {
      StartNextWave(game);
    }
  }

  RendererFollow(&game->renderer, PlayerPosition(&game->player), dt);

  if (!PlayerIsAlive(&game->player)) {
    game->state = GAME_OVER;
  }
}

void GameDraw(Game *game) {
  BeginDrawing();
  ClearBackground((Color){14, 16, 24, 255});

  BeginMode3D(game->renderer.camera);
  RendererDrawScene(&game->renderer, game);
  EndMode3D();

  game->restartRequested = HudDraw(game);
  EndDrawing();
}
