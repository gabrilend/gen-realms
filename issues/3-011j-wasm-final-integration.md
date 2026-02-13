# 3-011j: WASM Final Integration

## Current Behavior

Game client entry point in JavaScript (`symbeline.js`) orchestrates all modules.
HTML loads 22 JS files and CSS. WASM module loaded as secondary component.

## Intended Behavior

Single game-client.c that integrates all WASM modules. Minimal index.html shell.
WASM is the primary component with Emscripten-generated loader as only JS.

## Dependencies

- 3-011a through 3-011i (all WASM modules)

## Suggested Implementation Steps

1. Create `src/client/wasm/game-client.h` with:
   - GameState enum (loading, menu, connecting, waiting, playing, game_over, error)
   - ActionMode enum (none, select_hand, select_trade, select_target, confirm)
   - GameClientConfig structure
   - GameData structure combining all zone/panel data

2. Create `src/client/wasm/game-client.c` implementing:
   - `game_init()` - Initialize all modules
   - `game_cleanup()` - Clean up all modules
   - `game_start()` - Start render loop
   - `game_stop()` - Stop render loop
   - `game_connect()` / `game_disconnect()` - Server connection
   - `game_send_action()` - Send player actions
   - `game_play_card()` / `game_buy_card()` / `game_attack_base()`
   - Main loop with input, update, render phases
   - WebSocket callbacks for game state updates
   - AI callbacks for narrative updates

3. Create entry point `game_main()` exported for HTML

## Files Created

- `src/client/wasm/game-client.h` - Main client API
- `src/client/wasm/game-client.c` - Main client implementation

## Files to Update

- `index.html` - Minimal shell (~30 lines)
- `Makefile.wasm` - All new source files

## JS Files Replaced

- `assets/web/symbeline.js` (main orchestration)
- All remaining JS files

## Acceptance Criteria

- [x] All modules integrate via game-client.c
- [x] Single entry point (game_main)
- [x] Main loop handles input, update, render
- [x] WebSocket callbacks update game state
- [x] AI callbacks update narrative
- [x] Default data for testing without server
- [x] index.html reduced to minimal shell (index-wasm.html created)
- [x] Makefile.wasm updated with all source files
- [ ] Build and run with pure WASM client (requires Emscripten SDK)
- [ ] Remove old JS files (cleanup script created)

## Implementation Notes

Module initialization order:
1. canvas_init() - Canvas and rendering
2. input_init() - Input handling
3. prefs_init() - Load preferences
4. anim_init() - Animation system
5. ws_init() - WebSocket with callbacks
6. ai_init() - AI hooks with callbacks

Main loop flow:
1. Calculate delta time
2. Update layout if canvas resized
3. handle_input() - Process mouse/keyboard
4. update(dt) - Update animations, WebSocket, AI
5. render() - Draw current state
6. input_update() - Clear single-frame events

Game state machine:
- LOADING → MENU (on init complete)
- MENU → CONNECTING (on connect)
- CONNECTING → WAITING (on ws connect)
- WAITING → PLAYING (on game state received)
- PLAYING → GAME_OVER (on win/loss)
- Any → ERROR (on fatal error)

## Remaining Work

1. ~~Update index.html to minimal shell~~ → Created index-wasm.html (30 lines)
2. ~~Update Makefile.wasm with all source files~~ → Done (12 WASM + 8 core + 1 lib = 21 source files)
3. Test full integration → Requires Emscripten SDK
4. ~~Document JS cleanup~~ → Created scripts/cleanup-legacy-js.sh

## Completed Items (2026-02-12)

- Created `assets/web/index-wasm.html` - Minimal 30-line HTML shell
- Updated `Makefile.wasm` with all new WASM source files
- Added WebSocket callback exports to EMCC_FLAGS
- Added `legacy` target for backward-compatible builds
- Added `size`, `serve`, and `sources` development targets
- Created `scripts/cleanup-legacy-js.sh` for JS file removal

## Build Instructions

```bash
# Install Emscripten SDK (one-time setup)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest

# Build pure WASM client
source emsdk_env.sh
make -f Makefile.wasm

# Test locally
make -f Makefile.wasm serve
# Open http://localhost:8000/assets/web/index-wasm.html

# Check build size
make -f Makefile.wasm size
```

## Files Changed

- `assets/web/index-wasm.html` - New minimal HTML shell
- `Makefile.wasm` - Updated with all source files
- `scripts/cleanup-legacy-js.sh` - Cleanup script for old JS files
