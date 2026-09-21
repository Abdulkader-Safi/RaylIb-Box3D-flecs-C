// The window, the world and the loop.
//
// A game is one function: hand AppRun something that registers your components
// and systems, and the framework does the rest.
//
//   int main(void) {
//     AppConfig config = {.width = 1280, .height = 720, .title = "My game"};
//     return AppRun(&config, GameRegister);
//   }
#ifndef CORE_APP_H
#define CORE_APP_H

#include "core/math.h"
#include "flecs.h"

typedef struct AppConfig {
  int width;
  int height;
  const char *title;
  int targetFps;
  Color clearColor;
  unsigned int randomSeed; // Zero leaves raylib's own seeding alone.
} AppConfig;

// Called once, after the framework is up and before the first frame. Register
// your components first, then your systems, then seed the world.
typedef void (*AppModuleFn)(ecs_world_t *world);

int AppRun(const AppConfig *config, AppModuleFn registerGame);

// Ends the loop after the current frame.
void AppRequestQuit(ecs_world_t *world);

#endif // CORE_APP_H
