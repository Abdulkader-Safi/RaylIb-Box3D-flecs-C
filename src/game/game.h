// The one thing main.c knows about the game.
#ifndef GAME_H
#define GAME_H

#include "flecs.h"

// Registers every component and system, then seeds the world. Called once by
// the framework, before the first frame.
void GameRegister(ecs_world_t *world);

// The same game with nothing that draws, for running it without a screen.
void GameRegisterHeadless(ecs_world_t *world);

#endif // GAME_H
