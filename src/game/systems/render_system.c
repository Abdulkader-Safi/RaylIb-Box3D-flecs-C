// Drawing the world, one system per kind of shape.
//
// Nothing here knows what an enemy is. An entity is drawn as a capsule because
// it has a CapsuleVisual, so giving a new thing a body on screen is giving it
// a component, never editing this file.
#include "core/core.h"
#include "game/components/components.h"
#include "game/config.h"
#include "game/systems/systems.h"

static void GridSystem(ecs_iter_t *it) {
  (void)it;
  GfxDrawGrid((int)(ARENA_HALF_EXTENT * 2.0f), 1.0f, ARENA_GRID_HEIGHT, COLOR_GRID);
}

static void BoxRenderSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const BoxVisual *visuals = ecs_field(it, BoxVisual, 1);

  for (int i = 0; i < it->count; ++i) {
    GfxDrawBox(positions[i].value, visuals[i].size, visuals[i].color);
    if (visuals[i].wireframe) {
      GfxDrawBoxWires(positions[i].value, visuals[i].size, COLOR_WALL_EDGE);
    }
  }
}

static void CapsuleRenderSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const CapsuleVisual *visuals = ecs_field(it, CapsuleVisual, 1);

  for (int i = 0; i < it->count; ++i) {
    Vec3 centre = positions[i].value;
    Vec3 bottom = Vec3Make(centre.x, centre.y - visuals[i].halfHeight, centre.z);
    Vec3 top = Vec3Make(centre.x, centre.y + visuals[i].halfHeight, centre.z);
    GfxDrawCapsule(bottom, top, visuals[i].radius, visuals[i].color);
  }
}

static void SphereRenderSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const SphereVisual *visuals = ecs_field(it, SphereVisual, 1);

  for (int i = 0; i < it->count; ++i) {
    GfxDrawSphere(positions[i].value, visuals[i].radius, visuals[i].color);
  }
}

// A stub of a barrel and a line along the floor, so where the player is
// pointing reads at a glance.
static void AimRenderSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const Aim *aims = ecs_field(it, Aim, 1);

  for (int i = 0; i < it->count; ++i) {
    Vec3 centre = positions[i].value;
    Vec3 barrelStart = Vec3Add(centre, Vec3Scale(aims[i].direction, PLAYER_RADIUS));
    Vec3 barrelEnd = Vec3Add(centre, Vec3Scale(aims[i].direction, PLAYER_RADIUS + 0.7f));
    GfxDrawCapsule(barrelStart, barrelEnd, 0.12f, COLOR_BULLET);

    Vec3 floorStart = Vec3Make(centre.x, AIM_LINE_HEIGHT, centre.z);
    GfxDrawLine(floorStart, Vec3Add(floorStart, Vec3Scale(aims[i].direction, 14.0f)), COLOR_AIM);
  }
}

// Fades an enemy toward white as it takes damage, which is the only feedback a
// player gets that a target is nearly down.
static void HurtTintSystem(ecs_iter_t *it) {
  const Health *health = ecs_field(it, Health, 0);
  CapsuleVisual *visuals = ecs_field(it, CapsuleVisual, 1);

  for (int i = 0; i < it->count; ++i) {
    float wear = 1.0f - (float)health[i].current / (float)health[i].max;
    Color healthy = COLOR_ENEMY;
    Color hurt = COLOR_ENEMY_HURT;
    visuals[i].color = (Color){
        (unsigned char)(healthy.r + (hurt.r - healthy.r) * wear),
        (unsigned char)(healthy.g + (hurt.g - healthy.g) * wear),
        (unsigned char)(healthy.b + (hurt.b - healthy.b) * wear),
        255,
    };
  }
}

void RenderSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, HurtTintSystem, PhaseCamera, [in] Health, CapsuleVisual, Enemy);

  ECS_SYSTEM(world, GridSystem, PhaseDraw3D, 0);
  ECS_SYSTEM(world, BoxRenderSystem, PhaseDraw3D, [in] Position, [in] BoxVisual);
  ECS_SYSTEM(world, CapsuleRenderSystem, PhaseDraw3D, [in] Position, [in] CapsuleVisual);
  ECS_SYSTEM(world, SphereRenderSystem, PhaseDraw3D, [in] Position, [in] SphereVisual);
  ECS_SYSTEM(world, AimRenderSystem, PhaseDraw3D, [in] Position, [in] Aim);
}
