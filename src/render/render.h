// Draws the 3D scene. Owns nothing but the camera.
#ifndef RENDER_H
#define RENDER_H

#include "raylib.h"

struct Game; // Defined in core/game.h, which includes this header.

typedef struct Renderer {
  Camera3D camera;
} Renderer;

void RendererInit(Renderer *renderer, Vector3 target);

// Eases the camera toward the player so it never snaps on a hard turn.
void RendererFollow(Renderer *renderer, Vector3 target, float dt);

void RendererDrawScene(const Renderer *renderer, const struct Game *game);

#endif // RENDER_H
