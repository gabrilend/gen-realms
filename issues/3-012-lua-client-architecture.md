# 3-012: Love2D Graphics Client

## Current Behavior

Browser client requires JavaScript runtime (via Emscripten EM_ASM bridging).
JavaScript is inherently insecure - executes arbitrary remote code.
Terminal client (ncurses) already exists for text-based play.

## Intended Behavior

Love2D-based graphical client that replaces HTML5 Canvas JavaScript API.
No JavaScript anywhere. Server sends game state as JSON.
Love2D provides canvas-like drawing API plus multi-threading for parallel
computation (animations, rendering, network I/O braided together).

## Rationale

- Love2D wraps SDL2/OpenGL with Lua scripting
- `love.thread` allows parallel computation threads
- `love.graphics.*` mirrors Canvas API (rect, circle, line, image, text)
- Cross-platform (Windows, macOS, Linux, Android, iOS)
- Users can inspect all game scripts before running
- No remote code execution without explicit consent

## Architecture

```
Server (C)                     Love2D Client (Lua)
┌──────────────┐              ┌─────────────────────────────┐
│ Game Logic   │◄────TCP──────│ Network Thread              │
│ (Phase 1)    │              │ (luasocket)                 │
├──────────────┤              ├─────────────────────────────┤
│ Session Mgmt │──JSON state──►│ State Thread                │
│ (Phase 2)    │              │ (parse, diff, queue anims)  │
├──────────────┤              ├─────────────────────────────┤
│ Protocol     │              │ Render Thread (main)        │
│ (2-005)      │              │ (love.graphics.*, input)    │
└──────────────┘              └─────────────────────────────┘
                                       │
                              ┌────────┴────────┐
                              │ Thread Channels │
                              │ (braided data)  │
                              └─────────────────┘
```

## Dependencies

- 2-005: Protocol implementation (provides JSON gamestate format)
- 1-012: Gamestate serialization (complete)
- Love2D 11.x runtime

## Suggested Implementation Steps

### 3-012a: Love2D Project Structure

1. Create `src/client/love/conf.lua`:
   - Window configuration (resizable, vsync)
   - Module enables (thread, graphics, audio)
   - Identity for save directory

2. Create `src/client/love/main.lua`:
   - love.load() - Initialize threads, connect to server
   - love.update(dt) - Process thread channels, update animations
   - love.draw() - Render current game state
   - love.keypressed/mousepressed - Input handling

### 3-012b: Thread Architecture

1. Create `src/client/love/threads/network.lua`:
   - TCP connection via luasocket
   - JSON encode/decode
   - Send actions, receive state
   - Push to state channel on receive

2. Create `src/client/love/threads/state.lua`:
   - Receive from network channel
   - Parse JSON to Lua tables
   - Compute state diffs for animations
   - Push render-ready data to render channel

3. Thread synchronization:
   - love.thread.newChannel() for inter-thread communication
   - Non-blocking channel:pop() in main thread
   - Graceful shutdown via control channel

### 3-012c: Canvas API Wrapper

1. Create `src/client/love/canvas.lua` - Mirror HTML5 Canvas API:
   ```lua
   -- {{{ Canvas
   local Canvas = {}
   -- }}}

   -- {{{ fillRect
   function Canvas:fillRect(x, y, w, h)
       love.graphics.rectangle("fill", x, y, w, h)
   end
   -- }}}

   -- {{{ strokeRect
   function Canvas:strokeRect(x, y, w, h)
       love.graphics.rectangle("line", x, y, w, h)
   end
   -- }}}

   -- {{{ fillText
   function Canvas:fillText(text, x, y)
       love.graphics.print(text, x, y)
   end
   -- }}}

   -- {{{ drawImage
   function Canvas:drawImage(img, x, y, w, h)
       love.graphics.draw(img, x, y, 0, w/img:getWidth(), h/img:getHeight())
   end
   -- }}}

   -- {{{ setFillStyle
   function Canvas:setFillStyle(r, g, b, a)
       love.graphics.setColor(r, g, b, a or 1)
   end
   -- }}}
   ```

2. Port rendering from archived C code:
   - card-renderer → card.lua
   - zone-renderer → zones.lua
   - panel-renderer → panels.lua
   - animation → animation.lua

### 3-012d: Game Renderer

1. Create `src/client/love/render/card.lua`:
   - Card face with faction colors
   - Cost, attack, authority values
   - Selection highlight
   - Hover tooltip

2. Create `src/client/love/render/zones.lua`:
   - Hand (bottom, horizontal cards)
   - Trade row (center, purchasable)
   - Bases (left/right, player territories)
   - Play area (recently played cards)
   - Discard/deck piles

3. Create `src/client/love/render/panels.lua`:
   - Status bar (authority, resources)
   - Narrative panel (scrollable text)
   - Action buttons

4. Create `src/client/love/render/animation.lua`:
   - Easing functions (from archived C)
   - Card movement animations
   - Attack/damage effects
   - Animation queue with parallel execution

## Files to Create

```
src/client/love/
├── conf.lua              -- Love2D configuration
├── main.lua              -- Entry point, main loop
├── canvas.lua            -- HTML5 Canvas API wrapper
├── threads/
│   ├── network.lua       -- TCP/JSON communication
│   └── state.lua         -- State parsing, diff computation
├── render/
│   ├── card.lua          -- Card rendering
│   ├── zones.lua         -- Game zone layout
│   ├── panels.lua        -- UI panels
│   └── animation.lua     -- Animation system
├── game/
│   ├── state.lua         -- Game state data structures
│   ├── input.lua         -- Input handling, card selection
│   └── actions.lua       -- Action encoding for server
└── lib/
    └── json.lua          -- Pure Lua JSON parser
```

## Acceptance Criteria

- [ ] Love2D project runs and opens window
- [ ] Three threads: network, state, render (main)
- [ ] Connects to game server via TCP
- [ ] Receives and parses JSON gamestate
- [ ] Renders cards with faction colors
- [ ] Renders all game zones
- [ ] Mouse selection of cards
- [ ] Keyboard shortcuts (play, buy, attack, end)
- [ ] Sends actions back to server
- [ ] Animations for card movement
- [ ] No JavaScript anywhere

## Threading Model

Love2D's `love.thread` allows true parallel execution:

```
┌────────────┐     ┌────────────┐     ┌────────────┐
│  Network   │────►│   State    │────►│   Render   │
│  Thread    │     │   Thread   │     │  (main)    │
└────────────┘     └────────────┘     └────────────┘
      │                  │                  │
      └──────────────────┴──────────────────┘
                    Channels
              (thread-safe queues)
```

- Network thread: Blocking socket I/O, doesn't stall rendering
- State thread: JSON parsing, diff computation, animation planning
- Render thread: 60fps drawing, input handling (Love2D main thread)

Channels allow data to flow between threads without locks:
```lua
-- In network thread
state_channel:push(json_data)

-- In main thread
local data = state_channel:pop()  -- non-blocking
if data then process(data) end
```

## Comparison: HTML5 Canvas vs Love2D

| HTML5 Canvas | Love2D Equivalent |
|--------------|-------------------|
| ctx.fillRect(x,y,w,h) | love.graphics.rectangle("fill",x,y,w,h) |
| ctx.strokeRect(x,y,w,h) | love.graphics.rectangle("line",x,y,w,h) |
| ctx.fillText(s,x,y) | love.graphics.print(s,x,y) |
| ctx.drawImage(img,x,y) | love.graphics.draw(img,x,y) |
| ctx.fillStyle = color | love.graphics.setColor(r,g,b,a) |
| ctx.beginPath/arc/fill | love.graphics.circle("fill",x,y,r) |
| ctx.save/restore | love.graphics.push/pop() |
| Canvas element | love.graphics.newCanvas() |
| requestAnimationFrame | love.update(dt) + love.draw() |
| Web Workers (limited) | love.thread (full threading) |

## Notes

Love2D threading is more powerful than browser Web Workers:
- Shared memory via FFI (if needed)
- True parallel execution
- Channels for clean data passing
- No serialization overhead for simple types

The archived C WASM code provides reference implementations for:
- Zone layouts (theme.h defines positions)
- Card rendering (faction colors, layout)
- Animation easing functions
- Input handling patterns
