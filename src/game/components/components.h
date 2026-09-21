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

// On everything a restart is allowed to wipe, which is every entity the game
// spawns. The framework's singletons do not have it and so survive.
extern ECS_DECLARE(Spawned);

// ---------------------------------------------------------------- data

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
  int score;
  int wave;
  float waveBreak; // Seconds left before the next wave walks in.
  bool over;

  // Set by the game over panel, read by the restart system on the next frame.
  //
  // It cannot go on Input. The panel is drawn in PhaseDrawUI at the end of a
  // frame, and PhaseInput overwrites the whole Input singleton at the start of
  // the next one, which is before PhaseSpawn would ever see the flag. A key
  // press has no such problem: it is polled and acted on in the same frame.
  bool restartRequested;
} GameState;

extern ECS_COMPONENT_DECLARE(Health);
extern ECS_COMPONENT_DECLARE(MoveSpeed);
extern ECS_COMPONENT_DECLARE(Aim);
extern ECS_COMPONENT_DECLARE(Weapon);
extern ECS_COMPONENT_DECLARE(Damage);
extern ECS_COMPONENT_DECLARE(Bite);
extern ECS_COMPONENT_DECLARE(BoxVisual);
extern ECS_COMPONENT_DECLARE(CapsuleVisual);
extern ECS_COMPONENT_DECLARE(SphereVisual);
extern ECS_COMPONENT_DECLARE(PlayerTracker);
extern ECS_COMPONENT_DECLARE(GameState);

void GameComponentsRegister(ecs_world_t *world);

#endif // GAME_COMPONENTS_H
