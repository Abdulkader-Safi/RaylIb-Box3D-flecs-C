#include "game/ai/nav.h"

#include "game/config.h"

#include <string.h>

#define NAV_UNREACHABLE 0xFFFF

// One level is loaded at a time, so the grid lives here the way the physics
// world lives in core/physics.c. Making it a component would mean copying a
// few thousand tiles around for no gain.
static const Level *g_level;
static int g_width;
static int g_height;
static bool g_blocked[LEVEL_MAX_HEIGHT][LEVEL_MAX_WIDTH]; // Stops a body.
static bool g_opaque[LEVEL_MAX_HEIGHT][LEVEL_MAX_WIDTH];  // Stops a look.
static uint16_t g_distance[2][LEVEL_MAX_HEIGHT][LEVEL_MAX_WIDTH];

// Eight ways out of a tile. The diagonals come last so that when two steps tie
// on distance the straight one is taken, which looks less drunk.
static const int STEP_COLUMN[8] = {1, -1, 0, 0, 1, 1, -1, -1};
static const int STEP_ROW[8] = {0, 0, 1, -1, 1, -1, 1, -1};

static bool InBounds(int column, int row) {
  return column >= 0 && column < g_width && row >= 0 && row < g_height;
}

static void WorldToTile(Vec3 position, int *column, int *row) {
  *column = (int)floorf(position.x / TILE_SIZE + (float)g_width * 0.5f);
  *row = (int)floorf(position.z / TILE_SIZE + (float)g_height * 0.5f);
}

// A diagonal is only a step if both of the squares it cuts between are open.
// Without this enemies slip through the corner where two walls meet.
static bool CanStep(int column, int row, int direction) {
  int nextColumn = column + STEP_COLUMN[direction];
  int nextRow = row + STEP_ROW[direction];
  if (!InBounds(nextColumn, nextRow) || g_blocked[nextRow][nextColumn]) {
    return false;
  }
  if (STEP_COLUMN[direction] != 0 && STEP_ROW[direction] != 0) {
    if (g_blocked[row][nextColumn] || g_blocked[nextRow][column]) {
      return false;
    }
  }
  return true;
}

void NavSetLevel(const Level *level) {
  g_level = level;
  g_width = level != NULL ? level->width : 0;
  g_height = level != NULL ? level->height : 0;
  memset(g_distance, 0xFF, sizeof(g_distance));
}

static void RebuildPassability(bool doorsOpen) {
  for (int row = 0; row < g_height; ++row) {
    for (int column = 0; column < g_width; ++column) {
      char tile = g_level->rows[row][column];
      bool wall = tile == '#' || tile == ' ';
      bool shutDoor = tile == 'D' && !doorsOpen;
      g_blocked[row][column] = wall || shutDoor || tile == 'o';
      g_opaque[row][column] = wall || shutDoor;
    }
  }
}

// Breadth first from one tile, filling in how many steps every square is away.
static void FillField(int goalIndex, Vec3 goalPosition, bool active) {
  uint16_t (*field)[LEVEL_MAX_WIDTH] = g_distance[goalIndex];
  for (int row = 0; row < g_height; ++row) {
    for (int column = 0; column < g_width; ++column) {
      field[row][column] = NAV_UNREACHABLE;
    }
  }
  if (!active) {
    return;
  }

  int goalColumn;
  int goalRow;
  WorldToTile(goalPosition, &goalColumn, &goalRow);
  if (!InBounds(goalColumn, goalRow) || g_blocked[goalRow][goalColumn]) {
    return;
  }

  static int queueColumn[LEVEL_MAX_HEIGHT * LEVEL_MAX_WIDTH];
  static int queueRow[LEVEL_MAX_HEIGHT * LEVEL_MAX_WIDTH];
  int head = 0;
  int tail = 0;

  field[goalRow][goalColumn] = 0;
  queueColumn[tail] = goalColumn;
  queueRow[tail] = goalRow;
  tail += 1;

  while (head < tail) {
    int column = queueColumn[head];
    int row = queueRow[head];
    head += 1;

    for (int direction = 0; direction < 8; ++direction) {
      if (!CanStep(column, row, direction)) {
        continue;
      }
      int nextColumn = column + STEP_COLUMN[direction];
      int nextRow = row + STEP_ROW[direction];
      if (field[nextRow][nextColumn] != NAV_UNREACHABLE) {
        continue;
      }
      field[nextRow][nextColumn] = (uint16_t)(field[row][column] + 1);
      queueColumn[tail] = nextColumn;
      queueRow[tail] = nextRow;
      tail += 1;
    }
  }
}

void NavRebuild(bool doorsOpen, Vec3 playerPosition, bool alertActive, Vec3 alertPosition) {
  if (g_level == NULL) {
    return;
  }
  RebuildPassability(doorsOpen);
  FillField(NAV_GOAL_PLAYER, playerPosition, true);
  FillField(NAV_GOAL_ALERT, alertPosition, alertActive);
}

bool NavDirectionToward(Vec3 from, NavGoal goal, Vec3 *outDirection) {
  if (g_level == NULL) {
    return false;
  }

  int column;
  int row;
  WorldToTile(from, &column, &row);
  if (!InBounds(column, row)) {
    return false;
  }

  uint16_t (*field)[LEVEL_MAX_WIDTH] = g_distance[goal];
  uint16_t best = field[row][column];
  int bestColumn = -1;
  int bestRow = -1;

  // Standing on the goal is arriving, not a step.
  if (best == 0) {
    return false;
  }

  for (int direction = 0; direction < 8; ++direction) {
    if (!CanStep(column, row, direction)) {
      continue;
    }
    int nextColumn = column + STEP_COLUMN[direction];
    int nextRow = row + STEP_ROW[direction];
    if (field[nextRow][nextColumn] < best) {
      best = field[nextRow][nextColumn];
      bestColumn = nextColumn;
      bestRow = nextRow;
    }
  }

  if (bestColumn < 0) {
    return false;
  }

  // Aim at the middle of the next tile. Steering straight at the goal instead
  // is what drags a body along the wall it is trying to get around.
  Vec3 target = LevelTileToWorld(g_level, bestColumn, bestRow, from.y);
  Vec3 delta = Vec3Flat(Vec3Sub(target, from));
  if (Vec3LengthSquared(delta) < 1e-6f) {
    return false;
  }
  *outDirection = Vec3Normalize(delta);
  return true;
}

// Follows the same rule the enemies do, one tile at a time, and records where
// it went. Drawing this is drawing exactly the route they will take.
int NavRoutePoints(Vec3 from, NavGoal goal, float height, Vec3 *points, int maxPoints) {
  if (g_level == NULL || maxPoints <= 0) {
    return 0;
  }

  int column;
  int row;
  WorldToTile(from, &column, &row);
  if (!InBounds(column, row)) {
    return 0;
  }

  uint16_t (*field)[LEVEL_MAX_WIDTH] = g_distance[goal];
  int written = 0;

  while (written < maxPoints) {
    uint16_t best = field[row][column];
    if (best == 0 || best == NAV_UNREACHABLE) {
      break;
    }

    int bestColumn = -1;
    int bestRow = -1;
    for (int direction = 0; direction < 8; ++direction) {
      if (!CanStep(column, row, direction)) {
        continue;
      }
      int nextColumn = column + STEP_COLUMN[direction];
      int nextRow = row + STEP_ROW[direction];
      if (field[nextRow][nextColumn] < best) {
        best = field[nextRow][nextColumn];
        bestColumn = nextColumn;
        bestRow = nextRow;
      }
    }
    if (bestColumn < 0) {
      break;
    }

    column = bestColumn;
    row = bestRow;
    points[written++] = LevelTileToWorld(g_level, column, row, height);
  }
  return written;
}

float NavRouteDistance(Vec3 from, NavGoal goal) {
  if (g_level == NULL) {
    return -1.0f;
  }
  int column;
  int row;
  WorldToTile(from, &column, &row);
  if (!InBounds(column, row)) {
    return -1.0f;
  }
  uint16_t steps = g_distance[goal][row][column];
  if (steps == NAV_UNREACHABLE) {
    return -1.0f;
  }
  return (float)steps * TILE_SIZE;
}

bool NavLineOfSight(Vec3 from, Vec3 to) {
  if (g_level == NULL) {
    return false;
  }

  int column;
  int row;
  int targetColumn;
  int targetRow;
  WorldToTile(from, &column, &row);
  WorldToTile(to, &targetColumn, &targetRow);
  if (!InBounds(column, row) || !InBounds(targetColumn, targetRow)) {
    return false;
  }

  // Bresenham from one tile to the other, stopping at the first wall.
  int deltaColumn = abs(targetColumn - column);
  int deltaRow = -abs(targetRow - row);
  int stepColumn = column < targetColumn ? 1 : -1;
  int stepRow = row < targetRow ? 1 : -1;
  int error = deltaColumn + deltaRow;

  while (column != targetColumn || row != targetRow) {
    int doubled = 2 * error;
    if (doubled >= deltaRow) {
      error += deltaRow;
      column += stepColumn;
    }
    if (doubled <= deltaColumn) {
      error += deltaColumn;
      row += stepRow;
    }
    if (!InBounds(column, row)) {
      return false;
    }
    if (g_opaque[row][column]) {
      return false;
    }
  }
  return true;
}
