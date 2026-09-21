#include "game/components/components.h"

ECS_DECLARE(Player);
ECS_DECLARE(Enemy);
ECS_DECLARE(Bullet);
ECS_DECLARE(Spawned);

ECS_COMPONENT_DECLARE(Health);
ECS_COMPONENT_DECLARE(MoveSpeed);
ECS_COMPONENT_DECLARE(Aim);
ECS_COMPONENT_DECLARE(Weapon);
ECS_COMPONENT_DECLARE(Damage);
ECS_COMPONENT_DECLARE(Bite);
ECS_COMPONENT_DECLARE(BoxVisual);
ECS_COMPONENT_DECLARE(CapsuleVisual);
ECS_COMPONENT_DECLARE(SphereVisual);
ECS_COMPONENT_DECLARE(PlayerTracker);
ECS_COMPONENT_DECLARE(GameState);

void GameComponentsRegister(ecs_world_t *world) {
  ECS_TAG_DEFINE(world, Player);
  ECS_TAG_DEFINE(world, Enemy);
  ECS_TAG_DEFINE(world, Bullet);
  ECS_TAG_DEFINE(world, Spawned);

  ECS_COMPONENT_DEFINE(world, Health);
  ECS_COMPONENT_DEFINE(world, MoveSpeed);
  ECS_COMPONENT_DEFINE(world, Aim);
  ECS_COMPONENT_DEFINE(world, Weapon);
  ECS_COMPONENT_DEFINE(world, Damage);
  ECS_COMPONENT_DEFINE(world, Bite);
  ECS_COMPONENT_DEFINE(world, BoxVisual);
  ECS_COMPONENT_DEFINE(world, CapsuleVisual);
  ECS_COMPONENT_DEFINE(world, SphereVisual);
  ECS_COMPONENT_DEFINE(world, PlayerTracker);
  ECS_COMPONENT_DEFINE(world, GameState);

  ecs_singleton_set(world, PlayerTracker, {0});
  ecs_singleton_set(world, GameState, {0});
}
