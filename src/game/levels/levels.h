// Levels are drawn as text and built into entities at load time.
//
// A grid of characters is the fastest thing to author and to read back, and
// keeping the maps in C means the web build has no data files to fetch before
// it can start.
//
//   #  wall            .  floor           (space) outside the level
//   o  low cover       S  start           X  exit
//   D  locked door     k  keycard
//   l  light enemy     m  medium enemy    h  heavy enemy
//   L  light spawner   M  medium spawner  H  heavy spawner
//   c  coin            +  health          r  rapid fire     s  shield
#ifndef GAME_LEVELS_H
#define GAME_LEVELS_H

#include "core/core.h"

typedef struct Level {
  const char *name;
  const char *brief;
  int width;
  int height;
  const char *const *rows;
} Level;

int LevelCount(void);
const Level *LevelAt(int index);

// Clears whatever is standing and builds the level, player included.
void LevelLoad(ecs_world_t *world, int index);

// Where a tile sits in the world. The grid is centred on the origin so the
// camera has nothing special to do.
Vec3 LevelTileToWorld(const Level *level, int column, int row, float height);

#endif // GAME_LEVELS_H
