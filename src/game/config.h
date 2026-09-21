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

// Levels are drawn on a grid. One tile is this many metres, which is what
// turns a 24 x 16 sketch into a 48 x 32 metre room.
#define TILE_SIZE 2.0f
#define LEVEL_MAX_WIDTH 64
#define LEVEL_MAX_HEIGHT 48

#define ARENA_WALL_HEIGHT 3.0f
#define ARENA_COVER_HEIGHT 1.1f
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

// Three enemies, told apart by what they cost you to kill. Light ones rush and
// die fast, heavy ones soak damage and hit hard but cannot catch you.
#define ENEMY_LIGHT_RADIUS 0.40f
#define ENEMY_LIGHT_HEALTH 20
#define ENEMY_LIGHT_SPEED 7.5f
#define ENEMY_LIGHT_DAMAGE 6
#define ENEMY_LIGHT_COINS 1

#define ENEMY_MEDIUM_RADIUS 0.50f
#define ENEMY_MEDIUM_HEALTH 45
#define ENEMY_MEDIUM_SPEED 5.0f
#define ENEMY_MEDIUM_DAMAGE 10
#define ENEMY_MEDIUM_COINS 2

#define ENEMY_HEAVY_RADIUS 0.75f
#define ENEMY_HEAVY_HEALTH 110
#define ENEMY_HEAVY_SPEED 3.0f
#define ENEMY_HEAVY_DAMAGE 18
#define ENEMY_HEAVY_COINS 5

#define ENEMY_BITE_INTERVAL 0.7f

#define SPAWNER_INTERVAL 4.0f
#define SPAWNER_MAX_ALIVE 4

// Pickups drift toward the player once they are close, then vanish on touch.
#define PICKUP_RADIUS 0.35f
#define PICKUP_MAGNET_RANGE 4.0f
#define PICKUP_MAGNET_SPEED 9.0f
#define PICKUP_COLLECT_RANGE 1.0f
#define COIN_SCATTER 1.2f
#define HEALTH_PICKUP_AMOUNT 25

#define POWERUP_RAPID_INTERVAL 0.045f
#define POWERUP_RAPID_SECONDS 8.0f
#define POWERUP_SHIELD_SECONDS 8.0f

#define SCORE_PER_KILL 100
#define SCORE_PER_COIN 25
#define LEVEL_COMPLETE_BONUS 500

#define CAMERA_HEIGHT 30.0f
#define CAMERA_BACK_OFFSET 17.0f
#define CAMERA_FOLLOW_RATE 8.0f
#define CAMERA_FIELD_OF_VIEW 50.0f
// How far inside the level's edge the camera is allowed to look. Without this
// a player in a corner gets half a screen of empty space beside the level.
#define CAMERA_EDGE_MARGIN_X 14.0f
#define CAMERA_EDGE_MARGIN_Z 8.0f

#define COLOR_BACKGROUND ((Color){14, 16, 24, 255})
#define COLOR_FLOOR ((Color){26, 30, 42, 255})
#define COLOR_WALL ((Color){58, 68, 92, 255})
#define COLOR_WALL_EDGE ((Color){44, 52, 70, 255})
#define COLOR_GRID ((Color){48, 57, 78, 255})
#define COLOR_PANEL_TEXT ((Color){28, 32, 40, 255})
#define COLOR_PLAYER ((Color){90, 200, 255, 255})
#define COLOR_ENEMY ((Color){235, 84, 84, 255})
#define COLOR_ENEMY_LIGHT ((Color){245, 140, 90, 255})
#define COLOR_ENEMY_MEDIUM ((Color){235, 84, 84, 255})
#define COLOR_ENEMY_HEAVY ((Color){170, 58, 92, 255})
#define COLOR_COIN ((Color){255, 199, 64, 255})
#define COLOR_HEALTH ((Color){90, 220, 140, 255})
#define COLOR_RAPID ((Color){120, 190, 255, 255})
#define COLOR_SHIELD ((Color){190, 140, 255, 255})
#define COLOR_KEYCARD ((Color){232, 176, 72, 255})
#define COLOR_DOOR ((Color){150, 108, 44, 255})
#define COLOR_EXIT ((Color){80, 220, 190, 255})
#define COLOR_COVER ((Color){70, 80, 104, 255})
#define COLOR_SPAWNER ((Color){210, 80, 60, 255})
#define COLOR_MENU_DIM ((Color){10, 12, 18, 225})
#define COLOR_ENEMY_HURT ((Color){245, 220, 220, 255})
#define COLOR_BULLET ((Color){255, 214, 102, 255})
#define COLOR_AIM ((Color){255, 255, 255, 110})
#define COLOR_GOOD ((Color){90, 220, 140, 255})

#endif // GAME_CONFIG_H
