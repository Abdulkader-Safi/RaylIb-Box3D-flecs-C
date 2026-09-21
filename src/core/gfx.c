#include "core/gfx.h"

// raygui ships as a single header. This is the one place it is compiled, and
// the only file in the project that knows it exists.
#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#include "core/components.h"
#include "core/phases.h"

#include <stdarg.h>
#include <stdio.h>

static Color g_clearColor;

static Camera3D CurrentCamera(ecs_world_t *world) {
  const GameCamera *view = ecs_singleton_get(world, GameCamera);
  Camera3D camera = {
      .position = view->position,
      .target = view->target,
      .up = VEC3_UP,
      .fovy = view->fieldOfView,
      .projection = CAMERA_PERSPECTIVE,
  };
  return camera;
}

// Opens the frame and the 3D pass, so every system in PhaseDraw3D can draw
// without knowing it is inside one.
static void GfxFrameBeginSystem(ecs_iter_t *it) {
  BeginDrawing();
  ClearBackground(g_clearColor);
  BeginMode3D(CurrentCamera(it->world));
}

// Closes the 3D pass before the overlay systems run.
static void GfxOverlaySystem(ecs_iter_t *it) {
  (void)it;
  EndMode3D();
}

static void GfxFrameEndSystem(ecs_iter_t *it) {
  (void)it;
  EndDrawing();
}

void GfxSetClearColor(Color color) { g_clearColor = color; }

void GfxRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, GfxFrameBeginSystem, PhaseFrameBegin, 0);
  ECS_SYSTEM(world, GfxOverlaySystem, PhaseOverlay, 0);
  ECS_SYSTEM(world, GfxFrameEndSystem, PhaseFrameEnd, 0);
}

void GfxDrawBox(Vec3 center, Vec3 size, Color color) { DrawCubeV(center, size, color); }

void GfxDrawBoxWires(Vec3 center, Vec3 size, Color color) {
  DrawCubeWiresV(center, size, color);
}

void GfxDrawSphere(Vec3 center, float radius, Color color) {
  DrawSphere(center, radius, color);
}

void GfxDrawCapsule(Vec3 bottom, Vec3 top, float radius, Color color) {
  DrawCapsule(bottom, top, radius, 12, 8, color);
}

void GfxDrawLine(Vec3 from, Vec3 to, Color color) { DrawLine3D(from, to, color); }

void GfxDrawGrid(int slices, float spacing, float height, Color color) {
  // raylib's DrawGrid is always at y = 0 and always its own colour, so the
  // lines are drawn here instead.
  float half = (float)slices * spacing * 0.5f;
  for (int i = 0; i <= slices; ++i) {
    float offset = -half + (float)i * spacing;
    DrawLine3D((Vector3){offset, height, -half}, (Vector3){offset, height, half}, color);
    DrawLine3D((Vector3){-half, height, offset}, (Vector3){half, height, offset}, color);
  }
}

void GfxDrawText(const char *text, int x, int y, int size, Color color) {
  DrawText(text, x, y, size, color);
}

void GfxDrawTextCentered(const char *text, int y, int size, Color color) {
  DrawText(text, (GetScreenWidth() - MeasureText(text, size)) / 2, y, size, color);
}

void GfxDrawRect(int x, int y, int width, int height, Color color) {
  DrawRectangle(x, y, width, height, color);
}

void GfxDrawRectLines(int x, int y, int width, int height, Color color) {
  DrawRectangleLines(x, y, width, height, color);
}

void GfxDrawCircleLines(Vec2 center, float radius, Color color) {
  DrawCircleLinesV(center, radius, color);
}

void GfxDrawLine2D(Vec2 from, Vec2 to, Color color) { DrawLineV(from, to, color); }

void GfxDrawFps(int x, int y) { DrawFPS(x, y); }

void GfxPanel(Rect bounds, const char *title) { GuiPanel(bounds, title); }

bool GfxButton(Rect bounds, const char *label) { return GuiButton(bounds, label); }

int GfxScreenWidth(void) { return GetScreenWidth(); }

int GfxScreenHeight(void) { return GetScreenHeight(); }

int GfxMeasureText(const char *text, int size) { return MeasureText(text, size); }

const char *GfxFormat(const char *format, ...) {
  static char buffer[512];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  return buffer;
}

Vec3 GfxScreenPointOnPlane(ecs_world_t *world, Vec2 screenPoint, float planeY) {
  Ray ray = GetScreenToWorldRay(screenPoint, CurrentCamera(world));
  // A ray parallel to the plane, or pointing away from it, never lands on it.
  if (fabsf(ray.direction.y) < 1e-4f) {
    return (Vec3){ray.position.x, planeY, ray.position.z};
  }
  float distance = (planeY - ray.position.y) / ray.direction.y;
  if (distance < 0.0f) {
    return (Vec3){ray.position.x, planeY, ray.position.z};
  }
  return Vec3Add(ray.position, Vec3Scale(ray.direction, distance));
}
