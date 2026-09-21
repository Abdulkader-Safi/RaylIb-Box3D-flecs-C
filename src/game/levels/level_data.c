// The maps. Each row is one line of the grid, all the same length.
#include "game/levels/levels.h"

// 01 Intake. One lane, a keycard detour, and the first locked door.
static const char *const LEVEL_01[] = {
    "########################",
    "#....l........#........#",
    "#..S..........D....m...#",
    "#....oo...c...#...oo...#",
    "#....oo.......#........#",
    "########.######....l...#",
    "#............#.........#",
    "#...k...+....#....c....#",
    "#............#.........#",
    "#....l.......#......X..#",
    "########################",
};

// 02 Reactor Ring. A ring of corridors around a solid core, with spawners in
// the corners feeding the fight while you look for the way out.
static const char *const LEVEL_02[] = {
    "##########################",
    "#L......c.........c.....H#",
    "#.m...................l..#",
    "#........................#",
    "#....################....#",
    "#....################....#",
    "#....###          ###....#",
    "#S...###          ###....#",
    "#....###          ###....#",
    "#....###          ###....#",
    "#....################....#",
    "#....################....#",
    "#..l..........k..........#",
    "#M......c.........+.....M#",
    "#........................#",
    "############D#############",
    "############.#############",
    "############X#############",
};

// 03 Foundry Split. Two lanes. The top one is open and quick, the bottom one
// is a grind through cover, and the keycard is at the far end of it.
static const char *const LEVEL_03[] = {
    "##############################",
    "#S...l......oo.......l......c#",
    "#...........oo...............#",
    "#....c..............m........#",
    "#######.################.#####",
    "#............#...............#",
    "#..r.........#......h........#",
    "#............#...............#",
    "#######.################.#####",
    "#..oo....M......oo...........#",
    "#..oo...........oo......k....#",
    "#.........h..................#",
    "##########################D###",
    "                         #..##",
    "                         #X.##",
    "                         #####",
};

// 04 Core Vault. A hub with two wings. Both hold a shield, the right one holds
// the keycard, and the vault at the top is where it ends.
static const char *const LEVEL_04[] = {
    "###############################",
    "#.............#...............#",
    "#....s........D........k......#",
    "#......H......#.......H.......#",
    "#.............#...............#",
    "####.##########..#########.####",
    "#.............................#",
    "#...m.....c.......c.....m.....#",
    "#.............................#",
    "####.####################.#####",
    "#........#          #.........#",
    "#...+....#          #....s....#",
    "#........#          #.........#",
    "#####.####          ####.######",
    "    #...#              #...#   ",
    "    #.S.#              #.X.#   ",
    "    #####              #####   ",
};

static const Level LEVELS[] = {
    {
        .name = "Intake",
        .brief = "Find the keycard, open the door, reach the exit.",
        .width = 24,
        .height = 11,
        .rows = LEVEL_01,
    },
    {
        .name = "Reactor Ring",
        .brief = "The corners keep sending more. Ring the core, take the card.",
        .width = 26,
        .height = 18,
        .rows = LEVEL_02,
    },
    {
        .name = "Foundry Split",
        .brief = "Two lanes. The slow one has the card.",
        .width = 30,
        .height = 16,
        .rows = LEVEL_03,
    },
    {
        .name = "Core Vault",
        .brief = "Both wings are guarded. Only one has the card.",
        .width = 31,
        .height = 17,
        .rows = LEVEL_04,
    },
};

int LevelCount(void) { return (int)(sizeof(LEVELS) / sizeof(LEVELS[0])); }

const Level *LevelAt(int index) {
  if (index < 0) index = 0;
  if (index >= LevelCount()) index = LevelCount() - 1;
  return LEVELS + index;
}
