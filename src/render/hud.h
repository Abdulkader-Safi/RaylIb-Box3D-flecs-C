// The 2D overlay: health, score, wave, crosshair and the game over panel.
#ifndef HUD_H
#define HUD_H

#include <stdbool.h>

struct Game;

// Returns true when the player asked for a restart through the game over panel.
bool HudDraw(const struct Game *game);

#endif // HUD_H
