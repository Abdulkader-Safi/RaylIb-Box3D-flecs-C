// Vectors, angles and randomness for game code.
//
// The types are raylib's, renamed. That is deliberate: the framework hides
// raylib's API, not its perfectly good math, so there is no conversion layer
// and no second copy of vector math to keep in sync.
#ifndef CORE_MATH_H
#define CORE_MATH_H

#include "raylib.h"
#include "raymath.h"

typedef Vector2 Vec2;
typedef Vector3 Vec3;
typedef Rectangle Rect;
// Color and its named constants come straight from raylib.

#define VEC3_ZERO ((Vec3){0.0f, 0.0f, 0.0f})
#define VEC3_UP ((Vec3){0.0f, 1.0f, 0.0f})
#define VEC2_ZERO ((Vec2){0.0f, 0.0f})

static inline Vec2 Vec2Make(float x, float y) { return (Vec2){x, y}; }
static inline float Vec2Length(Vec2 v) { return Vector2Length(v); }
static inline Vec2 Vec2Normalize(Vec2 v) { return Vector2Normalize(v); }
static inline Vec2 Vec2Scale(Vec2 v, float s) { return Vector2Scale(v, s); }

static inline Vec3 Vec3Make(float x, float y, float z) { return (Vec3){x, y, z}; }
static inline Vec3 Vec3Add(Vec3 a, Vec3 b) { return Vector3Add(a, b); }
static inline Vec3 Vec3Sub(Vec3 a, Vec3 b) { return Vector3Subtract(a, b); }
static inline Vec3 Vec3Scale(Vec3 v, float s) { return Vector3Scale(v, s); }
static inline float Vec3Length(Vec3 v) { return Vector3Length(v); }
static inline float Vec3LengthSquared(Vec3 v) { return Vector3LengthSqr(v); }
static inline Vec3 Vec3Normalize(Vec3 v) { return Vector3Normalize(v); }
static inline float Vec3Distance(Vec3 a, Vec3 b) { return Vector3Distance(a, b); }
static inline Vec3 Vec3Lerp(Vec3 a, Vec3 b, float t) { return Vector3Lerp(a, b, t); }

// Drops the vertical component. Most of this game happens on a plane, so
// "which way is that, ignoring height" is asked constantly.
static inline Vec3 Vec3Flat(Vec3 v) { return (Vec3){v.x, 0.0f, v.z}; }

static inline float MathClamp(float value, float low, float high) {
  return value < low ? low : (value > high ? high : value);
}

// How far to move toward a target this frame for a given catch-up rate. The
// result does not change with frame rate, which a plain lerp factor would.
static inline float MathSmoothing(float rate, float dt) { return 1.0f - expf(-rate * dt); }

float RandomFloat(float low, float high);
float RandomAngle(void);
void RandomSeed(unsigned int seed);

#endif // CORE_MATH_H
