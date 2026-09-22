#include "core/settings.h"

#include "raylib.h"

#include <stdio.h>

ECS_COMPONENT_DECLARE(Settings);

#define SETTINGS_FILE "settings.txt"

static const Resolution RESOLUTIONS[] = {
    {1280, 720, "1280 x 720"},
    {1600, 900, "1600 x 900"},
    {1920, 1080, "1920 x 1080"},
    {2560, 1440, "2560 x 1440"},
};

#define RESOLUTION_COUNT ((int)(sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0])))

int SettingsResolutionCount(void) { return RESOLUTION_COUNT; }

Resolution SettingsResolutionAt(int index) {
  if (index < 0) index = 0;
  if (index >= RESOLUTION_COUNT) index = RESOLUTION_COUNT - 1;
  return RESOLUTIONS[index];
}

void SettingsRegister(ecs_world_t *world) {
  ECS_COMPONENT_DEFINE(world, Settings);
  ecs_singleton_set(world, Settings, {.resolutionIndex = 0, .fullscreen = false});
}

// Resizing while fullscreen would fight the monitor, so the stored size is
// only put on the window when there is a window to put it on.
static void ApplyWindowedSize(int index) {
  // A world built without presentation has no window to resize. The settings
  // still track what was chosen, they just have nothing to put it on.
  if (!IsWindowReady() || IsWindowFullscreen()) {
    return;
  }
  Resolution resolution = SettingsResolutionAt(index);
  SetWindowSize(resolution.width, resolution.height);
  // Re-centre, or a window that grew past the screen edge ends up half off it.
  int monitor = GetCurrentMonitor();
  SetWindowPosition((GetMonitorWidth(monitor) - resolution.width) / 2,
                    (GetMonitorHeight(monitor) - resolution.height) / 2);
}

// raylib only offers a toggle, so setting a state means checking it first.
// Real fullscreen and not borderless: on macOS a borderless window the size of
// the screen still sits under the menu bar, which is not what anyone means by
// fullscreen.
static void ApplyFullscreen(bool wanted) {
  if (!IsWindowReady() || IsWindowFullscreen() == wanted) {
    return;
  }
  ToggleFullscreen();
}

void SettingsApply(const ecs_world_t *world) {
  const Settings *settings = ecs_singleton_get(world, Settings);
  ApplyFullscreen(settings->fullscreen);
  ApplyWindowedSize(settings->resolutionIndex);
}

void SettingsApplyResolution(ecs_world_t *world, int index) {
  ecs_singleton_get_mut(world, Settings)->resolutionIndex = index;
  ApplyWindowedSize(index);
}

void SettingsToggleFullscreen(ecs_world_t *world) {
  Settings *settings = ecs_singleton_get_mut(world, Settings);
  settings->fullscreen = !settings->fullscreen;
  ApplyFullscreen(settings->fullscreen);
  if (!settings->fullscreen) {
    ApplyWindowedSize(settings->resolutionIndex);
  }
}

void SettingsSave(const ecs_world_t *world) {
  // A world with no window never applied these to anything, so it has no
  // business overwriting what the player chose the last time they played.
  if (!IsWindowReady()) {
    return;
  }
  const Settings *settings = ecs_singleton_get(world, Settings);
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "resolution %d\nfullscreen %d\n", settings->resolutionIndex,
           settings->fullscreen ? 1 : 0);
  SaveFileText(SETTINGS_FILE, buffer);
}

void SettingsLoad(ecs_world_t *world) {
  if (!IsWindowReady() || !FileExists(SETTINGS_FILE)) {
    return;
  }

  char *text = LoadFileText(SETTINGS_FILE);
  if (text == NULL) {
    return;
  }

  int resolutionIndex = 0;
  int fullscreen = 0;
  if (sscanf(text, "resolution %d fullscreen %d", &resolutionIndex, &fullscreen) == 2) {
    Settings *settings = ecs_singleton_get_mut(world, Settings);
    settings->resolutionIndex =
        (resolutionIndex < 0 || resolutionIndex >= RESOLUTION_COUNT) ? 0 : resolutionIndex;
    settings->fullscreen = fullscreen != 0;
  }
  UnloadFileText(text);
}
