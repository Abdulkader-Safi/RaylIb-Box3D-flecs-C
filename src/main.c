// The whole entry point. Configure a window, hand the framework the game's
// register function, and it owns the rest.
#include "core/core.h"
#include "game/config.h"
#include "game/game.h"

int main(void) {
  AppConfig config = {
      .width = SCREEN_WIDTH,
      .height = SCREEN_HEIGHT,
      .title = WINDOW_TITLE,
      .targetFps = 60,
      .clearColor = COLOR_BACKGROUND,
  };
  return AppRun(&config, GameRegister);
}
