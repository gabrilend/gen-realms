# 3-011f: WASM Animation System

## Current Behavior

Animations handled by JavaScript (`animation.js`, `card-animations.js`, `effects.js`).
Supports card movement, fading, scaling, and particle effects with easing functions.

## Intended Behavior

Complete animation system in C with easing functions, animation queue, and
particle system. No external JavaScript for animations.

## Dependencies

- 3-011a: Core Canvas Infrastructure (provides draw2d API, render loop timing)

## Suggested Implementation Steps

1. Create `src/client/wasm/animation.h` with:
   - EasingType enum (linear, quad, cubic, back, bounce)
   - AnimationType enum (move, fade, scale, flip, shake, pulse)
   - Animation structure with timing, values, callback
   - Particle structure for effects
   - AnimationSystem global manager

2. Create `src/client/wasm/animation.c` implementing:
   - `ease_apply()` - All easing functions
   - `anim_init()` / `anim_cleanup()` - System lifecycle
   - `anim_update()` - Update all animations by delta time
   - `anim_create_move/fade/scale/flip/shake/pulse()` - Animation creators
   - `anim_set_callback()` - Completion callbacks
   - `anim_cancel()` / `anim_cancel_for_target()` - Cancel animations
   - `anim_get_position/alpha/scale()` - Query animated values
   - `anim_spawn_particles()` - Create particle burst
   - `anim_render_particles()` - Draw particles

## Files Created

- `src/client/wasm/animation.h` - Animation system API
- `src/client/wasm/animation.c` - Animation system implementation

## JS Files Replaced

- `assets/web/animation.js`
- `assets/web/card-animations.js`
- `assets/web/effects.js`

## Acceptance Criteria

- [x] Easing functions implemented (linear, quad, cubic, back, bounce)
- [x] Move animations work (card play, buy, draw)
- [x] Fade animations work (in/out)
- [x] Scale animations work (attention effects)
- [x] Shake animation works (damage/error feedback)
- [x] Pulse animation works (looping glow)
- [x] Completion callbacks fire correctly
- [x] Particle system for visual effects
- [x] Multiple simultaneous animations supported

## Implementation Notes

Animation pool with 32 slots (MAX_ANIMATIONS).
Particle pool with 64 slots (MAX_PARTICLES).

Easing functions:
- `ease_out_quad`: t * (2 - t)
- `ease_out_cubic`: (t-1)³ + 1
- `ease_out_back`: overshoot effect
- `ease_out_bounce`: bouncing effect

Particles use simple physics:
- Velocity + gravity
- Lifetime with fade
- Color and size variation
