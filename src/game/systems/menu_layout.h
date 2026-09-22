// Where the menu rows sit on screen.
//
// Two systems need this and they must agree: hud_system draws the rows,
// mode_system decides what the mouse is over and what it clicked. Working it
// out in both places is how a menu ends up looking right and clicking wrong,
// so it is worked out here once.
#ifndef GAME_MENU_LAYOUT_H
#define GAME_MENU_LAYOUT_H

#include "core/core.h"
#include "game/components/components.h"

#define MENU_MAX_ROWS 4
#define MENU_ROW_HEIGHT 38
#define MENU_ROW_WIDTH 620
#define MENU_TEXT_SIZE 28

typedef struct MenuLayout {
  Rect rows[MENU_MAX_ROWS];
  int count;
} MenuLayout;

// Empty (count 0) for modes that have no rows to click.
MenuLayout MenuLayoutFor(GameMode mode);

// Which row contains the point, or -1 for none.
int MenuLayoutHit(const MenuLayout *layout, Vec2 point);

// Left half or right half of a row, as -1 or 1. Rows that carry a value use
// this so clicking the arrows either side does what the arrow keys do.
int MenuLayoutSide(const MenuLayout *layout, int row, Vec2 point);

#endif // GAME_MENU_LAYOUT_H
