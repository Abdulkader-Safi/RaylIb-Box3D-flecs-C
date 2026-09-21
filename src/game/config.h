// Every number you would want to change while balancing the game. None of
// these appear anywhere else in the code.
#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define WINDOW_TITLE "Twin Stick Shooter - raylib + Box3D + flecs"

// What can hit what. Each body gets one category bit and a mask of the bits it
// is willing to touch.
#define CATEGORY_ARENA (1ull << 0)
#define CATEGORY_PLAYER (1ull << 1)
#define CATEGORY_ENEMY (1ull << 2)
#define CATEGORY_BULLET (1ull << 3)

#define ARENA_HALF_EXTENT 24.0f
#define ARENA_WALL_HEIGHT 2.5f
#define ARENA_WALL_THICKNESS 1.0f
#define ARENA_FLOOR_HALF_THICKNESS 0.5f
// Ground markings, stacked clear of the floor and of each other.
//
// The gaps look absurd for something viewed from 30 metres up, and they have
// to be. Depth precision is spent near the camera: with a near plane of 1cm
// and a far plane a kilometre out, two surfaces a centimetre apart are the
// same depth by the far wall, and the floor and the grid trade pixels in
// bands. A tenth of a metre survives the whole arena and is invisible.
#define ARENA_FLOOR_VISUAL_SINK 0.05f // Drawn this far below where it collides.
#define ARENA_GRID_HEIGHT 0.05f
#define AIM_LINE_HEIGHT 0.12f

#define PLAYER_RADIUS 0.45f
#define PLAYER_HALF_HEIGHT 0.45f
#define PLAYER_SPEED 13.0f
#define PLAYER_MAX_HEALTH 100

#define WEAPON_INTERVAL 0.11f
#define WEAPON_DAMAGE 10
#define WEAPON_KNOCKBACK 3.5f
#define BULLET_RADIUS 0.18f
#define BULLET_SPEED 48.0f
#define BULLET_LIFETIME 1.4f

#define ENEMY_RADIUS 0.5f
#define ENEMY_HALF_HEIGHT 0.5f
#define ENEMY_SPEED 5.5f
#define ENEMY_MAX_HEALTH 30
#define ENEMY_BITE_DAMAGE 9
#define ENEMY_BITE_INTERVAL 0.7f
#define ENEMY_SPAWN_MARGIN 3.0f

#define WAVE_FIRST_COUNT 6
#define WAVE_COUNT_STEP 3
#define WAVE_BREAK_SECONDS 2.5f
#define SCORE_PER_KILL 100

#define CAMERA_HEIGHT 30.0f
#define CAMERA_BACK_OFFSET 17.0f
#define CAMERA_FOLLOW_RATE 8.0f
#define CAMERA_FIELD_OF_VIEW 50.0f

#define COLOR_BACKGROUND ((Color){14, 16, 24, 255})
#define COLOR_FLOOR ((Color){26, 30, 42, 255})
#define COLOR_WALL ((Color){58, 68, 92, 255})
#define COLOR_WALL_EDGE ((Color){44, 52, 70, 255})
#define COLOR_GRID ((Color){48, 57, 78, 255})
#define COLOR_PANEL_TEXT ((Color){28, 32, 40, 255})
#define COLOR_PLAYER ((Color){90, 200, 255, 255})
#define COLOR_ENEMY ((Color){235, 84, 84, 255})
#define COLOR_ENEMY_HURT ((Color){245, 220, 220, 255})
#define COLOR_BULLET ((Color){255, 214, 102, 255})
#define COLOR_AIM ((Color){255, 255, 255, 110})
#define COLOR_GOOD ((Color){90, 220, 140, 255})

#endif // GAME_CONFIG_H
