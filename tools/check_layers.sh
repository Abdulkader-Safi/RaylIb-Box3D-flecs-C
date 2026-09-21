#!/bin/sh
# Enforces the one rule the layout depends on: game code goes through core.
#
# src/game may not include raylib, raygui or Box3D headers, and may not call
# their functions. If this fails, the thing you reached for belongs in a core
# wrapper first.
set -eu

cd "$(dirname "$0")/.."
status=0

if grep -rnE '#include[[:space:]]*[<"](raylib|raymath|raygui|rlgl|box3d/)' src/game src/main.c; then
  echo "FAIL: the lines above include an engine header from game code."
  status=1
fi

if grep -rnE '\b(Draw[A-Z]|IsKeyDown|IsKeyPressed|IsMouseButton|GetMousePosition|BeginDrawing|EndDrawing|BeginMode3D|InitWindow|Gui[A-Z]|b3[A-Z])' src/game src/main.c; then
  echo "FAIL: the lines above call raylib, raygui or Box3D directly from game code."
  status=1
fi

if [ "$status" -eq 0 ]; then
  echo "layers ok: src/game reaches the engines only through src/core"
fi
exit $status
