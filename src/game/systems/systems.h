// One register function per system file. game.c calls them in this order, and
// which phase each system runs in is decided inside its own file.
//
// Adding a system is: write the file, add its register function here, call it
// from GameRegister. Nothing else moves.
#ifndef GAME_SYSTEMS_H
#define GAME_SYSTEMS_H

#include "flecs.h"

void PlayerSystemRegister(ecs_world_t *world);   // Drive and aim the player.
void WeaponSystemRegister(ecs_world_t *world);   // Fire bullets on a cooldown.
void ChaseSystemRegister(ecs_world_t *world);    // Steer enemies at the player.
void BiteSystemRegister(ecs_world_t *world);     // Hurt the player on contact.
void CombatSystemRegister(ecs_world_t *world);   // Turn collisions into damage.
void DeathSystemRegister(ecs_world_t *world);    // Remove what ran out of health.
void WaveSystemRegister(ecs_world_t *world);     // Send in the next wave.
void RestartSystemRegister(ecs_world_t *world);  // Start the run over.
void CameraSystemRegister(ecs_world_t *world);   // Follow the player.
void RenderSystemRegister(ecs_world_t *world);   // Draw the world.
void HudSystemRegister(ecs_world_t *world);      // Draw the overlay.

#endif // GAME_SYSTEMS_H
