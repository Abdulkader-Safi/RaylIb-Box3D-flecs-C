#include "core/app.h"

#include "core/clock.h"
#include "core/components.h"
#include "core/gfx.h"
#include "core/input.h"
#include "core/lifetime.h"
#include "core/phases.h"
#include "core/physics.h"
#include "core/settings.h"
#include "raylib.h"

ecs_world_t *AppWorldCreate(bool withPresentation) {
  ecs_world_t *world = ecs_init();

  // Order matters here and nowhere else: phases before the systems that pick
  // one, components before the systems that query them.
  PhasesRegister(world);
  CoreComponentsRegister(world);
  ClockRegister(world);
  SettingsRegister(world);
  PhysicsRegister(world);
  LifetimeRegister(world);

  if (withPresentation) {
    InputRegister(world);
    GfxRegister(world);
  }
  return world;
}

void AppWorldDestroy(ecs_world_t *world) {
  // Tearing down the world deletes every entity, which runs the PhysicsBody
  // destructor on each one. The physics world has to outlive that, so it is
  // destroyed second.
  ecs_fini(world);
  PhysicsShutdown();
}

int AppRun(const AppConfig *config, AppModuleFn registerGame) {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(config->width, config->height, config->title);
  SetTargetFPS(config->targetFps > 0 ? config->targetFps : 60);
  // Escape belongs to the game, which uses it to pause. Quitting goes through
  // the menu, or through the window's own close button.
  SetExitKey(KEY_NULL);
  if (config->randomSeed != 0) {
    RandomSeed(config->randomSeed);
  }

  GfxSetClearColor(config->clearColor);
  ecs_world_t *world = AppWorldCreate(true);
  SettingsLoad(world);
  registerGame(world);

  while (!ecs_singleton_get(world, Input)->quit) {
    ecs_progress(world, GetFrameTime());
  }

  SettingsSave(world);
  AppWorldDestroy(world);
  CloseWindow();
  return 0;
}

void AppRequestQuit(ecs_world_t *world) { ecs_singleton_get_mut(world, Input)->quit = true; }
