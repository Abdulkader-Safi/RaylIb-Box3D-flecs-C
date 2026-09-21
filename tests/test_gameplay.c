// Runs the real game with no screen attached and checks that the rules hold.
//
// This is the whole point of AppWorldCreate(false) and GameRegisterHeadless:
// the systems under test are the ones that ship, not a copy of them.
//
// Build and run:  make test
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/game.h"

#include <assert.h>
#include <stdio.h>

#define STEP (1.0f / 60.0f)

// Stands in for the input system, which needs a window. Aiming with the stick
// rather than the mouse also keeps the camera out of it: a stick gives a
// direction, so nothing has to be projected through a viewport.
static void PushInput(ecs_world_t *world, Vec2 aimStick, bool fire, bool restart) {
  Input *input = ecs_singleton_ensure(world, Input);
  input->move = VEC2_ZERO;
  input->aimIsStick = true;
  input->aimStick = aimStick;
  input->fire = fire;
  input->restart = restart;
}

// Points the weapon at whichever enemy the query hands back first.
static Vec2 AimAtAnEnemy(ecs_world_t *world, bool *found) {
  const PlayerTracker *tracker = ecs_singleton_get(world, PlayerTracker);
  ecs_query_t *query = ecs_query(world, {.terms = {{.id = ecs_id(Position), .inout = EcsIn},
                                                   {.id = Enemy}}});
  Vec2 aim = {0.0f, 1.0f};
  *found = false;

  ecs_iter_t it = ecs_query_iter(world, query);
  while (ecs_query_next(&it)) {
    if (it.count > 0) {
      const Position *positions = ecs_field(&it, Position, 0);
      Vec3 toEnemy = Vec3Normalize(Vec3Flat(Vec3Sub(positions[0].value, tracker->position)));
      // The stick's y axis points up the screen, which is negative Z.
      aim = Vec2Normalize(Vec2Make(toEnemy.x, -toEnemy.z));
      *found = true;
      ecs_iter_fini(&it);
      break;
    }
  }
  ecs_query_fini(query);
  return aim;
}

int main(void) {
  RandomSeed(20240921); // Spawn positions have to repeat run to run.

  ecs_world_t *world = AppWorldCreate(false);
  GameRegisterHeadless(world);

  const GameState *state = ecs_singleton_get(world, GameState);
  const PlayerTracker *tracker = ecs_singleton_get(world, PlayerTracker);

  // The world starts seeded: a player, and the arena it stands on.
  PushInput(world, VEC2_ZERO, false, false);
  ecs_progress(world, STEP);
  assert(tracker->entity != 0);
  assert(tracker->health == PLAYER_MAX_HEALTH);
  assert(state->wave == 1);
  assert(ecs_count(world, Enemy) == WAVE_FIRST_COUNT);
  assert(state->score == 0);

  // Ten seconds of standing still and shooting at whatever is closest.
  for (int frame = 0; frame < 600; ++frame) {
    bool found = false;
    Vec2 aim = AimAtAnEnemy(world, &found);
    PushInput(world, aim, found, false);
    ecs_progress(world, STEP);
  }

  printf("after 10s: score %d, wave %d, enemies %d, health %d, bullets %d\n", state->score,
         state->wave, ecs_count(world, Enemy), tracker->health, ecs_count(world, Bullet));

  // Bullets that reach an enemy have to kill it, and a kill has to pay out.
  assert(state->score >= WAVE_FIRST_COUNT * SCORE_PER_KILL);
  assert(state->score % SCORE_PER_KILL == 0);
  // Clearing a wave has to bring on the next one.
  assert(state->wave > 1);
  // Enemies that reach a player standing still have to hurt it.
  assert(tracker->health < PLAYER_MAX_HEALTH);

  // Bullets are not immortal: stop firing and the pool empties out.
  for (int frame = 0; frame < 180; ++frame) {
    PushInput(world, VEC2_ZERO, false, false);
    ecs_progress(world, STEP);
  }
  assert(ecs_count(world, Bullet) == 0);

  // A dead player ends the run and stops the waves.
  Health *health = ecs_get_mut(world, tracker->entity, Health);
  health->current = 0;
  PushInput(world, VEC2_ZERO, false, false);
  ecs_progress(world, STEP);
  assert(state->over);

  int waveAtDeath = state->wave;
  for (int frame = 0; frame < 300; ++frame) {
    PushInput(world, VEC2_ZERO, false, false);
    ecs_progress(world, STEP);
  }
  assert(state->wave == waveAtDeath);

  // The game over panel's button restarts too, and it is the harder path: it
  // is set while drawing at the end of a frame and has to survive PhaseInput
  // overwriting the Input singleton at the start of the next one.
  ecs_singleton_get_mut(world, GameState)->restartRequested = true;
  PushInput(world, VEC2_ZERO, false, false);
  ecs_progress(world, STEP);
  assert(!state->over);
  assert(!state->restartRequested);
  assert(tracker->entity != 0);
  assert(tracker->health == PLAYER_MAX_HEALTH);

  // And so does the key, from a fresh run this time.
  ecs_get_mut(world, tracker->entity, Health)->current = 0;
  PushInput(world, VEC2_ZERO, false, false);
  ecs_progress(world, STEP);
  assert(state->over);

  PushInput(world, VEC2_ZERO, false, true);
  ecs_progress(world, STEP);
  assert(!state->over);
  assert(state->score == 0);
  assert(state->wave == 1);
  assert(tracker->entity != 0);
  assert(tracker->health == PLAYER_MAX_HEALTH);
  assert(ecs_count(world, Enemy) == WAVE_FIRST_COUNT);

  AppWorldDestroy(world);
  printf("all gameplay checks passed\n");
  return 0;
}
