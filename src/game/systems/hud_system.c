// The flat overlay: health, score, wave, crosshair and the game over panel.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/systems/systems.h"

#define PANEL_WIDTH 420
#define PANEL_HEIGHT 190

static void DrawHealthBar(const PlayerTracker *tracker) {
  const int x = 24;
  const int y = 24;
  const int width = 280;
  const int height = 22;

  float fraction = tracker->maxHealth > 0 ? (float)tracker->health / (float)tracker->maxHealth : 0.0f;
  GfxDrawRect(x - 2, y - 2, width + 4, height + 4, (Color){0, 0, 0, 150});
  GfxDrawRect(x, y, (int)(width * fraction), height, fraction < 0.35f ? COLOR_ENEMY : COLOR_GOOD);
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

// Returns true when the play again button was pressed.
static bool DrawGameOver(const GameState *state) {
  int x = (GfxScreenWidth() - PANEL_WIDTH) / 2;
  int y = (GfxScreenHeight() - PANEL_HEIGHT) / 2;

  GfxDrawRect(0, 0, GfxScreenWidth(), GfxScreenHeight(), (Color){0, 0, 0, 160});
  GfxPanel((Rect){(float)x, (float)y, (float)PANEL_WIDTH, (float)PANEL_HEIGHT}, "#113#Game over");
  GfxDrawText(GfxFormat("Score %d", state->score), x + 24, y + 50, 28, RAYWHITE);
  GfxDrawText(GfxFormat("Reached wave %d", state->wave), x + 24, y + 88, 20, GRAY);
  return GfxButton((Rect){(float)(x + 24), (float)(y + 126), 160.0f, 36.0f}, "#77#Play again");
}

static void HudSystem(ecs_iter_t *it) {
  const GameState *state = ecs_singleton_get(it->world, GameState);
  const PlayerTracker *tracker = ecs_singleton_get(it->world, PlayerTracker);
  const Input *input = ecs_singleton_get(it->world, Input);

  DrawHealthBar(tracker);
  GfxDrawText(GfxFormat("SCORE  %d", state->score), 24, 60, 24, RAYWHITE);
  GfxDrawText(GfxFormat("WAVE  %d", state->wave), 24, 92, 20, GRAY);
  GfxDrawText(GfxFormat("ENEMIES  %d", ecs_count(it->world, Enemy)), 24, 118, 20, GRAY);
  GfxDrawFps(GfxScreenWidth() - 90, 24);

  if (!state->over && ecs_count(it->world, Enemy) == 0) {
    GfxDrawTextCentered(GfxFormat("Wave %d in %.1fs", state->wave + 1, state->waveBreak), 80, 28,
                        COLOR_GOOD);
  }

  GfxDrawText("WASD move   .   Mouse aim   .   Left click fire   .   R restart", 24,
              GfxScreenHeight() - 34, 18, GRAY);

  if (!state->over) {
    DrawCrosshair(input);
    return;
  }

  if (DrawGameOver(state)) {
    // The button feeds the same restart flag the keyboard and pad use, so the
    // restart system stays the only place a run is torn down.
    ecs_singleton_get_mut(it->world, Input)->restart = true;
  }
}

void HudSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, HudSystem, PhaseDrawUI, 0);
}
