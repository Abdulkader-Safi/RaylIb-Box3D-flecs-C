#include "game/systems/menu_layout.h"

static int RowCountFor(GameMode mode) {
  switch (mode) {
  case MODE_MENU: return 3;     // Start run, Settings, Quit.
  case MODE_PAUSED: return 3;   // Resume, Settings, Quit to title.
  case MODE_SETTINGS: return 3; // Resolution, Fullscreen, Back.
  default: return 0;
  }
}

MenuLayout MenuLayoutFor(GameMode mode) {
  MenuLayout layout = {0};
  layout.count = RowCountFor(mode);

  // The block is centred on the screen, with the first row one row above
  // centre. Drawing and hit testing both start from this.
  float left = (float)(GfxScreenWidth() - MENU_ROW_WIDTH) * 0.5f;
  float top = (float)(GfxScreenHeight() / 2 - MENU_ROW_HEIGHT);

  for (int i = 0; i < layout.count; ++i) {
    layout.rows[i] = (Rect){left, top + (float)(i * MENU_ROW_HEIGHT), (float)MENU_ROW_WIDTH,
                            (float)MENU_ROW_HEIGHT};
  }
  return layout;
}

int MenuLayoutHit(const MenuLayout *layout, Vec2 point) {
  for (int i = 0; i < layout->count; ++i) {
    Rect row = layout->rows[i];
    if (point.x >= row.x && point.x <= row.x + row.width && point.y >= row.y &&
        point.y <= row.y + row.height) {
      return i;
    }
  }
  return -1;
}

int MenuLayoutSide(const MenuLayout *layout, int row, Vec2 point) {
  if (row < 0 || row >= layout->count) {
    return 1;
  }
  Rect bounds = layout->rows[row];
  return point.x < bounds.x + bounds.width * 0.5f ? -1 : 1;
}
