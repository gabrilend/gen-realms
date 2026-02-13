# Phase 3 Progress: Client Renderers

## Goal
Implement both terminal (ncurses) and browser (HTML5 Canvas) client renderers with unified input handling and client-side features.

## Status: IN PROGRESS (Architecture Revision)

## Architecture Note (2026-02-12)

The browser client approach has been reconsidered. JavaScript is architecturally
insecure (executes arbitrary remote code). The WASM implementation still requires
JavaScript bridging via Emscripten's EM_ASM for Canvas/WebSocket access.

**New direction:** Lua-based client (3-012) that users run locally with a Lua
interpreter they control. No JavaScript anywhere. Server sends JSON game state,
client renders using terminal escape codes or Love2D graphics.

The 3-011 WASM work is archived as reference material - the C rendering logic
documents visual design and can inform the Lua implementation.

## Issues

| ID | Description | Status |
|----|-------------|--------|
| 3-001 | Terminal Renderer | COMPLETE |
| 3-001a | ↳ Terminal UI initialization | COMPLETE |
| 3-001b | ↳ Terminal window rendering | COMPLETE |
| 3-001c | ↳ Terminal formatting | COMPLETE |
| 3-001d | ↳ Terminal input/resize | COMPLETE |
| 3-002 | Terminal Input System | COMPLETE |
| 3-003 | WebAssembly Build Configuration | COMPLETE |
| 3-004 | Browser Canvas Renderer | COMPLETE |
| 3-004a | ↳ Canvas Infrastructure | COMPLETE |
| 3-004b | ↳ Card Rendering | COMPLETE |
| 3-004c | ↳ Game Zone Rendering | COMPLETE |
| 3-004d | ↳ Status and Narrative Panels | COMPLETE |
| 3-005 | Browser Input Handler | COMPLETE |
| 3-006 | Client Style Preferences | COMPLETE |
| 3-006a | ↳ Preferences storage schema | COMPLETE |
| 3-006b | ↳ Preferences UI panel | COMPLETE |
| 3-006c | ↳ Export/import functionality | COMPLETE |
| 3-007 | Draw Order Interface | COMPLETE |
| 3-008 | Animation System | COMPLETE |
| 3-008a | ↳ Animation core and queue | COMPLETE |
| 3-008b | ↳ Card movement animations | COMPLETE |
| 3-008c | ↳ Attack/damage effects | COMPLETE |
| 3-009 | Narrative Display | COMPLETE |
| 3-010 | Phase 3 Demo | COMPLETE |
| 3-011 | WASM JS Elimination | ARCHIVED |
| 3-011a-j | ↳ Sub-issues (10 total) | ARCHIVED |
| 3-012 | Love2D Graphics Client | PENDING |
| 3-012a | ↳ Love2D Project Structure | PENDING |
| 3-012b | ↳ Thread Architecture | PENDING |
| 3-012c | ↳ Canvas API Wrapper | PENDING |
| 3-012d | ↳ Game Renderer | PENDING |

## Completed: 24/28 (4 pending, 10 archived)

## Technology Stack
- ncurses for terminal UI (native C client, complete)
- Love2D for graphical client (replaces browser Canvas)
  - love.graphics.* mirrors HTML5 Canvas API
  - love.thread for parallel computation (braided threads)
  - luasocket for TCP networking
- JSON for game state serialization
- TCP for network communication

### Archived Technologies
- HTML5 Canvas (requires JavaScript)
- localStorage (requires JavaScript)
- Emscripten/WASM (requires JS bridging)
- WebSocket in browser (requires JavaScript)

## Recent Progress

### 3-012: Love2D Graphics Client (2026-02-12)
Replaces HTML5 Canvas JavaScript API with Love2D. Terminal client already
exists (ncurses). This provides the graphical renderer:
- love.graphics.* mirrors Canvas API (fillRect, drawImage, etc.)
- love.thread enables parallel computation (network, state, render threads)
- Three threads braided via channels for smooth 60fps rendering
- No JavaScript anywhere in the stack

Server sends JSON game state via TCP. Love2D client parses, computes animation
diffs, and renders. Users can inspect all Lua scripts before running.

### 3-011: WASM JS Elimination (ARCHIVED)
Created 12 C module pairs (~3,500 lines) for pure WASM client. However, the
implementation still requires JavaScript bridging via Emscripten's EM_ASM.
Since the goal is to eliminate JavaScript entirely (not just avoid writing it),
this approach is archived. The C code remains as reference for visual design.

Files created (archived as reference):
- canvas.h/c, draw2d.h/c, theme.h/c, input.h/c
- card-renderer.h/c, zone-renderer.h/c, panel-renderer.h/c
- animation.h/c, websocket.h/c, preferences.h/c
- ai-hooks.h/c, game-client.h/c

### 3-010: Phase 3 Demo (COMPLETE)
Comprehensive demonstration of all Phase 3 client rendering components:
- Full ncurses terminal UI with split-screen layout
- Status bar displaying authority, D10/D4, trade, combat
- Hand rendering with faction-colored cards
- Trade row with affordability highlighting
- Base display for both players (frontier/interior)
- Scrollable narrative panel
- Command input with parsing (play, buy, attack, end)
- Terminal resize handling

Files created:
- src/demo/phase-3-demo.c - Main demo implementation
- scripts/run-phase3-demo.sh - Launch script

Run with: `./scripts/run-phase3-demo.sh` or `make demo3 && ./bin/phase-3-demo`

### Terminal Client Files
- src/client/01-terminal.c - ncurses initialization, window management
- src/client/01-terminal.h - TerminalUI structure, color pairs
- src/client/02-terminal-render.c - Rendering functions for all panels
- src/client/02-terminal-render.h - Render function declarations
- src/client/03-terminal-input.c - Command parsing, input history
- src/client/03-terminal-input.h - Command types, input structures

### Browser Client Files
- src/client/wasm/*.c - WebAssembly client modules
- assets/web/*.js - JavaScript modules for canvas rendering
- assets/web/index.html - Browser client entry point

## Notes
Phase 3 provides the visual layer for both client types. Terminal clients connect via SSH and use ncurses for display. Browser clients load WebAssembly and render to canvas with JavaScript animations.

The terminal demo showcases the ncurses-based client with all UI elements working. The browser client can be tested by opening assets/web/index.html after starting the server.

Key fixes made during demo development:
- Updated terminal-input.h to use GamePhase from core/05-game.h
- Fixed Deck structure access (frontier_bases/interior_bases vs bases)
- Added POSIX defines for strcasecmp functions
