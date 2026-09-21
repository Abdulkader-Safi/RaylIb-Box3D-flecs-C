// Everything drawn flat on top of the world: the in-game readout, and every
// menu screen. Drawing only. Which screen is up, and what a key does on it,
// belongs to mode_system.c.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/levels/levels.h"
#include "game/systems/systems.h"

#define ROW_HEIGHT 38

static void DrawDim(void) {
  GfxDrawRect(0, 0, GfxScreenWidth(), GfxScreenHeight(), COLOR_MENU_DIM);
}

// One row of a menu. The highlighted row gets a marker rather than a colour
// change alone, so it reads without relying on the player seeing the tint.
static void DrawMenuRow(const char *label, int index, int selected, int y) {
  bool active = index == selected;
  const char *text = GfxFormat("%s %s", active ? ">" : " ", label);
  GfxDrawTextCentered(text, y, 28, active ? COLOR_BULLET : GRAY);
}

static void DrawTitle(const GameState *state) {
  DrawDim();
  GfxDrawTextCentered("TWIN STICK", 110, 72, RAYWHITE);
  GfxDrawTextCentered("raylib  .  Box3D  .  flecs", 190, 20, GRAY);

  int y = GfxScreenHeight() / 2 - ROW_HEIGHT;
  DrawMenuRow("Start run", 0, state->menuIndex, y);
  DrawMenuRow("Settings", 1, state->menuIndex, y + ROW_HEIGHT);
  DrawMenuRow("Quit", 2, state->menuIndex, y + ROW_HEIGHT * 2);

  GfxDrawTextCentered("W and S to move   .   Enter to choose", GfxScreenHeight() - 70, 18, GRAY);
}

static void DrawPause(const GameState *state) {
  DrawDim();
  GfxDrawTextCentered("PAUSED", 140, 56, RAYWHITE);

  int y = GfxScreenHeight() / 2 - ROW_HEIGHT;
  DrawMenuRow("Resume", 0, state->menuIndex, y);
  DrawMenuRow("Settings", 1, state->menuIndex, y + ROW_HEIGHT);
  DrawMenuRow("Quit to title", 2, state->menuIndex, y + ROW_HEIGHT * 2);

  GfxDrawTextCentered("Escape to resume", GfxScreenHeight() - 70, 18, GRAY);
}

static void DrawSettings(ecs_world_t *world, const GameState *state) {
  const Settings *settings = ecs_singleton_get(world, Settings);
  Resolution resolution = SettingsResolutionAt(settings->resolutionIndex);

  DrawDim();
  GfxDrawTextCentered("SETTINGS", 140, 56, RAYWHITE);

  int y = GfxScreenHeight() / 2 - ROW_HEIGHT;
  if (SettingsWindowIsAdjustable()) {
    DrawMenuRow(GfxFormat("Resolution    < %s >", resolution.label), 0, state->menuIndex, y);
    DrawMenuRow(GfxFormat("Fullscreen    < %s >", settings->fullscreen ? "on" : "off"), 1,
                state->menuIndex, y + ROW_HEIGHT);
  } else {
    // In a browser the page owns the window, so these would be lying.
    DrawMenuRow("Resolution    set by the page", 0, state->menuIndex, y);
    DrawMenuRow("Fullscreen    use the browser", 1, state->menuIndex, y + ROW_HEIGHT);
  }
  DrawMenuRow("Back", 2, state->menuIndex, y + ROW_HEIGHT * 2);

  GfxDrawTextCentered("A and D to change   .   Escape to go back", GfxScreenHeight() - 70, 18,
                      GRAY);
}

static void DrawLevelCleared(const GameState *state) {
  const Level *level = LevelAt(state->levelIndex);
  DrawDim();
  GfxDrawTextCentered("LEVEL CLEARED", 150, 52, COLOR_EXIT);
  GfxDrawTextCentered(level->name, 215, 26, RAYWHITE);
  GfxDrawTextCentered(GfxFormat("Time %.1fs    Coins %d    Score %d", state->levelTime,
                                state->coins, state->score),
                      GfxScreenHeight() / 2, 24, GRAY);
  GfxDrawTextCentered("Enter for the next level", GfxScreenHeight() / 2 + 60, 24, COLOR_BULLET);
}

static void DrawGameOver(const GameState *state) {
  DrawDim();
  GfxDrawTextCentered("YOU DIED", 150, 56, COLOR_ENEMY);
  GfxDrawTextCentered(GfxFormat("Score %d    Coins %d", state->score, state->coins),
                      GfxScreenHeight() / 2, 24, RAYWHITE);
  GfxDrawTextCentered("Enter to retry this level", GfxScreenHeight() / 2 + 60, 24, COLOR_BULLET);
}

static void DrawRunComplete(const GameState *state) {
  DrawDim();
  GfxDrawTextCentered("RUN COMPLETE", 150, 56, COLOR_EXIT);
  GfxDrawTextCentered(GfxFormat("Final score %d    Coins %d", state->score, state->coins),
                      GfxScreenHeight() / 2, 26, RAYWHITE);
  GfxDrawTextCentered("Enter to return to the title", GfxScreenHeight() / 2 + 60, 24,
                      COLOR_BULLET);
}

static void DrawHealthBar(const PlayerTracker *tracker, const Powerups *powerups) {
  const int x = 24;
  const int y = 24;
  const int width = 280;
  const int height = 22;

  float fraction =
      tracker->maxHealth > 0 ? (float)tracker->health / (float)tracker->maxHealth : 0.0f;
  bool shielded = powerups != NULL && powerups->shield > 0.0f;

  GfxDrawRect(x - 2, y - 2, width + 4, height + 4, (Color){0, 0, 0, 150});
  GfxDrawRect(x, y, (int)(width * fraction), height,
              shielded ? COLOR_SHIELD : (fraction < 0.35f ? COLOR_ENEMY : COLOR_GOOD));
  GfxDrawRectLines(x, y, width, height, RAYWHITE);
  GfxDrawText(GfxFormat("%d / %d", tracker->health, tracker->maxHealth), x + 8, y + 3, 16,
              RAYWHITE);
}

static void DrawCrosshair(const Input *input) {
  Vec2 at = input->aimScreen;
  GfxDrawCircleLines(at, 10.0f, RAYWHITE);
  GfxDrawLine2D(Vec2Make(at.x - 16, at.y), Vec2Make(at.x - 4, at.y), RAYWHITE);
  GfxDrawLine2D(Vec2Make(at.x + 4, at.y), Vec2Make(at.x + 16, at.y), RAYWHITE);
  GfxDrawLine2D(Vec2Make(at.x, at.y - 16), Vec2Make(at.x, at.y - 4), RAYWHITE);
  GfxDrawLine2D(Vec2Make(at.x, at.y + 4), Vec2Make(at.x, at.y + 16), RAYWHITE);
}

static void DrawPlayfieldHud(ecs_world_t *world, const GameState *state) {
  const PlayerTracker *tracker = ecs_singleton_get(world, PlayerTracker);
  const Powerups *powerups =
      tracker->entity != 0 ? ecs_get(world, tracker->entity, Powerups) : NULL;
  const Level *level = LevelAt(state->levelIndex);

  DrawHealthBar(tracker, powerups);
  GfxDrawText(GfxFormat("SCORE  %d", state->score), 24, 60, 24, RAYWHITE);
  GfxDrawText(GfxFormat("COINS  %d", state->coins), 24, 92, 20, COLOR_COIN);
  GfxDrawText(GfxFormat("ENEMIES  %d", ecs_count(world, Enemy)), 24, 118, 20, GRAY);

  // Top right: where you are and what you still need.
  const char *heading = GfxFormat("%d / %d  %s", state->levelIndex + 1, LevelCount(), level->name);
  GfxDrawText(heading, GfxScreenWidth() - GfxMeasureText(heading, 22) - 24, 24, 22, RAYWHITE);

  const char *objective = state->hasKeycard ? "Keycard taken. Find the exit."
                                            : "Find the keycard.";
  GfxDrawText(objective, GfxScreenWidth() - GfxMeasureText(objective, 18) - 24, 54, 18,
              state->hasKeycard ? COLOR_EXIT : COLOR_KEYCARD);

  if (powerups != NULL) {
    int y = 150;
    if (powerups->rapidFire > 0.0f) {
      GfxDrawText(GfxFormat("RAPID FIRE  %.1fs", powerups->rapidFire), 24, y, 20, COLOR_RAPID);
      y += 26;
    }
    if (powerups->shield > 0.0f) {
      GfxDrawText(GfxFormat("SHIELD  %.1fs", powerups->shield), 24, y, 20, COLOR_SHIELD);
    }
  }

  GfxDrawText("WASD move   .   Mouse aim   .   Click fire   .   Esc pause   .   R restart level",
              24, GfxScreenHeight() - 34, 18, GRAY);
}

static void HudSystem(ecs_iter_t *it) {
  const GameState *state = ecs_singleton_get(it->world, GameState);
  const Input *input = ecs_singleton_get(it->world, Input);

  // The world is drawn underneath in every mode, so a menu sits over the game
  // rather than replacing it.
  if (state->mode != MODE_MENU) {
    DrawPlayfieldHud(it->world, state);
  }

  switch (state->mode) {
  case MODE_MENU: DrawTitle(state); break;
  case MODE_PAUSED: DrawPause(state); break;
  case MODE_SETTINGS: DrawSettings(it->world, state); break;
  case MODE_LEVEL_CLEARED: DrawLevelCleared(state); break;
  case MODE_GAME_OVER: DrawGameOver(state); break;
  case MODE_RUN_COMPLETE: DrawRunComplete(state); break;
  case MODE_PLAYING: DrawCrosshair(input); break;
  }
}

void HudSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, HudSystem, PhaseDrawUI, 0);
}
