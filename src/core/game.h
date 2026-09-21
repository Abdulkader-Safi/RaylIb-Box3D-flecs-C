// Owns every piece of game state and the order things happen in each frame.
#ifndef GAME_H
#define GAME_H

#include "core/input.h"
#include "entities/bullet.h"
#include "entities/enemy.h"
#include "entities/player.h"
#include "physics/physics.h"
#include "render/render.h"
#include "world/arena.h"

typedef enum GameState {
  GAME_PLAYING,
  GAME_OVER,
} GameState;

typedef struct Game {
  Physics physics;
  Arena arena;
  Player player;
  EnemyPool enemies;
  BulletPool bullets;
  Renderer renderer;

  GameState state;
  bool restartRequested; // Set by the game over panel, read next update.
  int score;
  int wave;
  float waveBreakTimer;
} Game;

void GameInit(Game *game);
void GameShutdown(Game *game);

// Reads this frame's input, then simulates it.
void GameUpdate(Game *game, float dt);

// The simulation on its own, driven by whatever input you hand it. Touches no
// window or GPU, so tests can run it headless.
void GameSimulate(Game *game, const InputState *input, float dt);

void GameDraw(Game *game);

#endif // GAME_H
