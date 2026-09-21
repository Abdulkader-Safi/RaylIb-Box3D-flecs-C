# Twin stick shooter

A top-down arena shooter, and the small framework it runs on.

The split is the point. `src/core` is the framework: it owns the window, the
loop, the ECS world, drawing, input and rigid bodies, and it is the only code
that talks to [raylib](https://www.raylib.com/), raygui or
[Box3D](https://github.com/erincatto/box3d). `src/game` is the game: components,
the systems that act on them, and the recipes that build entities. Working on
the game means adding one of those three things and nothing else.

Entities, components and systems come from [flecs](https://www.flecs.dev/).

Four levels, drawn as grids of text. Find the keycard, open the door, reach the
exit. Kill things for coins, and pick up what they drop.

## Controls

| Action          | Keyboard and mouse   | Gamepad     |
| --------------- | -------------------- | ----------- |
| Move            | WASD or arrow keys   | Left stick  |
| Aim             | Mouse                | Right stick |
| Fire            | Left click           | Right stick |
| Pause           | Escape or P          | Start       |
| Menus           | W and S, Enter       | D-pad, A    |
| Restart level   | R                    |             |

Pushing the right stick both aims and fires, so it takes a firm push to
register. A stick resting slightly off centre would otherwise hold the trigger
down forever and outvote the mouse.

## Build and run

raylib 5.5, Box3D v0.1.0 and flecs v4.1.6 are fetched by CMake on the first
configure. You need CMake and a C compiler, nothing else.

```sh
make          # what each target does
make build    # build the desktop game
make run      # build and play it
make test     # layer check, then the headless gameplay checks
make web      # build for WebAssembly and pack dist/game-web.zip
make serve    # build for web and serve it on localhost:8000
make clean
```

## Putting it on itch.io

`make web` needs Emscripten on PATH (`brew install emscripten`, or an emsdk you
have sourced). It produces `dist/game-web.zip` with `index.html` at the root,
which is the shape itch.io wants.

Upload that zip, tick "This file will be played in the browser", and set the
viewport to 1280 x 720. Check it locally first with `make serve`: opening
`index.html` off disk will not work, because browsers refuse to fetch the
`.wasm` over `file://`.

Three things about the web build are load-bearing, all commented where they
live:

- The browser owns the loop. `AppRun` hands one frame over through
  `emscripten_set_main_loop` instead of running a `while`, because blocking
  would freeze the page.
- The canvas stays at its startup size, with no `FLAG_WINDOW_RESIZABLE`. If
  raylib resizes the canvas itself, Emscripten's GLFW carries on scaling mouse
  coordinates against the size it cached at startup, and aiming drifts away
  from the cursor. The page scales the canvas with CSS instead, keeping its
  16:9 shape so the pointer maps back by a plain scale.
- Box3D builds with its scalar maths. Its own Emscripten flags crash LLVM's
  WebAssembly instruction selector from `-O2` up.

## Layout

```
src/
  main.c                    Describes a window, hands over GameRegister. That is all.

  core/                     The framework. The only code that includes raylib or Box3D.
    core.h                  One include that brings in the whole framework.
    app.h/.c                Window, ECS world, the loop.
    phases.h/.c             The frame as an ordered chain of phases.
    components.h/.c         Position, PhysicsBody, Lifetime, Input, GameCamera, Contacts.
    physics.h/.c            Box3D behind a handle. Stepping, bodies, contacts as entity pairs.
    gfx.h/.c                Drawing calls, and the systems that open and close each pass.
    input.h/.c              Keyboard, mouse and pad folded into the Input singleton.
    lifetime.h/.c           Count down, then delete.
    system.h/.c             Registering a system that may change the world on the spot.
    math.h/.c               Vec2, Vec3, Rect, Color and the maths game code needs.

  game/                     The game. Never includes an engine header.
    config.h                Every number worth tuning.
    game.h/.c               Registers the components and systems, then seeds the world.
    components/             What entities can have.
    levels/                 The maps, drawn as text, and the builder that reads them.
    spawn/                  How each kind of entity is built.
    systems/                What happens each frame, one file per system.

tests/test_gameplay.c       Runs the real systems with no window attached.
tools/check_layers.sh       Fails if game code reaches past core.
include/raygui.h            Vendored, compiled once inside core/gfx.c.
```

## Adding to the game

Three moves cover almost everything.

**A component.** Add the struct and an `extern ECS_COMPONENT_DECLARE` to
`game/components/components.h`, then a `ECS_COMPONENT_DECLARE` and a
`ECS_COMPONENT_DEFINE` in `components.c`. Done.

**A system.** New file in `game/systems/`, one register function, add it to
`systems.h` and to the list in `game.c`. The phase it runs in is chosen inside
its own file:

```c
static void DriftSystem(ecs_iter_t *it) {
  const PhysicsBody *bodies = ecs_field(it, PhysicsBody, 0);
  const Drift *drift = ecs_field(it, Drift, 1);

  for (int i = 0; i < it->count; ++i) {
    PhysicsApplyImpulse(bodies[i], Vec3Scale(drift[i].direction, it->delta_time));
  }
}

void DriftSystemRegister(ecs_world_t *world) {
  ECS_SYSTEM(world, DriftSystem, PhaseLogic, [in] PhysicsBody, [in] Drift);
}
```

**A kind of entity.** A spawn function in `game/spawn/spawn.c` that asks the
framework for a body and attaches the components that describe it. Give it a
`CapsuleVisual` and it gets drawn; no drawing code changes.

A system that spawns or deletes entities is registered with
`SystemRegisterImmediate` instead of `ECS_SYSTEM`. The comment at the top of
`core/system.h` explains when that matters.

## The frame

`core/phases.c` is the whole order, top to bottom. Seven of the phases are for
game systems:

| Phase              | For                                            |
| ------------------ | ---------------------------------------------- |
| `PhaseSpawn`       | Create and destroy entities, waves, restarts   |
| `PhaseTrack`       | Publish facts other systems need, as singletons |
| `PhaseLogic`       | Read input, steer, decide                      |
| `PhasePostPhysics` | React to the collisions the step produced      |
| `PhaseCleanup`     | Remove what died this frame                    |
| `PhaseCamera`      | Point the camera                               |
| `PhaseDraw3D`      | Draw inside the 3D pass                        |
| `PhaseDrawUI`      | Draw the flat overlay                          |

The rest belong to the framework. It polls input, steps physics, copies body
positions into `Position`, and opens and closes the drawing passes around your
draw systems. A draw system just draws.

## How physics reaches the ECS

Bodies are described, not built by hand. `PhysicsCreateBody` takes a `BodyDesc`
and returns a `PhysicsBody`, which is the engine's id packed into an integer so
that no game header has to include Box3D. Three kinds cover the game: a static
box for scenery, an upright capsule with rotation locked for anything that
walks, and a weightless sphere with continuous collision for anything shot.

Two details do most of the work:

- The entity id rides along on the body as its user data. When Box3D reports
  that two shapes started touching, the framework turns that into a pair of
  entity ids in the `Contacts` singleton. `combat_system.c` reads that list and
  never learns what a shape is.
- Deleting an entity destroys its body, because the teardown hangs off the
  `PhysicsBody` component itself. No system has to remember, and wiping a whole
  run with one `ecs_delete_with` cleans up the physics world along with it.

Characters are driven by writing horizontal velocity each frame and leaving the
vertical part to gravity. Control stays crisp, and the solver still resolves
every wall and every body against every other, which is what makes a wave of
chasers pile up and squeeze around each other rather than overlap.

Physics runs on a fixed 1/60 step with a leftover accumulator, so behaviour does
not drift with frame rate. Contact events describe only the step that just ran,
so they are drained inside the stepping loop. Draining after it would lose every
hit that happened during a catch-up step.

## Writing a level

`src/game/levels/level_data.c`. A level is a grid of characters and a width and
height that must match it, which the headless test checks. One tile is two
metres.

```
#  wall            .  floor           (space) outside the level
o  low cover       S  start           X  exit
D  locked door     k  keycard
l  light enemy     m  medium enemy    h  heavy enemy
L  light spawner   M  medium spawner  H  heavy spawner
c  coin            +  health          r  rapid fire     s  shield
```

The maps live in C rather than in data files on purpose: the web build then has
nothing to fetch before it can start.

The test does more than check the shapes. For every level it runs a breadth
first search from the start, and fails if the keycard is behind the door it
opens or if the exit cannot be reached once that door is open. A level nobody
can finish fails there rather than in someone's hands.

## Tuning it

`src/game/config.h`. Wave sizes, fire rate, knockback, camera height, arena
size and the palette are all there, and none of those numbers appear anywhere
else in the code.
