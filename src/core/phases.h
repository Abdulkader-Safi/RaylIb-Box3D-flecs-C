// The frame, as an ordered list of phases.
//
// flecs runs systems in phase order, and a phase is just an entity that depends
// on another one. Declaring the whole chain here means a system's place in the
// frame is chosen when you register it, not by which file happened to be
// compiled first.
//
// Game systems use these six:
//
//   PhaseSpawn        create and destroy entities, wave logic, restarts
//   PhaseTrack        publish facts later systems need, as singletons
//   PhaseLogic        read input, steer, decide
//   PhasePostPhysics  react to the collisions the step just produced
//   PhaseCleanup      remove what died this frame
//   PhaseCamera       point the camera
//   PhaseDraw3D       draw inside the 3D pass
//   PhaseDrawUI       draw the flat overlay on top
//
// The rest belong to the framework: they step physics, copy body positions into
// Position, and open and close the drawing passes around your draw systems.
#ifndef CORE_PHASES_H
#define CORE_PHASES_H

#include "flecs.h"

// For game systems.
extern ecs_entity_t PhaseSpawn;
extern ecs_entity_t PhaseTrack;
extern ecs_entity_t PhaseLogic;
extern ecs_entity_t PhasePostPhysics;
extern ecs_entity_t PhaseCleanup;
extern ecs_entity_t PhaseCamera;
extern ecs_entity_t PhaseDraw3D;
extern ecs_entity_t PhaseDrawUI;

// Framework only. Listed here because they sit in the same chain.
extern ecs_entity_t PhaseInput;       // Core polls the keyboard, mouse and pad.
extern ecs_entity_t PhasePhysics;     // Core steps Box3D and collects contacts.
extern ecs_entity_t PhaseSync;        // Core copies body positions into Position.
extern ecs_entity_t PhaseFrameBegin;  // Core opens the frame and the 3D pass.
extern ecs_entity_t PhaseOverlay;     // Core closes the 3D pass.
extern ecs_entity_t PhaseFrameEnd;    // Core closes the frame.

void PhasesRegister(ecs_world_t *world);

#endif // CORE_PHASES_H
