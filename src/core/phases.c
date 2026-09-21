#include "core/phases.h"

ecs_entity_t PhaseInput;
ecs_entity_t PhaseSpawn;
ecs_entity_t PhaseTrack;
ecs_entity_t PhaseLogic;
ecs_entity_t PhasePhysics;
ecs_entity_t PhaseSync;
ecs_entity_t PhasePostPhysics;
ecs_entity_t PhaseCleanup;
ecs_entity_t PhaseCamera;
ecs_entity_t PhaseFrameBegin;
ecs_entity_t PhaseDraw3D;
ecs_entity_t PhaseOverlay;
ecs_entity_t PhaseDrawUI;
ecs_entity_t PhaseFrameEnd;

static ecs_entity_t AddPhase(ecs_world_t *world, const char *name, ecs_entity_t after) {
  ecs_entity_t phase = ecs_new_w_id(world, EcsPhase);
  ecs_set_name(world, phase, name);
  ecs_add_pair(world, phase, EcsDependsOn, after);
  return phase;
}

void PhasesRegister(ecs_world_t *world) {
  // One unbroken chain hanging off EcsOnLoad, front to back. Each phase runs
  // after the one it depends on, so reading this function top to bottom is
  // reading the frame in order. Branching off several builtin phases instead
  // would leave the order between branches up to flecs.
  PhaseInput = AddPhase(world, "Input", EcsOnLoad);
  PhaseSpawn = AddPhase(world, "Spawn", PhaseInput);
  PhaseTrack = AddPhase(world, "Track", PhaseSpawn);
  PhaseLogic = AddPhase(world, "Logic", PhaseTrack);
  PhasePhysics = AddPhase(world, "Physics", PhaseLogic);
  PhaseSync = AddPhase(world, "Sync", PhasePhysics);
  PhasePostPhysics = AddPhase(world, "PostPhysics", PhaseSync);
  PhaseCleanup = AddPhase(world, "Cleanup", PhasePostPhysics);
  PhaseCamera = AddPhase(world, "Camera", PhaseCleanup);
  PhaseFrameBegin = AddPhase(world, "FrameBegin", PhaseCamera);
  PhaseDraw3D = AddPhase(world, "Draw3D", PhaseFrameBegin);
  PhaseOverlay = AddPhase(world, "Overlay", PhaseDraw3D);
  PhaseDrawUI = AddPhase(world, "DrawUI", PhaseOverlay);
  PhaseFrameEnd = AddPhase(world, "FrameEnd", PhaseDrawUI);
}
