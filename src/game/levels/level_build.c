// Turns a grid of characters into entities.
#include "game/levels/levels.h"

#include "game/ai/nav.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/spawn/spawn.h"

static char TileAt(const Level *level, int column, int row) {
  if (column < 0 || column >= level->width || row < 0 || row >= level->height) {
    return '#';
  }
  return level->rows[row][column];
}

static bool IsWall(char tile) { return tile == '#'; }
static bool IsVoid(char tile) { return tile == ' '; }
static bool IsCover(char tile) { return tile == 'o'; }

Vec3 LevelTileToWorld(const Level *level, int column, int row, float height) {
  // The grid is centred on the origin, so nothing downstream needs an offset.
  float x = ((float)column - (float)level->width * 0.5f + 0.5f) * TILE_SIZE;
  float z = ((float)row - (float)level->height * 0.5f + 0.5f) * TILE_SIZE;
  return Vec3Make(x, height, z);
}

// Walls and floors are merged along each row before anything is built, so a
// twenty tile wall is one box rather than twenty. It saves the physics world a
// pile of bodies and the renderer a pile of draw calls, for about ten lines.
static void BuildRuns(ecs_world_t *world, const Level *level, bool (*match)(char), float height,
                      float centreY, Color color, bool solid) {
  for (int row = 0; row < level->height; ++row) {
    int column = 0;
    while (column < level->width) {
      if (!match(TileAt(level, column, row))) {
        column += 1;
        continue;
      }

      int start = column;
      while (column < level->width && match(TileAt(level, column, row))) {
        column += 1;
      }

      int length = column - start;
      Vec3 centre = LevelTileToWorld(level, start, row, centreY);
      centre.x += (float)(length - 1) * TILE_SIZE * 0.5f;
      Vec3 size = Vec3Make((float)length * TILE_SIZE, height, TILE_SIZE);
      SpawnBlock(world, centre, size, color, solid);
    }
  }
}

static bool MatchWall(char tile) { return IsWall(tile); }
static bool MatchCover(char tile) { return IsCover(tile); }
static bool MatchFloor(char tile) { return !IsVoid(tile) && !IsWall(tile); }

static void BuildContents(ecs_world_t *world, const Level *level) {
  float standing = ENEMY_MEDIUM_RADIUS + 0.6f;

  for (int row = 0; row < level->height; ++row) {
    for (int column = 0; column < level->width; ++column) {
      char tile = TileAt(level, column, row);
      Vec3 at = LevelTileToWorld(level, column, row, standing);
      Vec3 floorLevel = LevelTileToWorld(level, column, row, 0.6f);

      switch (tile) {
      case 'S': SpawnPlayer(world, LevelTileToWorld(level, column, row,
                                                    PLAYER_HALF_HEIGHT + PLAYER_RADIUS + 0.05f));
        break;
      case 'X': SpawnExit(world, LevelTileToWorld(level, column, row, 0.05f)); break;
      case 'D': SpawnDoor(world, LevelTileToWorld(level, column, row, ARENA_WALL_HEIGHT * 0.5f));
        break;
      case 'k': SpawnPickup(world, floorLevel, PICKUP_KEYCARD, 0); break;
      case 'c': SpawnPickup(world, floorLevel, PICKUP_COIN, 1); break;
      case '+': SpawnPickup(world, floorLevel, PICKUP_HEALTH, HEALTH_PICKUP_AMOUNT); break;
      case 'r': SpawnPickup(world, floorLevel, PICKUP_RAPID_FIRE, 0); break;
      case 's': SpawnPickup(world, floorLevel, PICKUP_SHIELD, 0); break;
      case 'l': SpawnEnemy(world, at, ENEMY_LIGHT); break;
      case 'm': SpawnEnemy(world, at, ENEMY_MEDIUM); break;
      case 'h': SpawnEnemy(world, at, ENEMY_HEAVY); break;
      case 'L': SpawnSpawner(world, floorLevel, ENEMY_LIGHT); break;
      case 'M': SpawnSpawner(world, floorLevel, ENEMY_MEDIUM); break;
      case 'H': SpawnSpawner(world, floorLevel, ENEMY_HEAVY); break;
      default: break;
      }
    }
  }
}

void LevelLoad(ecs_world_t *world, int index) {
  bool suspended = SpawnBegin(world);

  // Everything the game built carries Spawned, so one call clears the board and
  // the PhysicsBody destructor takes each rigid body down with it.
  ecs_delete_with(world, Spawned);

  const Level *level = LevelAt(index);
  NavSetLevel(level);

  BuildRuns(world, level, MatchFloor, ARENA_FLOOR_HALF_THICKNESS * 2.0f,
            -ARENA_FLOOR_HALF_THICKNESS - 0.05f, COLOR_FLOOR, false);
  BuildRuns(world, level, MatchWall, ARENA_WALL_HEIGHT, ARENA_WALL_HEIGHT * 0.5f, COLOR_WALL, true);
  BuildRuns(world, level, MatchCover, ARENA_COVER_HEIGHT, ARENA_COVER_HEIGHT * 0.5f, COLOR_COVER,
            true);

  // One slab under the whole grid so nothing can fall through a seam between
  // two floor runs. The walls stop anyone reaching the parts that show.
  Vec3 slabSize = Vec3Make((float)level->width * TILE_SIZE, ARENA_FLOOR_HALF_THICKNESS * 2.0f,
                           (float)level->height * TILE_SIZE);
  SpawnFloorSlab(world, Vec3Make(0.0f, -ARENA_FLOOR_HALF_THICKNESS, 0.0f), slabSize);

  BuildContents(world, level);

  ecs_singleton_set(world, Alert, {0});
  ecs_singleton_set(world, LevelBounds,
                    {Vec3Make((float)level->width * TILE_SIZE * 0.5f, 0.0f,
                              (float)level->height * TILE_SIZE * 0.5f)});

  GameState *state = ecs_singleton_get_mut(world, GameState);
  state->levelIndex = index;
  state->levelTime = 0.0f;
  state->hasKeycard = false;

  SpawnEnd(world, suspended);
}
