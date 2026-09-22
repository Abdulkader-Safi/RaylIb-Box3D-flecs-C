// Everything drawn flat on top of the world: the in-game readout, and every
// menu screen. Drawing only. Which screen is up, and what a key does on it,
// belongs to mode_system.c.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/levels/levels.h"
#include "game/systems/menu_layout.h"

#include <stdio.h>
#include "game/systems/systems.h"


static void DrawDim(void) {
  GfxDrawRect(0, 0, GfxScreenWidth(), GfxScreenHeight(), COLOR_MENU_DIM);
}

// One row of a menu, drawn inside the rectangle the mouse is tested against.
// The highlighted row gets a band and a marker, not just a colour, so it reads
// without relying on the player picking up the tint.
static void DrawMenuRow(const MenuLayout *layout, int index, int selected, const char *label) {
  if (index >= layout->count) {
    return;
  }
  Rect row = layout->rows[index];
  bool active = index == selected;

  if (active) {
    GfxDrawRect((int)row.x, (int)row.y, (int)row.width, (int)row.height, (Color){255, 255, 255, 18});
  }

  // Built here rather than by formatting one result into another. The label
  // is often itself a formatted string, and this row is the last place that
  // should care where it came from.
  char text[160];
  snprintf(text, sizeof(text), "%s %s", active ? ">" : " ", label);
  // Centred inside the row rather than on the screen, so the text always sits
  // in the box that responds to the click.
  int textX = (int)(row.x + (row.width - (float)GfxMeasureText(text, MENU_TEXT_SIZE)) * 0.5f);
  int textY = (int)(row.y + (row.height - (float)MENU_TEXT_SIZE) * 0.5f);
  GfxDrawText(text, textX, textY, MENU_TEXT_SIZE, active ? COLOR_BULLET : GRAY);
}

static void DrawTitle(const GameState *state) {
  DrawDim();
  GfxDrawTextCentered("TWIN STICK", 110, 72, RAYWHITE);
  GfxDrawTextCentered("raylib  .  Box3D  .  flecs", 190, 20, GRAY);

  MenuLayout layout = MenuLayoutFor(MODE_MENU);
  DrawMenuRow(&layout, 0, state->menuIndex, "Start run");
  DrawMenuRow(&layout, 1, state->menuIndex, "Settings");
  DrawMenuRow(&layout, 2, state->menuIndex, "Quit");

  GfxDrawTextCentered("Click, or W and S and Enter", GfxScreenHeight() - 70, 18, GRAY);
}

static void DrawPause(const GameState *state) {
  DrawDim();
  GfxDrawTextCentered("PAUSED", 140, 56, RAYWHITE);

  MenuLayout layout = MenuLayoutFor(MODE_PAUSED);
  DrawMenuRow(&layout, 0, state->menuIndex, "Resume");
  DrawMenuRow(&layout, 1, state->menuIndex, "Settings");
  DrawMenuRow(&layout, 2, state->menuIndex, "Quit to title");

  GfxDrawTextCentered("Escape to resume", GfxScreenHeight() - 70, 18, GRAY);
}

static void DrawSettings(ecs_world_t *world, const GameState *state) {
  const Settings *settings = ecs_singleton_get(world, Settings);
  Resolution resolution = SettingsResolutionAt(settings->resolutionIndex);

  DrawDim();
  GfxDrawTextCentered("SETTINGS", 140, 56, RAYWHITE);

  MenuLayout layout = MenuLayoutFor(MODE_SETTINGS);
  DrawMenuRow(&layout, 0, state->menuIndex, GfxFormat("Resolution    < %s >", resolution.label));
  DrawMenuRow(&layout, 1, state->menuIndex,
              GfxFormat("Fullscreen    < %s >", settings->fullscreen ? "on" : "off"));
  DrawMenuRow(&layout, 2, state->menuIndex, "Back");

  GfxDrawTextCentered("Click either side of a row to change it   .   Escape to go back",
                      GfxScreenHeight() - 70, 18, GRAY);
}

static void DrawLevelCleared(const GameState *state) {
  const Level *level = LevelAt(state->levelIndex);
  DrawDim();
  GfxDrawTextCentered("LEVEL CLEARED", 150, 52, COLOR_EXIT);
  GfxDrawTextCentered(level->name, 215, 26, RAYWHITE);
  GfxDrawTextCentered(GfxFormat("Time %.1fs    Coins %d    Score %d", state->levelTime,
                                state->coins, state->score),
                      GfxScreenHeight() / 2, 24, GRAY);
  GfxDrawTextCentered("Click or press Enter for the next level", GfxScreenHeight() / 2 + 60, 24,
                      COLOR_BULLET);
}

static void DrawGameOver(const GameState *state) {
  DrawDim();
  GfxDrawTextCentered("YOU DIED", 150, 56, COLOR_ENEMY);
  GfxDrawTextCentered(GfxFormat("Score %d    Coins %d", state->score, state->coins),
                      GfxScreenHeight() / 2, 24, RAYWHITE);
  GfxDrawTextCentered("Click or press Enter to retry this level", GfxScreenHeight() / 2 + 60, 24,
                      COLOR_BULLET);
}

static void DrawRunComplete(const GameState *state) {
  DrawDim();
  GfxDrawTextCentered("RUN COMPLETE", 150, 56, COLOR_EXIT);
  GfxDrawTextCentered(GfxFormat("Final score %d    Coins %d", state->score, state->coins),
                      GfxScreenHeight() / 2, 26, RAYWHITE);
  GfxDrawTextCentered("Click or press Enter to return to the title", GfxScreenHeight() / 2 + 60,
                      24, COLOR_BULLET);
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

  GfxDrawText("WASD move  .  Mouse aim  .  Click fire  .  Esc pause  .  R restart  .  F1 AI view",
              24, GfxScreenHeight() - 34, 18, GRAY);

  // The legend only earns its space while the view it explains is on.
  if (ecs_singleton_get(world, DebugView)->showAi) {
    int x = GfxScreenWidth() - 250;
    int y = 96;
    GfxDrawText("AI VIEW", x, y, 20, RAYWHITE);
    GfxDrawText("guard", x, y + 28, 18, COLOR_AI_GUARD);
    GfxDrawText("patrol", x, y + 50, 18, COLOR_AI_PATROL);
    GfxDrawText("investigating", x, y + 72, 18, COLOR_AI_INVESTIGATE);
    GfxDrawText("chasing", x, y + 94, 18, COLOR_AI_CHASE);
    GfxDrawText("can see you", x, y + 122, 18, COLOR_AI_SEEN);
    GfxDrawText("inner ring: sight", x, y + 144, 16, COLOR_AI_SIGHT);
    GfxDrawText("outer ring: hearing", x, y + 164, 16, COLOR_AI_HEARING);
  }
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
