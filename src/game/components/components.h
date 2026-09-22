// Everything the game adds on top of the framework's components.
//
// Adding a component is two lines here (the struct and the declaration) plus
// one in components.c. Nothing else in the project needs to change.
#ifndef GAME_COMPONENTS_H
#define GAME_COMPONENTS_H

#include "core/core.h"

// ---------------------------------------------------------------- tags
// Tags carry no data. They exist so a system can ask for "the enemies".

extern ECS_DECLARE(Player);
extern ECS_DECLARE(Enemy);
extern ECS_DECLARE(Bullet);
extern ECS_DECLARE(Wall);    // Anything solid the level is built from.
extern ECS_DECLARE(Exit);    // Step on it with the level cleared to move on.
extern ECS_DECLARE(Keycard); // Opens every locked door in the level.

// On everything a restart is allowed to wipe, which is every entity the game
// spawns. The framework's singletons do not have it and so survive.
extern ECS_DECLARE(Spawned);

// ---------------------------------------------------------------- data

// How a level is won, and what the run is worth so far.
typedef enum GameMode {
  MODE_MENU,
  MODE_PLAYING,
  MODE_PAUSED,
  MODE_SETTINGS,
  MODE_LEVEL_CLEARED,
  MODE_GAME_OVER,
  MODE_RUN_COMPLETE,
} GameMode;

typedef enum EnemyTier {
  ENEMY_LIGHT,
  ENEMY_MEDIUM,
  ENEMY_HEAVY,
} EnemyTier;

// What an enemy is currently doing about the player.
typedef enum AiState {
  AI_GUARD,       // Has noticed nothing. Holds its ground.
  AI_PATROL,      // Has noticed nothing, but will not stand still about it.
  AI_INVESTIGATE, // Heard something, or lost sight. Goes to look.
  AI_CHASE,       // Can see the player right now.
} AiState;

typedef enum PickupKind {
  PICKUP_COIN,
  PICKUP_HEALTH,
  PICKUP_RAPID_FIRE,
  PICKUP_SHIELD,
  PICKUP_KEYCARD,
} PickupKind;

typedef struct Health {
  int current;
  int max;
} Health;

typedef struct MoveSpeed {
  float value; // Metres per second.
} MoveSpeed;

// Which way the owner is pointing, flat on the ground plane.
typedef struct Aim {
  Vec3 direction;
} Aim;

typedef struct Weapon {
  float interval; // Seconds between shots.
  float cooldown; // Seconds still to wait.
  int damage;
  float knockback;
  float bulletSpeed;
  float bulletRadius;
  float bulletLifetime;
} Weapon;

// What a bullet does to the thing it reaches.
typedef struct Damage {
  int amount;
  float knockback;
} Damage;

// What an enemy can notice. Nothing reads the player's position directly.
typedef struct Senses {
  float sight;   // Needs a clear line as well as the range.
  float hearing; // Carries through walls.
  float memory;  // Seconds it keeps hunting after losing the player.
} Senses;

// What it is doing about what it noticed.
typedef struct Brain {
  AiState state;
  float alertTimer;  // Above zero means it still cares.
  float repathTimer; // Staggered so they do not all think on the same frame.
  Vec3 wanderTarget;
  bool wandering;
} Brain;

// The last thing anything heard, shared by every enemy. One gunshot should
// pull the whole room, not just whoever happened to be looking.
typedef struct Alert {
  Vec3 position;
  float timer;
  bool active;
} Alert;

// What an enemy is worth when it dies.
typedef struct Loot {
  EnemyTier tier;
  int coins;
  int score;
} Loot;

// Lies on the floor until the player gets close, then drifts over and is taken.
typedef struct Pickup {
  PickupKind kind;
  int amount;
} Pickup;

// A timed effect on the player. Zero means it is not running.
typedef struct Powerups {
  float rapidFire;
  float shield;
} Powerups;

// A locked door. It disappears once the player is carrying a keycard.
typedef struct Door {
  bool locked;
} Door;

// Drips enemies into the level while it has fewer than its share alive.
typedef struct Spawner {
  EnemyTier tier;
  float interval;
  float cooldown;
  int maxAlive;
} Spawner;

// A melee attack on a timer, for anything that hurts by touching.
typedef struct Bite {
  float interval;
  float cooldown;
  int damage;
  float range;
} Bite;

// ---------------------------------------------------------------- visuals
// A draw system exists per visual, so giving an entity a shape is giving it a
// component. No entity needs to know how it gets drawn.

typedef struct BoxVisual {
  Vec3 size;
  Color color;
  bool wireframe; // Draws an outline on top of the solid box.
} BoxVisual;

typedef struct CapsuleVisual {
  float radius;
  float halfHeight;
  Color color;
} CapsuleVisual;

typedef struct SphereVisual {
  float radius;
  Color color;
} SphereVisual;

// ---------------------------------------------------------------- singleton

// How far the level reaches, so the camera can stay over it.
typedef struct LevelBounds {
  Vec3 half; // Half the level's size in metres, centred on the origin.
} LevelBounds;

// Who and where the player is, refreshed every frame in PhaseTrack.
//
// Four systems want this and none of them should have to go looking: enemies
// steer at it, bites reach for it, the camera follows it and the HUD reads its
// health. Publishing it once is cheaper and shorter than four queries.
typedef struct PlayerTracker {
  ecs_entity_t entity; // Zero when there is no player.
  Vec3 position;
  int health;
  int maxHealth;
} PlayerTracker;

typedef struct GameState {
  GameMode mode;
  GameMode modeBeforeSettings; // So the settings screen knows where to return.
  int levelIndex;
  int score;
  int coins;
  float levelTime;
  float navTimer; // Counts down to the next rebuild of the shared routes.
  bool hasKeycard;
  int menuIndex;   // Which row of the current menu is highlighted.
  bool ignoreFireUntilRelease; // Set when a click dismissed a menu.
  bool over;

  // Set by the game over panel, read by the restart system on the next frame.
  //
  // It cannot go on Input. The panel is drawn in PhaseDrawUI at the end of a
  // frame, and PhaseInput overwrites the whole Input singleton at the start of
  // the next one, which is before PhaseSpawn would ever see the flag. A key
  // press has no such problem: it is polled and acted on in the same frame.
  bool restartRequested;
} GameState;

extern ECS_COMPONENT_DECLARE(Senses);
extern ECS_COMPONENT_DECLARE(Brain);
extern ECS_COMPONENT_DECLARE(Alert);
extern ECS_COMPONENT_DECLARE(Loot);
extern ECS_COMPONENT_DECLARE(Pickup);
extern ECS_COMPONENT_DECLARE(Powerups);
extern ECS_COMPONENT_DECLARE(Door);
extern ECS_COMPONENT_DECLARE(Spawner);
extern ECS_COMPONENT_DECLARE(Health);
extern ECS_COMPONENT_DECLARE(MoveSpeed);
extern ECS_COMPONENT_DECLARE(Aim);
extern ECS_COMPONENT_DECLARE(Weapon);
extern ECS_COMPONENT_DECLARE(Damage);
extern ECS_COMPONENT_DECLARE(Bite);
extern ECS_COMPONENT_DECLARE(BoxVisual);
extern ECS_COMPONENT_DECLARE(CapsuleVisual);
extern ECS_COMPONENT_DECLARE(SphereVisual);
extern ECS_COMPONENT_DECLARE(LevelBounds);
extern ECS_COMPONENT_DECLARE(PlayerTracker);
extern ECS_COMPONENT_DECLARE(GameState);

void GameComponentsRegister(ecs_world_t *world);

#endif // GAME_COMPONENTS_H
