// Which screen the game is on, and what each one's keys do.
//
// The mode lives on GameState and everything else reads it. Only this file
// changes it, so there is one place to look when the game will not start or
// will not unpause.
#include "core/core.h"
#include "game/components/components.h"
#include "game/levels/levels.h"
#include "game/systems/systems.h"

// Rows on the title screen and the pause screen, in the order they are drawn.
#define MENU_ROWS_TITLE 3
#define MENU_ROWS_PAUSE 3

static int MenuRowCount(GameMode mode) {
  switch (mode) {
  case MODE_MENU: return MENU_ROWS_TITLE;
  case MODE_PAUSED: return MENU_ROWS_PAUSE;
  case MODE_SETTINGS: return 3;
  default: return 0;
  }
}

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

static void ApplyPauseChoice(ecs_world_t *world, GameState *state) {
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

  int rows = MenuRowCount(state->mode);
  if (rows > 0) {
    if (input->menuUp) state->menuIndex = (state->menuIndex + rows - 1) % rows;
    if (input->menuDown) state->menuIndex = (state->menuIndex + 1) % rows;
  }

  switch (state->mode) {
  case MODE_MENU:
    if (input->confirm) ApplyTitleChoice(it->world, state);
    break;

  case MODE_PAUSED:
    if (input->confirm) ApplyPauseChoice(it->world, state);
    if (input->pause) state->mode = MODE_PLAYING;
    break;

  case MODE_SETTINGS:
    if (input->menuLeft) ApplySettingsChoice(it->world, state, -1);
    if (input->menuRight) ApplySettingsChoice(it->world, state, 1);
    if (input->confirm) ApplySettingsChoice(it->world, state, 1);
    if (input->pause) {
      SettingsSave(it->world);
      state->mode = state->modeBeforeSettings;
      state->menuIndex = 0;
    }
    break;

  case MODE_LEVEL_CLEARED:
    if (input->confirm) {
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
    if (input->confirm || input->restart) {
      state->over = false;
      state->mode = MODE_PLAYING;
      LevelLoad(it->world, state->levelIndex);
    }
    break;

  case MODE_RUN_COMPLETE:
    if (input->confirm) {
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
