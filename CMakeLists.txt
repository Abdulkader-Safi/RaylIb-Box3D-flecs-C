cmake_minimum_required(VERSION 3.16...3.27)
cmake_policy(SET CMP0135 NEW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(game)

set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the executable
add_executable(${PROJECT_NAME}
    src/main.c
)

# Set include directories after the executable is defined
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Set platform-specific dependencies
if(APPLE)
  set(PLATFORM_LIBS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
elseif(WIN32)
  set(PLATFORM_LIBS winmm gdi32 opengl32)
elseif(UNIX)
  set(PLATFORM_LIBS m pthread dl GL X11 Xrandr Xi Xxf86vm Xinerama Xcursor)
endif()

# Use FetchContent to download raylib if not already present
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(raylib)

target_link_libraries(${PROJECT_NAME} raylib ${PLATFORM_LIBS})

# Optional: Set the runtime output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
