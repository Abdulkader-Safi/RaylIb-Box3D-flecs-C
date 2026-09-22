// Components the framework itself defines and drives. Game components live in
// game/components/components.h.
#ifndef CORE_COMPONENTS_H
#define CORE_COMPONENTS_H

#include "core/math.h"
#include "flecs.h"

#include <stdbool.h>
#include <stdint.h>

// Where a thing is. Anything with a PhysicsBody gets this written for it every
// frame during PhaseSync, so draw systems never talk to the physics engine.
typedef struct Position {
  Vec3 value;
} Position;

// A handle to a rigid body. The engine's own id is packed into an integer so
// that game headers never have to include the physics library.
typedef struct PhysicsBody {
  uint64_t handle;
} PhysicsBody;

// Counts down in seconds, then the entity is deleted. Bullets use it.
typedef struct Lifetime {
  float remaining;
} Lifetime;

// One frame of intent, as a singleton. Written by the framework in PhaseInput.
typedef struct Input {
  Vec2 move;      // x right, y forward. Length never above 1.
  Vec2 aimScreen; // Mouse position in pixels, when aiming with a mouse.
  Vec2 aimStick;  // Right stick direction, when aiming with a pad.
  bool aimIsStick;
  bool fire;
  bool restart;
  bool quit;

  // Menu intent. These are edges, true only on the frame the key went down,
  // so a held key does not run down a list of options.
  bool pause;
  bool confirm;
  bool click;      // Left button, the frame it went down.
  bool mouseMoved; // So hovering only steals the menu highlight when it moves.
  bool toggleDebug;
  bool menuUp;
  bool menuDown;
  bool menuLeft;
  bool menuRight;
} Input;

// Where the 3D pass looks from, as a singleton. A game system points it.
typedef struct GameCamera {
  Vec3 position;
  Vec3 target;
  float fieldOfView;
} GameCamera;

// The collisions that began during the last physics step, as a singleton.
// Refilled every frame, so read it in PhasePostPhysics or not at all.
#define MAX_CONTACTS_PER_FRAME 512

typedef struct Contact {
  ecs_entity_t a;
  ecs_entity_t b;
} Contact;

typedef struct Contacts {
  Contact items[MAX_CONTACTS_PER_FRAME];
  int count;
} Contacts;

extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(PhysicsBody);
extern ECS_COMPONENT_DECLARE(Lifetime);
extern ECS_COMPONENT_DECLARE(Input);
extern ECS_COMPONENT_DECLARE(GameCamera);
extern ECS_COMPONENT_DECLARE(Contacts);

void CoreComponentsRegister(ecs_world_t *world);

#endif // CORE_COMPONENTS_H
