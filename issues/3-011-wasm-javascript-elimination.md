# 3-011: WebAssembly JavaScript/CSS Elimination

## Current Behavior

The browser client relies on 22 JavaScript files (~7,800 lines) and 1 CSS file
(594 lines) for rendering, input handling, animation, and UI. The WASM module
only handles game logic, with all browser interaction delegated to JavaScript.

**Current architecture:**
```
index.html
    ├── style.css (594 lines)
    ├── canvas.js
    ├── card-renderer.js
    ├── zone-renderer.js
    ├── panel-renderer.js
    ├── input-handler.js
    ├── animation.js
    ├── card-animations.js
    ├── effects.js
    ├── narrative.js
    ├── preferences.js
    ├── preferences-ui.js
    ├── draw-order.js
    ├── art-tracker.js
    ├── style-merger.js
    ├── generation-queue.js
    ├── image-cache.js
    ├── upgrade-viz.js
    ├── battle-canvas.js
    ├── region-selector.js
    ├── scene-composition.js
    ├── style-transfer.js
    └── symbeline.js (WASM loader)
```

## Intended Behavior

Pure WebAssembly client with no external JavaScript or CSS dependencies.
Only Emscripten's auto-generated loader JS (which bootstraps the WASM) remains.

**Target architecture:**
```
index.html (minimal shell, ~30 lines)
    └── symbeline.js (Emscripten-generated WASM loader)
        └── symbeline.wasm (all rendering, input, UI in C)
```

**Benefits:**
- Single binary distribution
- No JS parsing/JIT overhead
- Consistent behavior across browsers
- Easier debugging (single codebase)
- Smaller total download (~140 KB vs current ~80 KB JS + WASM)

## Suggested Implementation Steps

### Sub-Issue 3-011a: Core Canvas Infrastructure

**Files to create:**
- `src/client/wasm/canvas.c` - Canvas initialization and context management
- `src/client/wasm/draw2d.c` - 2D drawing primitives (rect, text, line, circle)
- `src/client/wasm/draw2d.h` - Drawing API declarations

**JS files replaced:** `canvas.js`

**Key implementation details:**

1. Use `emscripten/html5.h` for canvas access:
```c
#include <emscripten/html5.h>

int canvas_init(const char* canvas_id) {
    emscripten_get_canvas_element_size(canvas_id, &width, &height);
    /* ... */
}
```

2. Use `EM_ASM` for 2D context calls (inlined, no external JS):
```c
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    EM_ASM({
        var ctx = Module.canvas.getContext('2d');
        ctx.fillStyle = '#' + ($4).toString(16).padStart(6, '0');
        ctx.fillRect($0, $1, $2, $3);
    }, x, y, w, h, color);
}
```

3. Alternative: Use WebGL2 for hardware acceleration:
```c
EmscriptenWebGLContextAttributes attrs;
emscripten_webgl_init_context_attributes(&attrs);
attrs.majorVersion = 2;
gl_ctx = emscripten_webgl_create_context(canvas_id, &attrs);
```

**Acceptance criteria:**
- [ ] Canvas initializes without external JS
- [ ] All drawing primitives work (rect, text, line, arc, roundRect)
- [ ] Canvas resizes correctly on window resize
- [ ] Render loop runs via `emscripten_set_main_loop`

---

### Sub-Issue 3-011b: Theme and Layout Constants

**Files to create:**
- `src/client/wasm/theme.h` - Color definitions as RGB uint32_t
- `src/client/wasm/layout.h` - Zone positions, spacing, font sizes

**CSS sections replaced:** All of `style.css`

**Key implementation details:**

1. Colors as compile-time constants:
```c
#define RGB(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (b))

static const Color COLOR_BG_DARK = RGB(0x0d, 0x0d, 0x1a);
static const Color COLOR_FACTION_MERCHANT = RGB(0xd4, 0xa0, 0x17);
```

2. Layout as percentage-based zones:
```c
typedef struct { int x, y, w, h; } ZonePercent; /* 0-100 */
static const ZonePercent ZONE_HAND = { 0, 72, 100, 28 };
```

3. Font handling - use browser's default monospace:
```c
EM_ASM({ Module.canvas.getContext('2d').font = '14px monospace'; });
```

**Acceptance criteria:**
- [ ] All faction colors defined
- [ ] All value colors (authority, combat, trade) defined
- [ ] Zone layout matches current JS implementation
- [ ] No CSS file required

---

### Sub-Issue 3-011c: Input Handling

**Files to create:**
- `src/client/wasm/input.c` - Mouse and keyboard event handling
- `src/client/wasm/input.h` - Input state queries

**JS files replaced:** `input-handler.js`

**Key implementation details:**

1. Use Emscripten HTML5 callbacks:
```c
#include <emscripten/html5.h>

emscripten_set_mousedown_callback(canvas_id, NULL, EM_TRUE, mouse_cb);
emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, EM_TRUE, key_cb);
```

2. Track input state for polling:
```c
typedef struct {
    int mouse_x, mouse_y;
    int mouse_clicked;  /* Set on mouseup, cleared after frame */
    char keys_pressed[256];  /* Set on keydown, cleared after frame */
    char keys_down[256];     /* Held state */
} InputState;
```

3. Hit testing for zones/cards:
```c
int input_hit_test_zone(int x, int y);  /* Returns zone ID or -1 */
int input_hit_test_card(int x, int y);  /* Returns card index or -1 */
```

**Acceptance criteria:**
- [ ] Mouse clicks detected and translated to game actions
- [ ] Keyboard shortcuts work (Enter, Escape, number keys)
- [ ] Hover states tracked for tooltips
- [ ] No external JS event handlers

---

### Sub-Issue 3-011d: Card and Zone Rendering

**Files to create:**
- `src/client/wasm/card-renderer.c` - Card drawing with factions, effects, badges
- `src/client/wasm/zone-renderer.c` - Trade row, hand, bases, play area

**JS files replaced:** `card-renderer.js`, `zone-renderer.js`

**Key implementation details:**

1. Card rendering function:
```c
void card_render(const Card* card, int x, int y, int w, int h, CardState state) {
    Color faction_color = get_faction_color(card->faction);
    draw_rect(x, y, w, h, COLOR_BG_CARD);
    draw_rect_outline(x, y, w, h, faction_color, 2);
    draw_text(x + 4, y + 16, card->name, faction_color);
    /* Cost, type, effects, upgrade badges... */
}
```

2. Zone rendering with click bounds tracking:
```c
typedef struct {
    int x, y, w, h;
    int card_index;
    ZoneType zone;
} ClickBound;

static ClickBound click_bounds[MAX_CLICK_BOUNDS];
static int click_bound_count;

void zone_render_trade_row(Card** cards, int count, Zone* zone) {
    /* Render cards and register click bounds */
}
```

**Acceptance criteria:**
- [ ] Cards render with correct faction colors
- [ ] Upgrade badges display (attack, trade, authority bonuses)
- [ ] Selected/hover states render correctly
- [ ] Click bounds tracked for all interactive elements

---

### Sub-Issue 3-011e: Panel Rendering

**Files to create:**
- `src/client/wasm/panel-renderer.c` - Status bar and narrative panel

**JS files replaced:** `panel-renderer.js`, `narrative.js`

**Key implementation details:**

1. Status bar with game state:
```c
void panel_render_status(GameState* state, Zone* zone) {
    char buf[64];
    snprintf(buf, sizeof(buf), "Turn %d | %s", state->turn, phase_name(state->phase));
    draw_text(zone->x + 10, zone->y + 20, buf, COLOR_TEXT_PRIMARY);
    /* Authority, trade, combat values... */
}
```

2. Narrative panel with word wrapping:
```c
void panel_render_narrative(NarrativeBuffer* buf, Zone* zone) {
    int y = zone->y + 10;
    for (int i = buf->scroll; i < buf->count && y < zone->y + zone->h; i++) {
        /* Word wrap and render each line */
        y += render_wrapped_text(zone->x + 10, y, zone->w - 20, buf->entries[i]);
    }
}
```

3. Narrative buffer management:
```c
typedef struct {
    char* entries[MAX_NARRATIVE_ENTRIES];
    int count;
    int scroll;
} NarrativeBuffer;
```

**Acceptance criteria:**
- [ ] Status bar shows turn, phase, resources
- [ ] Narrative panel displays LLM text with word wrapping
- [ ] Scrolling works for long narratives
- [ ] Different entry types styled (action, system, combat)

---

### Sub-Issue 3-011f: Animation System

**Files to create:**
- `src/client/wasm/animation.c` - Animation queue and easing
- `src/client/wasm/animation.h` - Animation types and API

**JS files replaced:** `animation.js`, `card-animations.js`, `effects.js`

**Key implementation details:**

1. Easing functions:
```c
typedef float (*EasingFn)(float t);

static float ease_out_quad(float t) { return t * (2.0f - t); }
static float ease_in_out_cubic(float t) {
    return t < 0.5f ? 4*t*t*t : 1 - powf(-2*t + 2, 3) / 2;
}
```

2. Animation structure:
```c
typedef struct Animation {
    AnimationType type;
    float start[4], end[4];  /* Position/color values */
    float duration_ms, elapsed_ms;
    EasingFn easing;
    void* target;
    struct Animation* next;
} Animation;
```

3. Queue management:
```c
void animation_queue_add(AnimationQueue* q, Animation* anim);
void animation_queue_update(AnimationQueue* q, float dt_ms);
void animation_queue_render(AnimationQueue* q);  /* Render active anims */
```

**Acceptance criteria:**
- [ ] Card movement animations (play, buy, draw, scrap)
- [ ] Attack/damage visual effects
- [ ] Turn transition effects
- [ ] Animation speed respects user preferences

---

### Sub-Issue 3-011g: WebSocket Communication

**Files to create:**
- `src/client/wasm/websocket.c` - WebSocket connection and messaging

**JS files replaced:** WebSocket portions of `symbeline.js`

**Key implementation details:**

1. Use Emscripten WebSocket library:
```c
#include <emscripten/websocket.h>

EMSCRIPTEN_WEBSOCKET_T ws;

int websocket_connect(const char* url) {
    EmscriptenWebSocketCreateAttributes attrs = {
        .url = url,
        .protocols = NULL,
        .createOnMainThread = EM_TRUE
    };
    ws = emscripten_websocket_new(&attrs);
    emscripten_websocket_set_onmessage_callback(ws, NULL, on_message);
    return ws > 0 ? 0 : -1;
}
```

2. Message handling:
```c
static EM_BOOL on_message(int type, const EmscriptenWebSocketMessageEvent* e, void* data) {
    if (e->isText) {
        protocol_handle_message((const char*)e->data);
    }
    return EM_TRUE;
}
```

3. Reconnection logic in C:
```c
static int reconnect_attempts = 0;
static const int MAX_RECONNECT = 5;
static const int RECONNECT_DELAY_MS = 3000;
```

**Build flag:** `-lwebsocket.js`

**Acceptance criteria:**
- [ ] WebSocket connects to game server
- [ ] Messages sent/received correctly
- [ ] Automatic reconnection on disconnect
- [ ] Connection status exposed to UI

---

### Sub-Issue 3-011h: Preferences Storage

**Files to create:**
- `src/client/wasm/storage.c` - localStorage access
- `src/client/wasm/preferences.c` - Preference loading/saving

**JS files replaced:** `preferences.js`, `preferences-ui.js`

**Key implementation details:**

1. localStorage via EM_ASM:
```c
void storage_set(const char* key, const char* value) {
    EM_ASM({ localStorage.setItem(UTF8ToString($0), UTF8ToString($1)); }, key, value);
}

char* storage_get(const char* key) {
    int len = EM_ASM_INT({
        var v = localStorage.getItem(UTF8ToString($0));
        return v ? lengthBytesUTF8(v) + 1 : 0;
    }, key);
    if (len == 0) return NULL;
    char* buf = malloc(len);
    EM_ASM({ stringToUTF8(localStorage.getItem(UTF8ToString($0)), $1, $2); }, key, buf, len);
    return buf;
}
```

2. Preferences structure:
```c
typedef struct {
    char style_guide[256];
    char negative_prompts[256];
    int animation_speed;  /* 0-100 */
    int show_narrative;
    int high_contrast;
    int large_text;
} Preferences;
```

3. Preferences UI rendered in canvas (not DOM modal):
```c
void preferences_ui_render(Preferences* prefs, int x, int y, int w, int h);
void preferences_ui_handle_click(int x, int y);
```

**Acceptance criteria:**
- [ ] Preferences persist across sessions
- [ ] Settings UI renders in canvas
- [ ] Import/export functionality
- [ ] No DOM elements for preferences

---

### Sub-Issue 3-011i: AI Integration Modules

**Files to create:**
- `src/client/wasm/art-tracker.c` - Track cards needing regeneration
- `src/client/wasm/gen-queue.c` - Image generation queue
- `src/client/wasm/img-cache.c` - Image caching (IDBFS)

**JS files replaced:** `art-tracker.js`, `generation-queue.js`, `image-cache.js`,
`style-merger.js`, `upgrade-viz.js`, `battle-canvas.js`, `region-selector.js`,
`scene-composition.js`, `style-transfer.js`

**Key implementation details:**

1. IDBFS for persistent image cache:
```c
void cache_init(void) {
    EM_ASM(
        FS.mkdir('/cache');
        FS.mount(IDBFS, {}, '/cache');
        FS.syncfs(true, function(err) { /* ... */ });
    );
}
```

2. Generation queue in C:
```c
typedef struct {
    int card_instance_id;
    int priority;
    char prompt[512];
    int retry_count;
} GenerationTask;
```

3. Style transfer data from existing `src/visual/02-card-prompts.c`

**Alternative approach:** Move all AI image generation to server-side,
with browser just displaying received images. This simplifies the WASM client.

**Acceptance criteria:**
- [ ] Cards marked for regeneration tracked
- [ ] Generation requests queued with priorities
- [ ] Images cached persistently
- [ ] Battle canvas renders with region support

---

### Sub-Issue 3-011j: Final Integration and Cleanup

**Tasks:**
1. Remove all external JS files from `assets/web/`
2. Update `index.html` to minimal shell (~30 lines)
3. Update `Makefile.wasm` with all new source files
4. Update build script to verify no external JS
5. Test complete client functionality

**Minimal index.html:**
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Symbeline Realms</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        html, body { width: 100%; height: 100%; overflow: hidden; }
        #canvas { width: 100%; height: 100%; display: block; }
    </style>
</head>
<body>
    <canvas id="canvas"></canvas>
    <script src="symbeline.js"></script>
    <script>SymbelineRealms().then(M => M._client_init());</script>
</body>
</html>
```

**Acceptance criteria:**
- [ ] No external JS files in assets/web/ except Emscripten loader
- [ ] No external CSS files
- [ ] All functionality works as before
- [ ] Build size under 200 KB gzipped
- [ ] Phase 3 demo updated to use pure WASM client

---

## Dependencies

- 3-003: WASM build configuration (complete)
- 3-004: Browser canvas renderer (provides current JS implementation to port)
- 3-005: Browser input handler (provides current JS implementation to port)

## Related Documents

- `docs/wasm-transition-plan.md` - Detailed code examples and migration guide
- `src/client/wasm/js-interop.h` - Current WASM-JS bridge (to be eliminated)
- `Makefile.wasm` - Current WASM build configuration

## Technical Notes

### Why EM_ASM Instead of External JS

The `EM_ASM` macro embeds JavaScript inline in the C source, which Emscripten
compiles into the WASM binary. This eliminates external JS files while still
allowing browser API access where no pure WASM alternative exists.

### WebGL vs 2D Canvas

For this card game, 2D Canvas is sufficient and simpler. WebGL would provide:
- Hardware acceleration for effects
- Shader-based rendering
- Better performance for particles

But adds complexity. Can be added later if performance issues arise.

### Image Loading

Images from ComfyUI will be loaded via:
```c
EM_ASM({
    var img = new Image();
    img.onload = function() {
        Module.canvas.getContext('2d').drawImage(img, $0, $1, $2, $3);
    };
    img.src = UTF8ToString($4);
}, x, y, w, h, url);
```

### Memory Management

- Use Emscripten's `ALLOW_MEMORY_GROWTH=1` for dynamic allocation
- Prefer stack allocation for temporary buffers
- Pool allocators for frequently created/destroyed objects (animations)

### Testing Strategy

1. Create C test harness that exercises each module
2. Build WASM with new module
3. Visual comparison with JS version in browser
4. Remove JS file from index.html
5. Verify no regressions

## Statistics

**Current JS/CSS:**
- 22 JavaScript files: ~7,800 lines
- 1 CSS file: 594 lines
- Total: ~8,400 lines of browser code

**Estimated C replacement:**
- ~4,000 lines of C (more compact, less boilerplate)
- Compiled WASM: ~115 KB
- Emscripten glue: ~25 KB
- Total: ~140 KB gzipped

## Acceptance Criteria

- [ ] All 22 JavaScript files eliminated
- [ ] CSS file eliminated
- [ ] index.html reduced to minimal shell
- [ ] All game functionality preserved
- [ ] Build size under 200 KB gzipped
- [ ] No console errors in browser
- [ ] Works in Chrome, Firefox, Safari
- [ ] Phase 3 demo passes with pure WASM client
