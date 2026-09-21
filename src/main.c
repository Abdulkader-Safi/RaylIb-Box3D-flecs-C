// Entry point: open a window, run the loop, tear everything down.
// All the game lives in src/core/game.c.
#include "core/config.h"
#include "core/game.h"
#include "raylib.h"

#include <stdlib.h>

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(60);
  SetExitKey(KEY_ESCAPE);

  // The Game struct holds the entity pools, so it goes on the heap rather than
  // the stack.
  Game *game = malloc(sizeof(Game));
  if (game == NULL) {
    CloseWindow();
    return 1;
  }
  GameInit(game);

  while (!WindowShouldClose()) {
    GameUpdate(game, GetFrameTime());
    GameDraw(game);
  }

  GameShutdown(game);
  free(game);
  CloseWindow();
  return 0;
}
