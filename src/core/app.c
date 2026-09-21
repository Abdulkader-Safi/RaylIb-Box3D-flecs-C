#include "core/app.h"

#include "core/components.h"
#include "core/gfx.h"
#include "core/input.h"
#include "core/lifetime.h"
#include "core/phases.h"
#include "core/physics.h"
#include "raylib.h"

int AppRun(const AppConfig *config, AppModuleFn registerGame) {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(config->width, config->height, config->title);
  SetTargetFPS(config->targetFps > 0 ? config->targetFps : 60);
  SetExitKey(KEY_ESCAPE);
  if (config->randomSeed != 0) {
    RandomSeed(config->randomSeed);
  }

  ecs_world_t *world = ecs_init();

  // Order matters here and nowhere else: phases before the systems that pick
  // one, components before the systems that query them.
  PhasesRegister(world);
  CoreComponentsRegister(world);
  InputRegister(world);
  PhysicsRegister(world);
  GfxRegister(world, config->clearColor);
  LifetimeRegister(world);

  registerGame(world);

  while (!ecs_singleton_get(world, Input)->quit) {
    ecs_progress(world, GetFrameTime());
  }

  // Tearing down the world deletes every entity, which runs the PhysicsBody
  // destructor on each one. The physics world has to outlive that, so it is
  // destroyed second.
  ecs_fini(world);
  PhysicsShutdown();
  CloseWindow();
  return 0;
}

void AppRequestQuit(ecs_world_t *world) { ecs_singleton_ensure(world, Input)->quit = true; }
