# 3-012: Lua Client Architecture

## Current Behavior

Browser client requires JavaScript runtime (via Emscripten EM_ASM bridging).
JavaScript is inherently insecure - executes arbitrary remote code.
Users must trust browser vendors and every website's scripts.

## Intended Behavior

Lua-based client that users run locally with a Lua interpreter they control.
No JavaScript anywhere in the stack. Server sends game state as JSON/Lua tables.
Client renders locally using terminal escape codes or Love2D graphics.

## Rationale

- Lua interpreters are small (~50KB), auditable, and sandboxed
- Love2D provides graphics without browser dependency (~15MB)
- Users can inspect game scripts before running
- No remote code execution without explicit consent
- Aligns with project preference for Lua over other languages

## Architecture

```
Server (C)                     Client (Lua)
┌──────────────┐              ┌──────────────┐
│ Game Logic   │◄────TCP/WS───│ Lua Interp   │
│ (Phase 1)    │              │ or Love2D    │
├──────────────┤              ├──────────────┤
│ Session Mgmt │──JSON state──►│ State Parser │
│ (Phase 2)    │              │              │
├──────────────┤              ├──────────────┤
│ Protocol     │──Lua scripts─►│ Renderer     │
│ (2-005)      │  (once, 6MB) │ (term/gfx)   │
└──────────────┘              └──────────────┘
```

## Dependencies

- 2-005: Protocol implementation (provides JSON gamestate format)
- 1-012: Gamestate serialization (complete)

## Suggested Implementation Steps

### 3-012a: Lua Protocol Client

1. Create `src/client/lua/protocol.lua`:
   - TCP socket connection (LuaSocket or Love2D socket)
   - JSON parsing (cjson or pure Lua)
   - Message send/receive helpers
   - Reconnection handling

2. Create `src/client/lua/gamestate.lua`:
   - Parse JSON gamestate into Lua tables
   - Card, player, zone data structures
   - State diff tracking for animations

### 3-012b: Lua Terminal Renderer

1. Create `src/client/lua/terminal.lua`:
   - ANSI escape code output
   - Box drawing characters
   - Color support (16/256/true color detection)
   - Cursor positioning

2. Create `src/client/lua/term-ui.lua`:
   - Card rendering (ASCII art)
   - Zone layout (hand, trade row, bases)
   - Status bar and narrative panel
   - Input handling (blocking read)

### 3-012c: Lua Love2D Renderer (Optional)

1. Create `src/client/lua/love-ui.lua`:
   - Love2D main.lua entry point
   - Card rendering (sprites or vector)
   - Zone layout with mouse interaction
   - Animation system

2. Asset loading:
   - Card images (or generate from data)
   - Fonts and UI elements

### 3-012d: Script Distribution

1. Package client scripts:
   - Single-file bundle option
   - Versioned script downloads
   - Checksum verification

2. First-run experience:
   - Connect to server
   - Download client scripts
   - Display for user inspection
   - Run after confirmation

## Files to Create

- `src/client/lua/protocol.lua` - Network communication
- `src/client/lua/gamestate.lua` - State parsing
- `src/client/lua/terminal.lua` - ANSI rendering primitives
- `src/client/lua/term-ui.lua` - Terminal UI layout
- `src/client/lua/love-ui.lua` - Love2D UI (optional)
- `src/client/lua/main.lua` - Entry point

## Files to Archive

- `src/client/wasm/*` - WASM implementation (keep for reference)
- `assets/web/*.js` - Legacy JavaScript files

## Acceptance Criteria

- [ ] Lua client connects to server via TCP
- [ ] Receives and parses JSON gamestate
- [ ] Renders game state in terminal with ANSI codes
- [ ] Handles user input for card selection
- [ ] Sends actions back to server
- [ ] Optional Love2D graphical mode works
- [ ] No JavaScript anywhere in client

## Security Considerations

- Client scripts are static, versioned, inspectable
- No eval() or loadstring() on server data
- JSON gamestate is data only, not code
- Users can audit entire client before running
- Lua sandbox prevents filesystem/network abuse beyond game

## Comparison with Browser Approach

| Aspect | Browser (JS) | Lua Client |
|--------|--------------|------------|
| Install | None | Lua interp (~50KB) |
| Security | Runs any remote code | User controls execution |
| Audit | Minified, obfuscated | Plain text scripts |
| Graphics | Canvas (via JS) | Love2D (optional) |
| Terminal | Not possible | Native support |
| Size | ~200KB gzipped | ~6MB scripts + interpreter |

## Notes

The WASM/Emscripten work in 3-011 is not wasted:
- C rendering logic documents the visual design
- Can be compiled to native SDL2 client if desired
- Algorithm implementations port to Lua

Browser JavaScript is architecturally insecure - not a bug to fix,
but a fundamental design that trusts remote code. Lua clients
restore user agency over what code runs on their machine.
