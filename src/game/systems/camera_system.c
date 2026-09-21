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

  GameCamera *camera = ecs_singleton_get_mut(it->world, GameCamera);
  camera->fieldOfView = CAMERA_FIELD_OF_VIEW;
  camera->target =
      Vec3Lerp(camera->target, tracker->position, MathSmoothing(CAMERA_FOLLOW_RATE, it->delta_time));
  camera->position = Vec3Add(camera->target, Vec3Make(0.0f, CAMERA_HEIGHT, CAMERA_BACK_OFFSET));
}

void CameraSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, CameraFollowSystem, PhaseCamera, 0);
}
