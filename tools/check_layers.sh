#!/bin/sh
# Enforces the one rule the layout depends on: game code goes through core.
#
# src/game and src/main.c may not include an engine header or call an engine
# function. If this trips, whatever you reached for wants a core wrapper first.
set -eu

cd "$(dirname "$0")/.."
status=0
targets="src/game src/main.c"

headers='#include[[:space:]]*[<"](raylib|raymath|raygui|rlgl|flecs/|box3d/)'
if grep -rnE "$headers" $targets; then
  echo "FAIL: engine header included from game code (see above)."
  status=1
fi

# Entry points the framework wraps. A call to one of these from game code means
# the wrapper was bypassed.
calls='\b(InitWindow|CloseWindow|WindowShouldClose|BeginDrawing|EndDrawing|BeginMode3D'
calls="$calls"'|EndMode3D|ClearBackground|SetTargetFPS|SetConfigFlags|GetFrameTime'
calls="$calls"'|DrawText|DrawTextEx|DrawRectangle|DrawRectangleLines|DrawCircle|DrawCircleLines'
calls="$calls"'|DrawCircleLinesV|DrawLine|DrawLineV|DrawLine3D|DrawCube|DrawCubeV|DrawCubeWires'
calls="$calls"'|DrawCubeWiresV|DrawSphere|DrawCapsule|DrawGrid|DrawPlane|DrawFPS|MeasureText'
calls="$calls"'|TextFormat|GetScreenWidth|GetScreenHeight|GetScreenToWorldRay|IsKeyDown'
calls="$calls"'|IsKeyPressed|IsMouseButtonDown|IsMouseButtonPressed|GetMousePosition'
calls="$calls"'|IsGamepadAvailable|GetGamepadAxisMovement|IsGamepadButtonPressed|GetRandomValue'
calls="$calls"'|SetRandomSeed|Gui[A-Z][A-Za-z]*|b3[A-Z][A-Za-z]*|Vector2[A-Z][A-Za-z]*'
calls="$calls"'|Vector3[A-Z][A-Za-z]*)[[:space:]]*\('
if grep -rnE "$calls" $targets; then
  echo "FAIL: engine call from game code (see above)."
  status=1
fi

if [ "$status" -eq 0 ]; then
  echo "layers ok: src/game reaches raylib, raygui and Box3D only through src/core"
fi
exit $status
