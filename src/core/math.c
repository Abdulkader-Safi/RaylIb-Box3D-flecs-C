#include "core/math.h"

float RandomFloat(float low, float high) {
  // GetRandomValue is integer only, so the range is sampled in thousandths.
  return low + (float)GetRandomValue(0, 1000) * 0.001f * (high - low);
}

float RandomAngle(void) { return (float)GetRandomValue(0, 359) * DEG2RAD; }

void RandomSeed(unsigned int seed) { SetRandomSeed(seed); }
