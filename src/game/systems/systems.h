// One register function per system file. game.c calls them in this order, and
// which phase each system runs in is decided inside its own file.
//
// Adding a system is: write the file, add its register function here, call it
// from GameRegister. Nothing else moves.
#ifndef GAME_SYSTEMS_H
#define GAME_SYSTEMS_H

#include "flecs.h"

void ModeSystemRegister(ecs_world_t *world);     // Menus, pausing, level flow.
void SpawnerSystemRegister(ecs_world_t *world);  // Keep a level populated.
void PlayerSystemRegister(ecs_world_t *world);   // Drive and aim the player.
void WeaponSystemRegister(ecs_world_t *world);   // Fire bullets on a cooldown.
void ChaseSystemRegister(ecs_world_t *world);    // Steer enemies at the player.
void BiteSystemRegister(ecs_world_t *world);     // Hurt the player on contact.
void PickupSystemRegister(ecs_world_t *world);   // Coins, health, powerups, keycard.
void ExitSystemRegister(ecs_world_t *world);     // Standing on the exit wins.
void DoorSystemRegister(ecs_world_t *world);     // Open locked doors.
void CombatSystemRegister(ecs_world_t *world);   // Turn collisions into damage.
void DeathSystemRegister(ecs_world_t *world);    // Pay out and remove the dead.
void CameraSystemRegister(ecs_world_t *world);   // Follow the player.
void RenderSystemRegister(ecs_world_t *world);   // Draw the world.
void HudSystemRegister(ecs_world_t *world);      // Draw the overlay and menus.

#endif // GAME_SYSTEMS_H
