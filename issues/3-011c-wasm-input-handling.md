# 3-011c: WASM Input Handling

## Current Behavior

Mouse and keyboard input handled by JavaScript (`input-handler.js`), which
translates browser events into game actions and calls WASM functions.

## Intended Behavior

All input handling implemented in C using Emscripten HTML5 callbacks. No
external JavaScript event handlers.

## Dependencies

- 3-011a: Core Canvas Infrastructure (provides canvas element ID)
- 3-011b: Theme and Layout Constants (provides Zone/Layout structures)

## Suggested Implementation Steps

1. Create `src/client/wasm/input.h` with:
   - Key code constants (arrow keys, enter, escape, letters, numbers)
   - Mouse button constants
   - MouseState structure (position, buttons, wheel)
   - KeyboardState structure (keys down, pressed, released, modifiers)
   - HitTarget structure for hit testing results

2. Create `src/client/wasm/input.c` implementing:
   - `input_init()` - Register Emscripten callbacks
   - `input_cleanup()` - Unregister callbacks
   - `input_update()` - Clear single-frame events
   - Mouse callback handlers (mousedown, mouseup, mousemove, wheel)
   - Keyboard callback handlers (keydown, keyup)
   - `input_hit_test()` - Zone hit testing
   - `input_get_card_at()` - Card index within zone

## Files Created

- `src/client/wasm/input.h` - Input state structures and API
- `src/client/wasm/input.c` - Input handling via Emscripten callbacks

## JS File Replaced

- `assets/web/input-handler.js`

## Acceptance Criteria

- [x] Mouse clicks detected via Emscripten callbacks
- [x] Keyboard input handled via Emscripten callbacks
- [x] Single-frame events (clicked, pressed, released) work
- [x] Hit testing returns correct zone and card index
- [x] Modifiers tracked (shift, ctrl, alt)
- [x] No external JS event handlers

## Implementation Notes

Uses emscripten/html5.h for:
- `emscripten_set_mousedown_callback()`
- `emscripten_set_mouseup_callback()`
- `emscripten_set_mousemove_callback()`
- `emscripten_set_wheel_callback()`
- `emscripten_set_keydown_callback()`
- `emscripten_set_keyup_callback()`

Input state polling model - callbacks update state, game code reads it.
