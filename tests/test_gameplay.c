// Headless check of the loop that is easy to break: fire, hit, score, clean up.
// Build and run it with:  cmake --build build --target game_tests && ./build/bin/game_tests
#include "core/game.h"
#include "raymath.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define STEP (1.0f / 60.0f)

static const Enemy *FirstLivingEnemy(const EnemyPool *pool) {
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    if (pool->items[i].alive) {
      return pool->items + i;
    }
  }
  return NULL;
}

// One frame of a player standing still and shooting at the nearest enemy.
static InputState AimAtNearestEnemy(const Game *game) {
  InputState input = {0};
  const Enemy *target = FirstLivingEnemy(&game->enemies);
  Vector3 playerPosition = PlayerPosition(&game->player);
  input.aimPoint = target != NULL ? EnemyPosition(target)
                                  : Vector3Add(playerPosition, (Vector3){0.0f, 0.0f, 1.0f});
  input.firing = target != NULL;
  return input;
}

int main(void) {
  SetRandomSeed(20240921); // Spawn positions must repeat run to run.

  Game *game = malloc(sizeof(Game));
  assert(game != NULL);
  GameInit(game);

  assert(game->wave == 1);
  assert(game->enemies.aliveCount == WAVE_FIRST_COUNT);
  assert(game->score == 0);
  assert(game->player.health == PLAYER_MAX_HEALTH);

  int startingEnemies = game->enemies.aliveCount;
  for (int frame = 0; frame < 600; ++frame) {
    InputState input = AimAtNearestEnemy(game);
    GameSimulate(game, &input, STEP);
  }

  printf("after 10s: score %d, enemies %d, health %d, bullets %d\n", game->score,
         game->enemies.aliveCount, game->player.health, game->bullets.aliveCount);

  // Bullets that reach an enemy have to kill it and pay out.
  assert(game->score >= SCORE_PER_KILL);
  assert(game->score % SCORE_PER_KILL == 0);
  assert(game->score >= startingEnemies * SCORE_PER_KILL);

  // Enemies that reach a player standing still have to hurt it.
  assert(game->player.health < PLAYER_MAX_HEALTH);

  // Spent and expired bullets must give their slots back.
  InputState idle = {0};
  for (int frame = 0; frame < 180; ++frame) {
    GameSimulate(game, &idle, STEP);
  }
  assert(game->bullets.aliveCount == 0);

  // Killing a wave has to start the next one.
  EnemyPoolClear(&game->enemies);
  int waveBefore = game->wave;
  for (int frame = 0; frame < 300; ++frame) {
    GameSimulate(game, &idle, STEP);
  }
  assert(game->wave > waveBefore);
  assert(game->enemies.aliveCount > 0);

  // A dead player ends the run, and restarting puts everything back.
  PlayerDamage(&game->player, PLAYER_MAX_HEALTH);
  GameSimulate(game, &idle, STEP);
  assert(game->state == GAME_OVER);

  InputState restart = {0};
  restart.restart = true;
  GameSimulate(game, &restart, STEP);
  assert(game->state == GAME_PLAYING);
  assert(game->wave == 1);
  assert(game->score == 0);
  assert(game->player.health == PLAYER_MAX_HEALTH);

  GameShutdown(game);
  free(game);
  printf("all gameplay checks passed\n");
  return 0;
}
