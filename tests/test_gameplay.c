// Runs the real game with no screen attached and checks that the rules hold.
//
// This is the point of AppWorldCreate(false) and GameRegisterHeadless: the
// systems under test are the ones that ship, not a copy of them.
//
// Build and run:  make test
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/game.h"
#include "game/levels/levels.h"
#include "game/ai/nav.h"
#include "game/systems/menu_layout.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define STEP (1.0f / 60.0f)

// Stands in for the input system, which needs a window. Aiming with the stick
// rather than the mouse also keeps the camera out of it: a stick gives a
// direction, so nothing has to be projected through a viewport.
static void PushInput(ecs_world_t *world, Vec2 aimStick, bool fire) {
  Input *input = ecs_singleton_get_mut(world, Input);
  memset(input, 0, sizeof(*input));
  input->aimIsStick = true;
  input->aimStick = aimStick;
  input->fire = fire;
}

// Clicks the middle of a menu row, the way a mouse would. The layout comes
// from the same function the HUD draws with, so this fails if the two ever
// stop agreeing about where a row is.
static void ClickMenuRow(ecs_world_t *world, GameMode mode, int row) {
  MenuLayout layout = MenuLayoutFor(mode);
  assert(row < layout.count);
  Rect bounds = layout.rows[row];

  Input *input = ecs_singleton_get_mut(world, Input);
  memset(input, 0, sizeof(*input));
  input->aimScreen = Vec2Make(bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f);
  input->click = true;
  input->mouseMoved = true;
}

// Clicks the left or right half of a row, which is how the settings arrows
// work under the mouse.
static void ClickMenuRowSide(ecs_world_t *world, GameMode mode, int row, int side) {
  MenuLayout layout = MenuLayoutFor(mode);
  assert(row < layout.count);
  Rect bounds = layout.rows[row];
  float x = side < 0 ? bounds.x + bounds.width * 0.25f : bounds.x + bounds.width * 0.75f;

  Input *input = ecs_singleton_get_mut(world, Input);
  memset(input, 0, sizeof(*input));
  input->aimScreen = Vec2Make(x, bounds.y + bounds.height * 0.5f);
  input->click = true;
  input->mouseMoved = true;
}

static void PressConfirm(ecs_world_t *world) {
  Input *input = ecs_singleton_get_mut(world, Input);
  memset(input, 0, sizeof(*input));
  input->confirm = true;
}

// The walking checks below are about the rules, not about surviving the trip,
// so the room gets cleared and the player patched up first.
static void ClearTheRoom(ecs_world_t *world) {
  ecs_delete_with(world, Enemy);
  const PlayerTracker *tracker = ecs_singleton_get(world, PlayerTracker);
  Health *health = ecs_get_mut(world, tracker->entity, Health);
  health->current = health->max;
}

static void Idle(ecs_world_t *world, int frames) {
  for (int i = 0; i < frames; ++i) {
    PushInput(world, VEC2_ZERO, false);
    ecs_progress(world, STEP);
  }
}

// Points the weapon at whichever enemy the query hands back first.
static Vec2 AimAtAnEnemy(ecs_world_t *world, bool *found) {
  const PlayerTracker *tracker = ecs_singleton_get(world, PlayerTracker);
  ecs_query_t *query =
      ecs_query(world, {.terms = {{.id = ecs_id(Position), .inout = EcsIn}, {.id = Enemy}}});
  Vec2 aim = {0.0f, 1.0f};
  *found = false;

  ecs_iter_t it = ecs_query_iter(world, query);
  while (ecs_query_next(&it)) {
    if (it.count > 0) {
      const Position *positions = ecs_field(&it, Position, 0);
      Vec3 toEnemy = Vec3Normalize(Vec3Flat(Vec3Sub(positions[0].value, tracker->position)));
      // The stick's y axis points up the screen, which is negative Z.
      aim = Vec2Normalize(Vec2Make(toEnemy.x, -toEnemy.z));
      *found = true;
      ecs_iter_fini(&it);
      break;
    }
  }
  ecs_query_fini(query);
  return aim;
}

// ---------------------------------------------------------------- the grid
//
// The test walks the player around for real, so it needs to know the way. A
// breadth first search over the tiles gives it one, and asking for that path
// at all is the check that matters: a level nobody can finish fails here
// rather than in someone's hands.

typedef struct Tile {
  int column;
  int row;
} Tile;

static bool FindTile(const Level *level, char wanted, Tile *out) {
  for (int row = 0; row < level->height; ++row) {
    for (int column = 0; column < level->width; ++column) {
      if (level->rows[row][column] == wanted) {
        *out = (Tile){column, row};
        return true;
      }
    }
  }
  return false;
}

static bool Passable(char tile, bool doorsOpen) {
  if (tile == '#' || tile == ' ' || tile == 'o') return false;
  if (tile == 'D') return doorsOpen;
  return true;
}

// Fills `path` from start to goal, goal first tile last. Returns the length,
// or 0 when there is no way through.
static int FindPath(const Level *level, Tile start, Tile goal, bool doorsOpen, Tile *path,
                    int capacity) {
  static int cameFrom[LEVEL_MAX_HEIGHT][LEVEL_MAX_WIDTH];
  static Tile queue[LEVEL_MAX_HEIGHT * LEVEL_MAX_WIDTH];

  for (int row = 0; row < level->height; ++row) {
    for (int column = 0; column < level->width; ++column) {
      cameFrom[row][column] = -1;
    }
  }

  int head = 0;
  int tail = 0;
  queue[tail++] = start;
  cameFrom[start.row][start.column] = -2; // The start has no predecessor.

  const int stepColumn[4] = {1, -1, 0, 0};
  const int stepRow[4] = {0, 0, 1, -1};

  while (head < tail) {
    Tile at = queue[head++];
    if (at.column == goal.column && at.row == goal.row) {
      break;
    }
    for (int i = 0; i < 4; ++i) {
      int column = at.column + stepColumn[i];
      int row = at.row + stepRow[i];
      if (column < 0 || column >= level->width || row < 0 || row >= level->height) continue;
      if (cameFrom[row][column] != -1) continue;
      if (!Passable(level->rows[row][column], doorsOpen)) continue;
      cameFrom[row][column] = i;
      queue[tail++] = (Tile){column, row};
    }
  }

  if (cameFrom[goal.row][goal.column] == -1) {
    return 0;
  }

  // Walk the predecessors back to the start, then reverse.
  int length = 0;
  Tile at = goal;
  while (!(at.column == start.column && at.row == start.row)) {
    if (length >= capacity) return 0;
    path[length++] = at;
    int direction = cameFrom[at.row][at.column];
    at.column -= stepColumn[direction];
    at.row -= stepRow[direction];
  }
  for (int i = 0; i < length / 2; ++i) {
    Tile swap = path[i];
    path[i] = path[length - 1 - i];
    path[length - 1 - i] = swap;
  }
  return length;
}

// Drives the move stick from tile centre to tile centre. Going via the centres
// keeps the player clear of the corners it would otherwise snag on.
static bool WalkPath(ecs_world_t *world, const Level *level, const Tile *path, int length,
                     int framesPerTile) {
  const PlayerTracker *tracker = ecs_singleton_get(world, PlayerTracker);
  const GameState *state = ecs_singleton_get(world, GameState);

  for (int i = 0; i < length; ++i) {
    // The trip can end before the last tile: stepping within reach of the exit
    // clears the level, and a cleared level stops taking movement. That is an
    // arrival, not a failure.
    if (state->mode != MODE_PLAYING) {
      return true;
    }
    Vec3 target = LevelTileToWorld(level, path[i].column, path[i].row, 0.0f);
    bool arrived = false;

    for (int frame = 0; frame < framesPerTile && !arrived; ++frame) {
      if (state->mode != MODE_PLAYING) {
        return true;
      }
      Vec3 toTarget = Vec3Flat(Vec3Sub(target, tracker->position));
      if (Vec3Length(toTarget) <= 0.6f) {
        arrived = true;
        break;
      }
      Vec3 direction = Vec3Normalize(toTarget);
      Input *input = ecs_singleton_get_mut(world, Input);
      memset(input, 0, sizeof(*input));
      input->move = Vec2Make(direction.x, -direction.z);
      ecs_progress(world, STEP);
    }
    if (!arrived) {
      return false;
    }
  }
  return true;
}

// Walks to the first tile matching `wanted`, pathing around the walls.
static bool WalkToTile(ecs_world_t *world, const Level *level, char wanted, bool doorsOpen) {
  static Tile path[LEVEL_MAX_HEIGHT * LEVEL_MAX_WIDTH];
  Tile start;
  Tile goal;
  if (!FindTile(level, 'S', &start) || !FindTile(level, wanted, &goal)) {
    return false;
  }

  // The player has moved since the level was built, so the search starts from
  // the tile it is standing on rather than from the level's start marker.
  const PlayerTracker *tracker = ecs_singleton_get(world, PlayerTracker);
  Vec3 at = tracker->position;
  start.column = (int)((at.x / TILE_SIZE) + (float)level->width * 0.5f);
  start.row = (int)((at.z / TILE_SIZE) + (float)level->height * 0.5f);

  int length = FindPath(level, start, goal, doorsOpen, path, (int)(sizeof(path) / sizeof(path[0])));
  if (length == 0) {
    return false;
  }
  return WalkPath(world, level, path, length, 240);
}

// Formatting one result into another used to eat the front of it. The label
// "Resolution" reached the screen as "solution", because the second call wrote
// into the same buffer the first had returned: it laid "> " over the "Re" and
// then copied the string it was already standing on. The settings rows did
// exactly this, which is why only they were wrong.
static void CheckFormatting(void) {
  const char *inner = GfxFormat("Resolution    < %s >", "1280 x 720");
  const char *outer = GfxFormat("%s %s", ">", inner);
  assert(strstr(outer, "Resolution") != NULL);
  assert(strstr(outer, "1280 x 720") != NULL);
  assert(strstr(inner, "Resolution") != NULL);

  // The loan also has to outlive a couple of later calls, or passing one
  // result into another is luck rather than a rule.
  const char *first = GfxFormat("first %d", 1);
  GfxFormat("second %d", 2);
  GfxFormat("third %d", 3);
  assert(strstr(first, "first 1") != NULL);
}

int main(void) {
  CheckFormatting();
  RandomSeed(20240921); // Spawner stagger has to repeat run to run.

  ecs_world_t *world = AppWorldCreate(false);
  GameRegisterHeadless(world);

  const GameState *state = ecs_singleton_get(world, GameState);
  const PlayerTracker *tracker = ecs_singleton_get(world, PlayerTracker);

  // Every level has to be well formed, or the builder walks off the end of a
  // row, and every level has to be finishable.
  static Tile scratch[LEVEL_MAX_HEIGHT * LEVEL_MAX_WIDTH];
  for (int i = 0; i < LevelCount(); ++i) {
    const Level *level = LevelAt(i);
    assert(level->width <= LEVEL_MAX_WIDTH && level->height <= LEVEL_MAX_HEIGHT);

    int starts = 0;
    int exits = 0;
    for (int row = 0; row < level->height; ++row) {
      assert((int)strlen(level->rows[row]) == level->width);
      for (int column = 0; column < level->width; ++column) {
        if (level->rows[row][column] == 'S') starts += 1;
        if (level->rows[row][column] == 'X') exits += 1;
      }
    }
    assert(starts == 1);
    assert(exits == 1);

    Tile start;
    Tile keycard;
    Tile exit;
    assert(FindTile(level, 'S', &start));
    assert(FindTile(level, 'X', &exit));

    int capacity = (int)(sizeof(scratch) / sizeof(scratch[0]));
    if (FindTile(level, 'k', &keycard)) {
      // The keycard cannot be behind the door it opens.
      assert(FindPath(level, start, keycard, false, scratch, capacity) > 0);
    }
    // And the exit has to be reachable once the door is open.
    assert(FindPath(level, start, exit, true, scratch, capacity) > 0);
    printf("level %d (%s) is finishable\n", i + 1, level->name);
  }

  // The game opens on the title screen with level one standing behind it.
  assert(state->mode == MODE_MENU);
  Idle(world, 1);
  assert(tracker->entity != 0);
  assert(tracker->health == PLAYER_MAX_HEALTH);

  // Nothing moves while a menu is up.
  Vec3 restingPosition = tracker->position;
  Idle(world, 60);
  assert(Vec3Distance(restingPosition, tracker->position) < 0.01f);
  assert(ClockIsPaused(world));

  // The menus take the mouse. Clicking Settings on the title screen opens it,
  // clicking the right half of the resolution row steps through the list, and
  // Back returns to where it came from.
  int resolutionBefore = ecs_singleton_get(world, Settings)->resolutionIndex;
  ClickMenuRow(world, MODE_MENU, 1);
  ecs_progress(world, STEP);
  assert(state->mode == MODE_SETTINGS);

  ClickMenuRowSide(world, MODE_SETTINGS, 0, 1);
  ecs_progress(world, STEP);
  assert(ecs_singleton_get(world, Settings)->resolutionIndex !=
         resolutionBefore || SettingsResolutionCount() == 1);

  ClickMenuRow(world, MODE_SETTINGS, 2);
  ecs_progress(world, STEP);
  assert(state->mode == MODE_MENU);

  // Hovering moves the highlight without clicking anything.
  {
    MenuLayout layout = MenuLayoutFor(MODE_MENU);
    Input *hover = ecs_singleton_get_mut(world, Input);
    memset(hover, 0, sizeof(*hover));
    hover->aimScreen = Vec2Make(layout.rows[2].x + layout.rows[2].width * 0.5f,
                                layout.rows[2].y + layout.rows[2].height * 0.5f);
    hover->mouseMoved = true;
  }
  ecs_progress(world, STEP);
  assert(state->menuIndex == 2);
  assert(state->mode == MODE_MENU);

  // Start run is the first row of the title menu.
  ClickMenuRow(world, MODE_MENU, 0);
  ecs_progress(world, STEP);
  assert(state->mode == MODE_PLAYING);
  // The click that started the game must not also pull the trigger. Letting
  // the button go hands shooting back.
  assert(state->ignoreFireUntilRelease);
  Idle(world, 1);
  assert(!state->ignoreFireUntilRelease);
  assert(!ClockIsPaused(world));
  assert(state->levelIndex == 0);
  assert(!state->hasKeycard);

  // ---------------------------------------------------------------- the AI
  //
  // The rule that matters: an enemy only chases what it can actually see. If
  // this ever passes by accident it is because there are no enemies, so the
  // count is checked too.
  {
    ecs_query_t *query = ecs_query(
        world, {.terms = {{.id = ecs_id(Position), .inout = EcsIn},
                          {.id = ecs_id(Brain), .inout = EcsIn},
                          {.id = ecs_id(Senses), .inout = EcsIn},
                          {.id = Enemy}}});
    int checked = 0;
    ecs_iter_t chase = ecs_query_iter(world, query);
    while (ecs_query_next(&chase)) {
      const Position *at = ecs_field(&chase, Position, 0);
      const Brain *brain = ecs_field(&chase, Brain, 1);
      const Senses *sense = ecs_field(&chase, Senses, 2);
      for (int i = 0; i < chase.count; ++i) {
        checked += 1;
        if (brain[i].state != AI_CHASE) {
          continue;
        }
        // Chasing means it can see the player, in range and in the open.
        assert(NavLineOfSight(at[i].value, tracker->position));
        assert(Vec3Length(Vec3Flat(Vec3Sub(tracker->position, at[i].value))) <= sense[i].sight);
      }
    }
    assert(checked > 0);
    ecs_query_fini(query);
  }

  // An enemy that has lost interest and cannot see anything never routes into
  // a wall. Every step the navigation offers has to be a step it can take.
  {
    ecs_query_t *query = ecs_query(
        world, {.terms = {{.id = ecs_id(Position), .inout = EcsIn}, {.id = Enemy}}});
    ecs_iter_t walk = ecs_query_iter(world, query);
    int routed = 0;
    while (ecs_query_next(&walk)) {
      const Position *at = ecs_field(&walk, Position, 0);
      for (int i = 0; i < walk.count; ++i) {
        Vec3 step;
        if (!NavDirectionToward(at[i].value, NAV_GOAL_PLAYER, &step)) {
          continue;
        }
        routed += 1;
        // A step of one tile must land somewhere with a route of its own,
        // which a wall never has.
        Vec3 next = Vec3Add(at[i].value, Vec3Scale(step, TILE_SIZE));
        assert(NavRouteDistance(next, NAV_GOAL_PLAYER) >= 0.0f);
      }
    }
    // At the start of level one the enemies share a region with the player,
    // so some of them must have a route. Later in the level the survivors are
    // behind a shut door and correctly have none at all.
    assert(routed > 0);
    ecs_query_fini(query);
  }

  // Being heard pulls them in. An enemy round a corner, alerted to where the
  // player is, has to close the distance along the route rather than grind
  // into the wall between the two of them.
  {
    Alert *alert = ecs_singleton_get_mut(world, Alert);
    alert->position = tracker->position;
    alert->timer = 600.0f; // Long enough that the test is not racing it.
    alert->active = true;

    ecs_query_t *query = ecs_query(
        world, {.terms = {{.id = ecs_id(Position), .inout = EcsIn}, {.id = Enemy}}});
    ecs_iter_t seek = ecs_query_iter(world, query);
    ecs_entity_t hunter = 0;
    float startRoute = -1.0f;
    bool stopped = false;
    while (!stopped && ecs_query_next(&seek)) {
      const Position *at = ecs_field(&seek, Position, 0);
      for (int i = 0; i < seek.count; ++i) {
        float route = NavRouteDistance(at[i].value, NAV_GOAL_PLAYER);
        // Pick one that has to go the long way round.
        if (route > 8.0f && !NavLineOfSight(at[i].value, tracker->position)) {
          hunter = seek.entities[i];
          startRoute = route;
          stopped = true;
          break;
        }
      }
    }
    // Only finish an iterator that was abandoned part way. One that ran out
    // has already finished itself.
    if (stopped) {
      ecs_iter_fini(&seek);
    }
    ecs_query_fini(query);

    if (hunter != 0) {
      for (int frame = 0; frame < 600; ++frame) {
        Alert *keep = ecs_singleton_get_mut(world, Alert);
        keep->position = tracker->position;
        keep->timer = 600.0f;
        keep->active = true;
        PushInput(world, VEC2_ZERO, false);
        ecs_progress(world, STEP);
      }
      float endRoute = NavRouteDistance(ecs_get(world, hunter, Position)->value, NAV_GOAL_PLAYER);
      printf("hunter route %.1fm -> %.1fm\n", startRoute, endRoute);
      assert(endRoute >= 0.0f);
      assert(endRoute < startRoute);
    }
  }

  // Shooting an enemy has to pay out in score and leave coins behind.
  int enemiesAtStart = ecs_count(world, Enemy);
  assert(enemiesAtStart > 0);
  for (int frame = 0; frame < 900; ++frame) {
    bool found = false;
    Vec2 aim = AimAtAnEnemy(world, &found);
    PushInput(world, aim, found);
    ecs_progress(world, STEP);
  }
  printf("after 15s: score %d, coins %d, enemies %d, health %d\n", state->score, state->coins,
         ecs_count(world, Enemy), tracker->health);
  assert(state->score > 0);

  // The keycard opens the door, and only then.
  const Level *level = LevelAt(0);
  assert(tracker->health > 0);
  ClearTheRoom(world);

  assert(ecs_count(world, Door) == 1);
  assert(WalkToTile(world, level, 'k', false));
  Idle(world, 2);
  assert(state->hasKeycard);
  Idle(world, 2);
  assert(ecs_count(world, Door) == 0);

  // Reaching the exit clears the level.
  ClearTheRoom(world);
  assert(WalkToTile(world, level, 'X', true));
  Idle(world, 2);
  assert(state->mode == MODE_LEVEL_CLEARED);
  int scoreAfterLevel = state->score;
  assert(scoreAfterLevel >= LEVEL_COMPLETE_BONUS);

  // Confirming moves on, and the new level is a fresh board.
  PressConfirm(world);
  ecs_progress(world, STEP);
  assert(state->mode == MODE_PLAYING);
  assert(state->levelIndex == 1);
  assert(!state->hasKeycard);
  assert(state->score == scoreAfterLevel);
  Idle(world, 1);
  assert(tracker->health == PLAYER_MAX_HEALTH);

  // A dead player stops the level rather than the whole program.
  ecs_get_mut(world, tracker->entity, Health)->current = 0;
  Idle(world, 2);
  assert(state->mode == MODE_GAME_OVER);

  PressConfirm(world);
  ecs_progress(world, STEP);
  Idle(world, 1);
  assert(state->mode == MODE_PLAYING);
  assert(state->levelIndex == 1);
  assert(tracker->health == PLAYER_MAX_HEALTH);

  // Pausing freezes the simulation, and Escape lets it go again.
  Input *input = ecs_singleton_get_mut(world, Input);
  memset(input, 0, sizeof(*input));
  input->pause = true;
  ecs_progress(world, STEP);
  assert(state->mode == MODE_PAUSED);
  assert(ClockIsPaused(world));

  restingPosition = tracker->position;
  Idle(world, 60);
  assert(Vec3Distance(restingPosition, tracker->position) < 0.01f);

  AppWorldDestroy(world);
  printf("all gameplay checks passed\n");
  return 0;
}
