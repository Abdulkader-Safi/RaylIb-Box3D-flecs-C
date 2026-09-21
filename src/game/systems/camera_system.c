// Holds the camera a fixed distance above and behind the player, easing toward
// it so a hard change of direction does not snap the view.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/systems/systems.h"

static void CameraFollowSystem(ecs_iter_t *it) {
  const PlayerTracker *tracker = ecs_singleton_get(it->world, PlayerTracker);
  if (tracker->entity == 0) {
    return;
  }

  // Follow the player, but stop short of the level's edge. Clamping the target
  // rather than the camera keeps the framing honest: the player can still walk
  // into a corner, the view just stops trailing them out over the void.
  const LevelBounds *bounds = ecs_singleton_get(it->world, LevelBounds);
  Vec3 wanted = tracker->position;
  float limitX = bounds->half.x - CAMERA_EDGE_MARGIN_X;
  float limitZ = bounds->half.z - CAMERA_EDGE_MARGIN_Z;
  // A level narrower than twice the margin has no room to pan, so it centres.
  wanted.x = limitX > 0.0f ? MathClamp(wanted.x, -limitX, limitX) : 0.0f;
  wanted.z = limitZ > 0.0f ? MathClamp(wanted.z, -limitZ, limitZ) : 0.0f;

  GameCamera *camera = ecs_singleton_get_mut(it->world, GameCamera);
  camera->fieldOfView = CAMERA_FIELD_OF_VIEW;
  camera->target = Vec3Lerp(camera->target, wanted, MathSmoothing(CAMERA_FOLLOW_RATE, it->delta_time));
  camera->position = Vec3Add(camera->target, Vec3Make(0.0f, CAMERA_HEIGHT, CAMERA_BACK_OFFSET));
}

void CameraSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, CameraFollowSystem, PhaseCamera, 0);
}
