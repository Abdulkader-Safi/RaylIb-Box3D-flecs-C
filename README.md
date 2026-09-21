# Twin stick shooter

A top-down arena shooter in C, drawn with [raylib](https://www.raylib.com/) and
simulated with [Box3D](https://github.com/erincatto/box3d), Erin Catto's 3D
physics engine. Move with one hand, aim with the other, and let the solver
handle the shoving when a wave closes in.

## Controls

| Action  | Keyboard and mouse   | Gamepad     |
| ------- | -------------------- | ----------- |
| Move    | WASD or arrow keys   | Left stick  |
| Aim     | Mouse                | Right stick |
| Fire    | Left click or space  | Right stick |
| Restart | R                    | Start       |
| Quit    | Escape               |             |

## Build and run

Both raylib 5.5 and Box3D v0.1.0 are pulled in by CMake on the first configure,
so there is nothing to install first beyond CMake and a C compiler.

```sh
make          # configure, build, run
make build    # build only
make test     # run the headless gameplay checks
make clean    # delete the build directory
```

Or drive CMake yourself:

```sh
cmake -S . -B build
cmake --build build
./build/bin/game
```

## How the folders are laid out

Headers are included by folder, so `#include "entities/enemy.h"` reads the same
from anywhere in the tree.

```
src/
  main.c              Opens the window and runs update then draw. Nothing else.
  core/
    config.h          Every tunable number: speeds, damage, wave sizes, camera.
    game.h/.c         All game state, and the order things happen in a frame.
    input.h/.c        Keyboard, mouse and gamepad folded into one InputState.
  physics/
    physics.h/.c      The Box3D world: fixed stepping, body factories, filtering.
  entities/
    entity.h          The tag every physics body carries as its user data.
    player.h/.c       Movement, aim, weapon cooldown, health.
    enemy.h/.c        A pool of chasers that steer at the player.
    bullet.h/.c       A pool of projectiles that expire or get spent on a hit.
  render/
    render.h/.c       The 3D pass: arena, characters, bullets, camera follow.
    hud.h/.c          The 2D overlay and the game over panel, built with raygui.
  world/
    arena.h/.c        The static floor and four walls, plus enemy spawn points.
tests/
  test_gameplay.c     Headless run of the fire, hit, score and wave logic.
include/
  raygui.h            Vendored, used only by the HUD.
```

## How the physics is wired

Everything that exists in the world is a Box3D body, and every body carries a
pointer to an `Entity` as its user data. That is the whole trick: when a contact
event arrives holding two shape ids, `game.c` walks them back to bodies, reads
the two `Entity` tags, and knows a bullet just met an enemy.

Three body shapes cover the game:

- The floor and walls are static box hulls.
- The player and enemies are capsules with rotation locked on all three axes, so
  a body absorbs a shove without tipping over or spinning.
- Bullets are weightless spheres flagged `isBullet`, which turns on continuous
  collision so a fast shot cannot tunnel through a wall.

Characters are driven by writing horizontal velocity each frame and leaving the
vertical component to gravity. Direct control stays crisp, and the solver still
resolves every wall and every collision between bodies, which is what makes a
crowd of enemies pile up and squeeze around each other instead of overlapping.

Physics runs on a fixed 1/60 step with a leftover accumulator, so behaviour does
not drift with frame rate. Contact events describe only the step that just ran,
so `PhysicsStep` drains them after each one and hands them to a callback.
Nothing destroys a body from inside that callback: hits mark their target, and
`BulletPoolUpdate` and `EnemyPoolReap` do the removals once the step is over.

## Tuning it

Open `src/core/config.h`. Wave sizes, fire rate, knockback, camera height and
arena size are all there, and none of them appear anywhere else in the code.
