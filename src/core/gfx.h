// Drawing, wrapped so game code never calls raylib.
//
// The framework opens the 3D pass before PhaseDraw3D and closes it before
// PhaseDrawUI, so a draw system just draws. Which direction the camera looks is
// the GameCamera singleton's business, and a game system owns that.
#ifndef CORE_GFX_H
#define CORE_GFX_H

#include "core/math.h"
#include "flecs.h"

void GfxRegister(ecs_world_t *world, Color clearColor);

// 3D, for systems in PhaseDraw3D.
void GfxDrawBox(Vec3 center, Vec3 size, Color color);
void GfxDrawBoxWires(Vec3 center, Vec3 size, Color color);
void GfxDrawSphere(Vec3 center, float radius, Color color);
void GfxDrawCapsule(Vec3 bottom, Vec3 top, float radius, Color color);
void GfxDrawLine(Vec3 from, Vec3 to, Color color);
void GfxDrawGrid(int slices, float spacing);

// 2D, for systems in PhaseDrawUI.
void GfxDrawText(const char *text, int x, int y, int size, Color color);
void GfxDrawTextCentered(const char *text, int y, int size, Color color);
void GfxDrawRect(int x, int y, int width, int height, Color color);
void GfxDrawRectLines(int x, int y, int width, int height, Color color);
void GfxDrawCircleLines(Vec2 center, float radius, Color color);
void GfxDrawLine2D(Vec2 from, Vec2 to, Color color);
void GfxDrawFps(int x, int y);

// A titled panel and a button, for menus and game over screens.
void GfxPanel(Rect bounds, const char *title);
bool GfxButton(Rect bounds, const char *label);

int GfxScreenWidth(void);
int GfxScreenHeight(void);
int GfxMeasureText(const char *text, int size);

// Printf for labels. The returned string is only valid until the next call.
const char *GfxFormat(const char *format, ...);

// Where a point on screen lands on a horizontal plane in the world. This is
// what turns a mouse position into somewhere to aim.
Vec3 GfxScreenPointOnPlane(ecs_world_t *world, Vec2 screenPoint, float planeY);

#endif // CORE_GFX_H
