cmake_minimum_required(VERSION 3.22...3.27)
cmake_policy(SET CMP0135 NEW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Box3D turns on Objective-C for its macOS backend. CMake only honours that
# from the top level project, so the language is declared here.
set(GAME_LANGUAGES C)
if(APPLE)
  list(APPEND GAME_LANGUAGES OBJC)
endif()
project(game LANGUAGES ${GAME_LANGUAGES})

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------------------- dependencies
if(EMSCRIPTEN)
  # raylib picks its backend from this, and it has to be set before raylib is
  # brought in below. The browser supplies everything else, so no system
  # libraries are linked.
  set(PLATFORM Web CACHE STRING "raylib platform" FORCE)
  set(PLATFORM_LIBS "")
  # Box3D reaches for SSE2 on wasm through Emscripten's emulation headers. See
  # the note below FetchContent for why the scalar path is the one we want.
  set(BOX3D_DISABLE_SIMD ON CACHE BOOL "Box3D scalar math on the web" FORCE)
elseif(APPLE)
  set(PLATFORM_LIBS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
elseif(WIN32)
  set(PLATFORM_LIBS winmm gdi32 opengl32)
elseif(UNIX)
  set(PLATFORM_LIBS m pthread dl GL X11 Xrandr Xi Xxf86vm Xinerama Xcursor)
endif()

include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Box3D only builds its samples and tests when it is the top level project, so
# pulling it in this way gives just the library.
FetchContent_Declare(
    box3d
    URL https://github.com/erincatto/box3d/archive/refs/tags/v0.1.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(FLECS_SHARED OFF CACHE BOOL "" FORCE)
set(FLECS_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    flecs
    URL https://github.com/SanderMertens/flecs/archive/refs/tags/v4.1.6.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(raylib box3d flecs)

if(EMSCRIPTEN)
  # Box3D compiles itself with -msimd128 -msse2 on Emscripten. From -O2 up,
  # that walks LLVM's WebAssembly instruction selector into runaway recursion
  # in isKnownNeverNaN and clang dies on shape.c (Emscripten 6.0.3, clang 23).
  # -O0 is fine, and so is any level without the flags, which is what pins it
  # on the vectoriser rather than on the optimiser.
  #
  # So the web build takes Box3D's scalar path, which needs two things: the
  # BOX3D_DISABLE_SIMD definition set above, or it includes emmintrin.h and
  # fails to compile without -msse2, and the flags removed here, because
  # -msimd128 alone still lets the vectoriser rebuild the pattern that crashes.
  #
  # The game does not miss the SIMD: a few dozen bodies, not a few thousand.
  # Worth retrying when Emscripten updates.
  get_target_property(BOX3D_OPTIONS box3d COMPILE_OPTIONS)
  if(BOX3D_OPTIONS)
    list(REMOVE_ITEM BOX3D_OPTIONS -msimd128 -msse2)
    set_target_properties(box3d PROPERTIES COMPILE_OPTIONS "${BOX3D_OPTIONS}")
  endif()
endif()

# --------------------------------------------------------------------- sources
# core is the framework: the only code that talks to raylib, raygui and Box3D.
set(CORE_SOURCES
    src/core/app.c
    src/core/clock.c
    src/core/components.c
    src/core/gfx.c
    src/core/input.c
    src/core/lifetime.c
    src/core/math.c
    src/core/phases.c
    src/core/physics.c
    src/core/settings.c
    src/core/system.c
)

# game is the ECS: components, the systems that act on them, and the recipes
# that build entities.
set(GAME_SOURCES
    src/game/game.c
    src/game/components/components.c
    src/game/levels/level_build.c
    src/game/levels/level_data.c
    src/game/spawn/spawn.c
    src/game/systems/bite_system.c
    src/game/systems/camera_system.c
    src/game/systems/chase_system.c
    src/game/systems/combat_system.c
    src/game/systems/death_system.c
    src/game/systems/door_system.c
    src/game/systems/exit_system.c
    src/game/systems/hud_system.c
    src/game/systems/mode_system.c
    src/game/systems/pickup_system.c
    src/game/systems/player_system.c
    src/game/systems/render_system.c
    src/game/systems/spawner_system.c
    src/game/systems/weapon_system.c
)

add_executable(${PROJECT_NAME} src/main.c ${CORE_SOURCES} ${GAME_SOURCES})

# Headers are included by folder, for example "game/systems/systems.h".
target_include_directories(${PROJECT_NAME} PRIVATE src)
# raygui is vendored, so its warnings are not ours to fix.
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE include)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

target_link_libraries(${PROJECT_NAME} raylib box3d flecs_static ${PLATFORM_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ------------------------------------------------------------------------- web
if(EMSCRIPTEN)
  # Emscripten emits a page plus the .js and .wasm that sit beside it.
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
  target_link_options(${PROJECT_NAME} PRIVATE
      -sUSE_GLFW=3            # raylib talks to the canvas through GLFW.
      -sALLOW_MEMORY_GROWTH=1 # flecs and Box3D size their pools as they go.
      -sSTACK_SIZE=4194304    # The 64KB default is not enough for the solver.
      -sEXIT_RUNTIME=0
      --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html
  )
  # Rebuild the page when the shell changes, which CMake cannot see by itself.
  set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY
      LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
endif()

# ----------------------------------------------------------------------- tests
# Headless check of the ECS wiring and the gameplay rules. It opens no window.
# Build and run:  cmake --build build --target game_tests && ./build/bin/game_tests
if(NOT EMSCRIPTEN)
add_executable(game_tests EXCLUDE_FROM_ALL
    tests/test_gameplay.c
    ${CORE_SOURCES}
    ${GAME_SOURCES}
)
target_include_directories(game_tests PRIVATE src)
target_include_directories(game_tests SYSTEM PRIVATE include)
target_link_libraries(game_tests raylib box3d flecs_static ${PLATFORM_LIBS})
set_target_properties(game_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
endif()
