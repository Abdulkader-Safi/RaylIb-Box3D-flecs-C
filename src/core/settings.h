// Window settings the player can change: which resolution, and whether the
// window is borderless fullscreen.
//
// The list is fixed rather than queried from the monitor. A handful of known
// good 16:9 sizes is what a settings menu actually needs, and it keeps the
// game's aspect ratio the one it was designed for.
#ifndef CORE_SETTINGS_H
#define CORE_SETTINGS_H

#include "flecs.h"

#include <stdbool.h>

typedef struct Resolution {
  int width;
  int height;
  const char *label;
} Resolution;

typedef struct Settings {
  int resolutionIndex;
  bool fullscreen;
} Settings;

extern ECS_COMPONENT_DECLARE(Settings);

void SettingsRegister(ecs_world_t *world);

int SettingsResolutionCount(void);
Resolution SettingsResolutionAt(int index);

// Puts the stored settings on the window. Call it once after loading, or the
// saved choices are remembered and never acted on.
void SettingsApply(const ecs_world_t *world);

void SettingsApplyResolution(ecs_world_t *world, int index);
void SettingsToggleFullscreen(ecs_world_t *world);

// Kept on disk between runs, next to the binary.
void SettingsSave(const ecs_world_t *world);
void SettingsLoad(ecs_world_t *world);

#endif // CORE_SETTINGS_H
