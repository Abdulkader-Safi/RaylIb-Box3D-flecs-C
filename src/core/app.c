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

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>

// On the web the browser owns the loop. A frame is handed over as a callback
// rather than run inside a while, because blocking here would freeze the page.
// The callback takes no argument, so the world is reached through a file
// static; there is one world per process either way.
static ecs_world_t *g_webWorld;

static void AppWebFrame(void) {
  ecs_progress(g_webWorld, GetFrameTime());
  if (ecs_singleton_get(g_webWorld, Input)->quit) {
    emscripten_cancel_main_loop();
  }
}
#endif

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
#if defined(__EMSCRIPTEN__)
  // No FLAG_WINDOW_RESIZABLE on the web, deliberately.
  //
  // With it set, raylib resizes the canvas itself through
  // emscripten_set_canvas_element_size. That moves the rendering but not
  // Emscripten's GLFW, which keeps scaling mouse coordinates against the size
  // it cached at startup. Drawing then follows the window while the pointer
  // reports a fraction of where it really is, so aiming drifts and clicks land
  // nowhere near the button they appear to be over.
  //
  // Holding the canvas at its startup size keeps both halves agreeing. The
  // page scales the element with CSS and GLFW maps the pointer back into it,
  // which is a plain scale it gets right.
  SetConfigFlags(FLAG_MSAA_4X_HINT);
#else
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
#endif
  InitWindow(config->width, config->height, config->title);
#if !defined(__EMSCRIPTEN__)
  // On the web the browser paces the frames, so raylib must not also throttle
  // them. On desktop nothing else will.
  SetTargetFPS(config->targetFps > 0 ? config->targetFps : 60);
#endif
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

#if defined(__EMSCRIPTEN__)
  g_webWorld = world;
  // Zero means one frame per browser repaint. This call does not return, so
  // the teardown below only ever runs on desktop. Closing the tab frees the
  // whole heap anyway.
  emscripten_set_main_loop(AppWebFrame, 0, 1);
#else
  while (!ecs_singleton_get(world, Input)->quit) {
    ecs_progress(world, GetFrameTime());
  }
#endif

  SettingsSave(world);
  AppWorldDestroy(world);
  CloseWindow();
  return 0;
}

void AppRequestQuit(ecs_world_t *world) { ecs_singleton_get_mut(world, Input)->quit = true; }
