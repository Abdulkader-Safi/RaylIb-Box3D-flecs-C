// Every tunable number in the game lives here, so balancing never means
// hunting through the gameplay code.
#ifndef CONFIG_H
#define CONFIG_H

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define WINDOW_TITLE "Twin Stick Shooter - raylib + Box3D"

// Physics runs on a fixed step so behaviour does not change with frame rate.
#define PHYSICS_TIME_STEP (1.0f / 60.0f)
#define PHYSICS_SUB_STEPS 4
#define PHYSICS_MAX_ACCUMULATED 0.25f
#define PHYSICS_GRAVITY (-24.0f)

#define ARENA_HALF_EXTENT 24.0f
#define ARENA_WALL_HEIGHT 2.5f
#define ARENA_WALL_THICKNESS 1.0f

#define PLAYER_RADIUS 0.45f
#define PLAYER_HALF_HEIGHT 0.45f
#define PLAYER_SPEED 13.0f
#define PLAYER_MAX_HEALTH 100
#define PLAYER_FIRE_INTERVAL 0.11f

#define ENEMY_RADIUS 0.5f
#define ENEMY_HALF_HEIGHT 0.5f
#define ENEMY_SPEED 5.5f
#define ENEMY_MAX_HEALTH 30
#define ENEMY_TOUCH_DAMAGE 9
#define ENEMY_TOUCH_INTERVAL 0.7f
#define ENEMY_SPAWN_MARGIN 3.0f
#define MAX_ENEMIES 128

#define BULLET_RADIUS 0.18f
#define BULLET_SPEED 48.0f
#define BULLET_LIFETIME 1.4f
#define BULLET_DAMAGE 10
#define BULLET_KNOCKBACK 3.5f
#define MAX_BULLETS 256

#define WAVE_FIRST_COUNT 6
#define WAVE_COUNT_STEP 3
#define WAVE_BREAK_SECONDS 2.5f
#define SCORE_PER_KILL 100

#define CAMERA_HEIGHT 30.0f
#define CAMERA_BACK_OFFSET 17.0f
#define CAMERA_FOLLOW_RATE 8.0f

#endif // CONFIG_H
