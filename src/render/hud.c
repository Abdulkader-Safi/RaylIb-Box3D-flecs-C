#include "render/hud.h"

// raygui ships as a single header. This is the one place it is compiled.
#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#include "core/config.h"
#include "core/game.h"

static const Color COLOR_PANEL = {0, 0, 0, 150};
static const Color COLOR_HEALTH = {90, 220, 140, 255};
static const Color COLOR_HEALTH_LOW = {235, 84, 84, 255};

static void DrawHealthBar(int health) {
  const int x = 24;
  const int y = 24;
  const int width = 280;
  const int height = 22;

  float fraction = (float)health / (float)PLAYER_MAX_HEALTH;
  DrawRectangle(x - 2, y - 2, width + 4, height + 4, COLOR_PANEL);
  DrawRectangle(x, y, (int)(width * fraction), height,
                fraction < 0.35f ? COLOR_HEALTH_LOW : COLOR_HEALTH);
  DrawRectangleLines(x, y, width, height, RAYWHITE);
  DrawText(TextFormat("%d / %d", health, PLAYER_MAX_HEALTH), x + 8, y + 3, 16, RAYWHITE);
}

static void DrawCrosshair(void) {
  Vector2 mouse = GetMousePosition();
  DrawCircleLinesV(mouse, 10.0f, RAYWHITE);
  DrawLineV((Vector2){mouse.x - 16, mouse.y}, (Vector2){mouse.x - 4, mouse.y}, RAYWHITE);
  DrawLineV((Vector2){mouse.x + 4, mouse.y}, (Vector2){mouse.x + 16, mouse.y}, RAYWHITE);
  DrawLineV((Vector2){mouse.x, mouse.y - 16}, (Vector2){mouse.x, mouse.y - 4}, RAYWHITE);
  DrawLineV((Vector2){mouse.x, mouse.y + 4}, (Vector2){mouse.x, mouse.y + 16}, RAYWHITE);
}

static bool DrawGameOver(const Game *game) {
  int width = 420;
  int height = 190;
  int x = (GetScreenWidth() - width) / 2;
  int y = (GetScreenHeight() - height) / 2;

  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 160});
  GuiPanel((Rectangle){(float)x, (float)y, (float)width, (float)height}, "#113#Game over");
  DrawText(TextFormat("Score %d", game->score), x + 24, y + 50, 28, RAYWHITE);
  DrawText(TextFormat("Reached wave %d", game->wave), x + 24, y + 88, 20, GRAY);

  return GuiButton((Rectangle){(float)(x + 24), (float)(y + 126), 160.0f, 36.0f}, "#77#Play again");
}

bool HudDraw(const Game *game) {
  DrawHealthBar(game->player.health);

  DrawText(TextFormat("SCORE  %d", game->score), 24, 60, 24, RAYWHITE);
  DrawText(TextFormat("WAVE  %d", game->wave), 24, 92, 20, GRAY);
  DrawText(TextFormat("ENEMIES  %d", game->enemies.aliveCount), 24, 118, 20, GRAY);
  DrawFPS(GetScreenWidth() - 90, 24);

  if (game->state == GAME_PLAYING && game->enemies.aliveCount == 0) {
    const char *message = TextFormat("Wave %d in %.1fs", game->wave + 1, game->waveBreakTimer);
    int textWidth = MeasureText(message, 28);
    DrawText(message, (GetScreenWidth() - textWidth) / 2, 80, 28, COLOR_HEALTH);
  }

  DrawText("WASD move   .   Mouse aim   .   Left click fire   .   R restart", 24,
           GetScreenHeight() - 34, 18, GRAY);

  if (game->state == GAME_OVER) {
    return DrawGameOver(game);
  }

  DrawCrosshair();
  return false;
}
