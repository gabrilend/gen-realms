# 3-011a: WASM Core Canvas Infrastructure

## Current Behavior

Canvas initialization and 2D drawing primitives are implemented in JavaScript
(`assets/web/canvas.js`). The WASM module calls external JS functions via
`js-interop.c` for all canvas operations.

## Intended Behavior

Canvas initialization and 2D drawing primitives implemented in C using
Emscripten's HTML5 API. No external JavaScript file needed for canvas setup.

## Dependencies

- 3-003: WASM build configuration (complete)

## Suggested Implementation Steps

1. Create `src/client/wasm/canvas.h` with drawing API declarations
2. Create `src/client/wasm/canvas.c` implementing:
   - Canvas initialization via `emscripten/html5.h`
   - 2D drawing primitives using EM_ASM (inline JS)
   - Resize handling
   - Render loop via `emscripten_set_main_loop`
3. Create `src/client/wasm/draw2d.h` for primitive shapes
4. Create `src/client/wasm/draw2d.c` implementing:
   - `draw_rect()`, `draw_rect_outline()`
   - `draw_text()`, `draw_text_centered()`
   - `draw_line()`, `draw_arc()`
   - `draw_rounded_rect()`
5. Update main.c to use new canvas system

## Files to Create

- `src/client/wasm/canvas.h` - Canvas management declarations
- `src/client/wasm/canvas.c` - Canvas management implementation
- `src/client/wasm/draw2d.h` - 2D drawing primitive declarations
- `src/client/wasm/draw2d.c` - 2D drawing primitive implementation

## JS File Replaced

- `assets/web/canvas.js` (partial - layout handling)

## Acceptance Criteria

- [ ] Canvas initializes without external JS
- [ ] All drawing primitives work (rect, text, line, arc, roundRect)
- [ ] Canvas resizes correctly on window resize
- [ ] Render loop runs via `emscripten_set_main_loop`
- [ ] Colors specified as RGB uint32_t work correctly
