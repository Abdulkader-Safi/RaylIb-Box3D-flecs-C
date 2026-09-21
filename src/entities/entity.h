// Shared tag every physics body carries, so a contact event can be traced back
// to the game object that owns the body.
#ifndef ENTITY_H
#define ENTITY_H

typedef enum EntityKind {
  ENTITY_NONE = 0,
  ENTITY_ARENA,
  ENTITY_PLAYER,
  ENTITY_ENEMY,
  ENTITY_BULLET,
} EntityKind;

typedef struct Entity {
  EntityKind kind;
  int index; // Slot in the pool that owns it, -1 for one-of-a-kind objects.
} Entity;

// Player, Enemy and Bullet all start with an Entity, so a body's user data
// pointer can be read as an Entity first and then cast to the real type.
#endif // ENTITY_H
