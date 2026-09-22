// Which screen the game is on, and what each one's keys do.
//
// The mode lives on GameState and everything else reads it. Only this file
// changes it, so there is one place to look when the game will not start or
// will not unpause.
#include "core/core.h"
#include "game/components/components.h"
#include "game/levels/levels.h"
#include "game/systems/menu_layout.h"
#include "game/systems/systems.h"

static void StartRun(ecs_world_t *world, GameState *state) {
  state->score = 0;
  state->coins = 0;
  state->over = false;
  state->mode = MODE_PLAYING;
  LevelLoad(world, 0);
}

static void EnterSettings(GameState *state) {
  state->modeBeforeSettings = state->mode;
  state->mode = MODE_SETTINGS;
  state->menuIndex = 0;
}

static void ApplyTitleChoice(ecs_world_t *world, GameState *state) {
  switch (state->menuIndex) {
  case 0: StartRun(world, state); break;
  case 1: EnterSettings(state); break;
  case 2: AppRequestQuit(world); break;
  default: break;
  }
}

static void ApplyPauseChoice(GameState *state) {
  switch (state->menuIndex) {
  case 0: state->mode = MODE_PLAYING; break;
  case 1: EnterSettings(state); break;
  case 2: state->mode = MODE_MENU; state->menuIndex = 0; break;
  default: break;
  }
}

static void ApplySettingsChoice(ecs_world_t *world, GameState *state, int direction) {
  Settings *settings = ecs_singleton_get_mut(world, Settings);

  switch (state->menuIndex) {
  case 0: {
    int count = SettingsResolutionCount();
    int next = (settings->resolutionIndex + direction + count) % count;
    SettingsApplyResolution(world, next);
    break;
  }
  case 1:
    SettingsToggleFullscreen(world);
    break;
  case 2:
    SettingsSave(world);
    state->mode = state->modeBeforeSettings;
    state->menuIndex = 0;
    break;
  default:
    break;
  }
}

// Hovering moves the highlight, clicking takes the row. Returns the direction
// a click on a value row means: its left half counts as the left arrow key.
//
// The highlight only follows the mouse when the mouse actually moved, or a
// cursor resting over a row would drag the selection back every time the
// player used the keyboard.
static int ReadMouseMenu(ecs_iter_t *it, GameState *state, bool *clicked) {
  const Input *input = ecs_singleton_get(it->world, Input);
  MenuLayout layout = MenuLayoutFor(state->mode);
  *clicked = false;

  if (layout.count == 0) {
    return 1;
  }

  int hovered = MenuLayoutHit(&layout, input->aimScreen);
  if (hovered >= 0 && input->mouseMoved) {
    state->menuIndex = hovered;
  }
  if (hovered >= 0 && input->click) {
    state->menuIndex = hovered;
    *clicked = true;
    return MenuLayoutSide(&layout, hovered, input->aimScreen);
  }
  return 1;
}

static void UpdateMode(ecs_iter_t *it, GameState *state) {
  const Input *input = ecs_singleton_get(it->world, Input);

  if (state->mode == MODE_PLAYING) {
    state->levelTime += it->delta_time;
    if (input->pause) {
      state->mode = MODE_PAUSED;
      state->menuIndex = 0;
    }
    if (input->restart) {
      LevelLoad(it->world, state->levelIndex);
    }
    return;
  }

  MenuLayout layout = MenuLayoutFor(state->mode);
  if (layout.count > 0) {
    if (input->menuUp) state->menuIndex = (state->menuIndex + layout.count - 1) % layout.count;
    if (input->menuDown) state->menuIndex = (state->menuIndex + 1) % layout.count;
  }

  bool clicked = false;
  int clickDirection = ReadMouseMenu(it, state, &clicked);
  if (clicked) {
    // A click that starts or resumes the game would otherwise still be held
    // down when the weapon system runs, and fire a shot on the way in.
    state->ignoreFireUntilRelease = true;
  }

  switch (state->mode) {
  case MODE_MENU:
    if (input->confirm || clicked) ApplyTitleChoice(it->world, state);
    break;

  case MODE_PAUSED:
    if (input->confirm || clicked) ApplyPauseChoice(state);
    if (input->pause) state->mode = MODE_PLAYING;
    break;

  case MODE_SETTINGS:
    if (input->menuLeft) ApplySettingsChoice(it->world, state, -1);
    if (input->menuRight) ApplySettingsChoice(it->world, state, 1);
    if (input->confirm) ApplySettingsChoice(it->world, state, 1);
    // Clicking the left half of a value row is the left arrow, the right half
    // the right arrow. On Back, either half just means Back.
    if (clicked) ApplySettingsChoice(it->world, state, clickDirection);
    if (input->pause) {
      SettingsSave(it->world);
      state->mode = state->modeBeforeSettings;
      state->menuIndex = 0;
    }
    break;

  case MODE_LEVEL_CLEARED:
    if (input->confirm || input->click) {
      int next = state->levelIndex + 1;
      if (next >= LevelCount()) {
        state->mode = MODE_RUN_COMPLETE;
      } else {
        state->mode = MODE_PLAYING;
        LevelLoad(it->world, next);
      }
    }
    break;

  case MODE_GAME_OVER:
    if (input->confirm || input->click || input->restart) {
      state->over = false;
      state->mode = MODE_PLAYING;
      LevelLoad(it->world, state->levelIndex);
    }
    break;

  case MODE_RUN_COMPLETE:
    if (input->confirm || input->click) {
      state->mode = MODE_MENU;
      state->menuIndex = 0;
    }
    break;

  default:
    break;
  }
}

static void ModeSystem(ecs_iter_t *it) {
  GameState *state = ecs_singleton_get_mut(it->world, GameState);
  const Input *input = ecs_singleton_get(it->world, Input);

  // Once the button that dismissed a menu is let go, shooting is fair again.
  if (!input->fire) {
    state->ignoreFireUntilRelease = false;
  }

  UpdateMode(it, state);

  // Everything except play is a still picture, so the simulation stops. This
  // reads the mode the frame is ending in, not the one it started with, or
  // choosing Start would leave the world frozen for one more frame.
  ClockSetPaused(it->world, state->mode != MODE_PLAYING);
}

void ModeSystemRegister(ecs_world_t *world) {
  // Immediate, because choosing a menu row can load a level, and that deletes
  // and rebuilds the whole board on the spot.
  SystemRegisterImmediate(world, "ModeSystem", PhaseSpawn, ModeSystem, NULL);
}
