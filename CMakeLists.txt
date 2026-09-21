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

add_executable(${PROJECT_NAME}
    src/main.c
    src/core/game.c
    src/core/input.c
    src/physics/physics.c
    src/entities/player.c
    src/entities/enemy.c
    src/entities/bullet.c
    src/render/render.c
    src/render/hud.c
    src/world/arena.c
)

# Headers are included by folder, for example "entities/enemy.h".
target_include_directories(${PROJECT_NAME} PRIVATE src)
# raygui is vendored, so its warnings are not ours to fix.
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE include)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

if(APPLE)
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

FetchContent_MakeAvailable(raylib box3d)

target_link_libraries(${PROJECT_NAME} raylib box3d ${PLATFORM_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Headless check of the gameplay loop. It opens no window, so it runs anywhere.
# Build and run:  cmake --build build --target game_tests && ./build/bin/game_tests
add_executable(game_tests EXCLUDE_FROM_ALL
    tests/test_gameplay.c
    src/core/game.c
    src/core/input.c
    src/physics/physics.c
    src/entities/player.c
    src/entities/enemy.c
    src/entities/bullet.c
    src/render/render.c
    src/render/hud.c
    src/world/arena.c
)
target_include_directories(game_tests PRIVATE src)
target_include_directories(game_tests SYSTEM PRIVATE include)
target_link_libraries(game_tests raylib box3d ${PLATFORM_LIBS})
set_target_properties(game_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
