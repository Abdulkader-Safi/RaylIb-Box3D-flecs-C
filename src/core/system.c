#include "core/system.h"

void SystemRegisterImmediate(ecs_world_t *world, const char *name, ecs_entity_t phase,
                             ecs_iter_action_t callback, const char *terms) {
  ecs_entity_desc_t entityDesc = {0};
  entityDesc.name = name;

  ecs_system_desc_t desc = {0};
  desc.entity = ecs_entity_init(world, &entityDesc);
  desc.query.expr = terms;
  desc.phase = phase;
  desc.callback = callback;
  desc.immediate = true;

  ecs_system_init(world, &desc);
}
