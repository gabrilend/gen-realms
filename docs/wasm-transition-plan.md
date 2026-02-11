# JavaScript/CSS to WebAssembly Transition Plan

This document inventories all JavaScript and CSS dependencies in the Symbeline Realms
browser client and provides a concrete transition path to pure WebAssembly with no
JavaScript or CSS in the final MVP.

## Document Overview

- **Current State:** 22 JavaScript files, 1 CSS file, 1 HTML file
- **Target State:** 1 HTML shell, 1 WASM binary, 0 JavaScript/CSS
- **Strategy:** Incremental migration using Emscripten's canvas API

---

## Current File Inventory

### JavaScript Files (22 total)

| File | Lines | Purpose | Priority |
|------|-------|---------|----------|
| `symbeline.js` | 294 | WASM loader, WebSocket, canvas bridge | P0 - Bootstrap |
| `canvas.js` | ~350 | Layout system, zones, render loop | P1 - Core |
| `card-renderer.js` | ~400 | Card drawing with factions, effects | P1 - Core |
| `zone-renderer.js` | ~500 | Trade row, hand, bases rendering | P1 - Core |
| `panel-renderer.js` | ~300 | Status bar, narrative panel | P1 - Core |
| `input-handler.js` | ~450 | Mouse/keyboard, actions, buttons | P1 - Core |
| `animation.js` | ~350 | Animation queue, easing functions | P2 - Polish |
| `card-animations.js` | ~300 | Play/buy/draw card animations | P2 - Polish |
| `effects.js` | ~250 | Attack/damage visual effects | P2 - Polish |
| `narrative.js` | ~300 | LLM narrative display, scrolling | P2 - Polish |
| `preferences.js` | ~200 | localStorage management | P3 - Features |
| `preferences-ui.js` | ~400 | Preferences modal DOM | P3 - Features |
| `draw-order.js` | ~350 | Draw order selection UI | P3 - Features |
| `art-tracker.js` | ~200 | Track cards needing regeneration | P4 - AI |
| `style-merger.js` | ~350 | Merge user prefs with card prompts | P4 - AI |
| `generation-queue.js` | ~300 | ComfyUI request queue | P4 - AI |
| `image-cache.js` | ~400 | Memory + IndexedDB cache | P4 - AI |
| `upgrade-viz.js` | ~350 | Particle effects for upgrades | P4 - AI |
| `battle-canvas.js` | ~500 | Battle scene canvas, regions | P4 - AI |
| `region-selector.js` | ~420 | Event-to-region mapping | P4 - AI |
| `scene-composition.js` | ~380 | Z-layer, element placement | P4 - AI |
| `style-transfer.js` | ~450 | Faction styles, color palettes | P4 - AI |

**Total:** ~7,800 lines of JavaScript

### CSS File (1 total)

| File | Lines | Purpose |
|------|-------|---------|
| `style.css` | 594 | Dark theme, layout, modals, responsive |

### HTML File (1 total)

| File | Purpose |
|------|---------|
| `index.html` | Shell with canvas, loading overlay, script tags |

---

## Existing C Infrastructure

### Current WASM Bridge (`src/client/wasm/js-interop.h`)

```c
/* Functions C calls into JS (will be eliminated) */
extern void js_log_info(const char* message);
extern void js_log_error(const char* message);
extern void js_send_message(const char* json);
extern int js_is_connected(void);
extern void js_clear_canvas(void);
extern void js_draw_rect(int x, int y, int w, int h, const char* color);
extern void js_draw_text(int x, int y, const char* text);
extern void js_draw_card(int x, int y, int w, int h, const char* card_json);
extern void js_request_render(void);
extern void js_storage_set(const char* key, const char* value);
extern const char* js_storage_get(const char* key);
```

### Current Makefile.wasm Exports

```makefile
EXPORTED_FUNCTIONS="['_client_init','_client_cleanup','_client_handle_message',
                    '_client_get_action','_client_render_frame','_malloc','_free']"
EXPORTED_RUNTIME_METHODS="['ccall','cwrap','stringToUTF8','UTF8ToString']"
```

---

## Transition Strategy

### Phase 1: Replace JS Rendering with C Canvas API

The first phase eliminates all JavaScript rendering code by using Emscripten's
direct canvas access via `emscripten.h` primitives.

#### 1.1 Canvas Context Access

**Current JS (canvas.js):**
```javascript
function init(canvasId) {
    canvas = document.getElementById(canvasId);
    ctx = canvas.getContext('2d');
    /* ... */
}
```

**Target C (src/client/wasm/canvas.c):**
```c
/* {{{ canvas_init */
#include <emscripten.h>
#include <emscripten/html5.h>

static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl_ctx;
static int canvas_width, canvas_height;

int canvas_init(const char* canvas_id) {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = 0;
    attrs.depth = 0;
    attrs.stencil = 0;
    attrs.antialias = 1;
    attrs.majorVersion = 2;  /* WebGL 2 */

    gl_ctx = emscripten_webgl_create_context(canvas_id, &attrs);
    if (gl_ctx <= 0) {
        return -1;  /* Fallback to 2D below */
    }
    emscripten_webgl_make_context_current(gl_ctx);

    /* Get canvas size */
    emscripten_get_canvas_element_size(canvas_id, &canvas_width, &canvas_height);
    return 0;
}
/* }}} */
```

#### 1.2 2D Drawing Primitives

For simpler 2D rendering, use Emscripten's inline JavaScript elimination with
`EM_ASM` macros that compile to direct WebAssembly:

**Target C (src/client/wasm/draw2d.c):**
```c
/* {{{ draw_rect */
#include <emscripten.h>

/* These EM_ASM calls will be inlined and don't require external JS files */
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    EM_ASM({
        var ctx = Module.canvas.getContext('2d');
        ctx.fillStyle = 'rgb(' + (($3 >> 16) & 0xFF) + ',' +
                        (($3 >> 8) & 0xFF) + ',' + ($3 & 0xFF) + ')';
        ctx.fillRect($0, $1, $2, $3);
    }, x, y, w, h, color);
}
/* }}} */

/* {{{ draw_text */
void draw_text(int x, int y, const char* text, uint32_t color) {
    EM_ASM({
        var ctx = Module.canvas.getContext('2d');
        ctx.fillStyle = 'rgb(' + (($3 >> 16) & 0xFF) + ',' +
                        (($3 >> 8) & 0xFF) + ',' + ($3 & 0xFF) + ')';
        ctx.font = '14px monospace';
        ctx.fillText(UTF8ToString($2), $0, $1);
    }, x, y, text, color);
}
/* }}} */

/* {{{ draw_line */
void draw_line(int x1, int y1, int x2, int y2, uint32_t color, int width) {
    EM_ASM({
        var ctx = Module.canvas.getContext('2d');
        ctx.strokeStyle = 'rgb(' + (($4 >> 16) & 0xFF) + ',' +
                          (($4 >> 8) & 0xFF) + ',' + ($4 & 0xFF) + ')';
        ctx.lineWidth = $5;
        ctx.beginPath();
        ctx.moveTo($0, $1);
        ctx.lineTo($2, $3);
        ctx.stroke();
    }, x1, y1, x2, y2, color, width);
}
/* }}} */

/* {{{ draw_rounded_rect */
void draw_rounded_rect(int x, int y, int w, int h, int r, uint32_t color) {
    EM_ASM({
        var ctx = Module.canvas.getContext('2d');
        ctx.fillStyle = 'rgb(' + (($5 >> 16) & 0xFF) + ',' +
                        (($5 >> 8) & 0xFF) + ',' + ($5 & 0xFF) + ')';
        ctx.beginPath();
        ctx.roundRect($0, $1, $2, $3, $4);
        ctx.fill();
    }, x, y, w, h, r, color);
}
/* }}} */
```

### Phase 2: Replace CSS with C-Defined Styles

All CSS styling becomes C constants compiled into the WASM binary.

#### 2.1 Color Definitions

**Current CSS (style.css):**
```css
.faction-merchant { color: #d4a017; }
.faction-wilds { color: #2d7a2d; }
.faction-kingdom { color: #3366cc; }
.faction-artificer { color: #cc3333; }
```

**Target C (src/client/wasm/theme.h):**
```c
/* {{{ theme colors */
#ifndef THEME_H
#define THEME_H

#include <stdint.h>

/* RGB color packed as 0x00RRGGBB */
typedef uint32_t Color;

/* {{{ color macros */
#define RGB(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))
/* }}} */

/* {{{ background colors */
static const Color COLOR_BG_DARK      = RGB(0x0d, 0x0d, 0x1a);  /* #0d0d1a */
static const Color COLOR_BG_PANEL     = RGB(0x1a, 0x1a, 0x2e);  /* #1a1a2e */
static const Color COLOR_BG_CARD      = RGB(0x22, 0x22, 0x22);  /* #222222 */
/* }}} */

/* {{{ faction colors */
static const Color COLOR_FACTION_MERCHANT  = RGB(0xd4, 0xa0, 0x17);  /* #d4a017 */
static const Color COLOR_FACTION_WILDS     = RGB(0x2d, 0x7a, 0x2d);  /* #2d7a2d */
static const Color COLOR_FACTION_KINGDOM   = RGB(0x33, 0x66, 0xcc);  /* #3366cc */
static const Color COLOR_FACTION_ARTIFICER = RGB(0xcc, 0x33, 0x33);  /* #cc3333 */
static const Color COLOR_FACTION_NEUTRAL   = RGB(0x88, 0x88, 0x88);  /* #888888 */
/* }}} */

/* {{{ value colors */
static const Color COLOR_AUTHORITY = RGB(0x44, 0xcc, 0xcc);  /* #44cccc */
static const Color COLOR_COMBAT    = RGB(0xcc, 0x66, 0xcc);  /* #cc66cc */
static const Color COLOR_TRADE     = RGB(0xd4, 0xa0, 0x17);  /* #d4a017 */
/* }}} */

/* {{{ text colors */
static const Color COLOR_TEXT_PRIMARY   = RGB(0xe0, 0xe0, 0xe0);  /* #e0e0e0 */
static const Color COLOR_TEXT_SECONDARY = RGB(0xaa, 0xaa, 0xaa);  /* #aaaaaa */
static const Color COLOR_TEXT_ACCENT    = RGB(0x88, 0x66, 0xcc);  /* #8866cc */
/* }}} */

/* {{{ status colors */
static const Color COLOR_STATUS_CONNECTED    = RGB(0x2d, 0x7a, 0x2d);
static const Color COLOR_STATUS_CONNECTING   = RGB(0xd4, 0xa0, 0x17);
static const Color COLOR_STATUS_DISCONNECTED = RGB(0xcc, 0x33, 0x33);
/* }}} */

#endif /* THEME_H */
/* }}} */
```

#### 2.2 Layout Constants

**Current CSS:**
```css
#game-header { padding: 8px 16px; }
#connect-dialog { min-width: 320px; padding: 24px 32px; }
```

**Target C (src/client/wasm/layout.h):**
```c
/* {{{ layout constants */
#ifndef LAYOUT_H
#define LAYOUT_H

/* {{{ spacing */
#define SPACING_XS   4
#define SPACING_SM   8
#define SPACING_MD   16
#define SPACING_LG   24
#define SPACING_XL   32
/* }}} */

/* {{{ header layout */
typedef struct {
    int height;
    int padding_x;
    int padding_y;
} HeaderLayout;

static const HeaderLayout HEADER_LAYOUT = {
    .height = 40,
    .padding_x = SPACING_MD,
    .padding_y = SPACING_SM
};
/* }}} */

/* {{{ dialog layout */
typedef struct {
    int min_width;
    int padding_x;
    int padding_y;
    int border_radius;
} DialogLayout;

static const DialogLayout CONNECT_DIALOG = {
    .min_width = 320,
    .padding_x = SPACING_XL,
    .padding_y = SPACING_LG,
    .border_radius = 0
};
/* }}} */

/* {{{ zone layout (percentages * 100) */
typedef struct {
    int x, y, w, h;  /* Percentages 0-100 */
} ZonePercent;

static const ZonePercent ZONE_STATUS     = { 0, 0, 100, 5 };
static const ZonePercent ZONE_TRADE_ROW  = { 0, 5, 100, 18 };
static const ZonePercent ZONE_OPP_BASES  = { 0, 23, 25, 12 };
static const ZonePercent ZONE_NARRATIVE  = { 75, 23, 25, 37 };
static const ZonePercent ZONE_PLAY_AREA  = { 0, 35, 75, 25 };
static const ZonePercent ZONE_PLR_BASES  = { 0, 60, 25, 12 };
static const ZonePercent ZONE_HAND       = { 0, 72, 100, 28 };
/* }}} */

#endif /* LAYOUT_H */
/* }}} */
```

### Phase 3: Replace JS Modules with C Implementations

#### 3.1 Card Renderer

**Current JS (card-renderer.js):**
```javascript
function renderCard(ctx, card, x, y, w, h, state) {
    const color = FACTION_COLORS[card.faction] || FACTION_COLORS.neutral;
    ctx.fillStyle = '#222';
    ctx.fillRect(x, y, w, h);
    ctx.strokeStyle = color;
    ctx.strokeRect(x, y, w, h);
    /* ... */
}
```

**Target C (src/client/wasm/card-renderer.c):**
```c
/* {{{ card_render */
#include "draw2d.h"
#include "theme.h"
#include "card.h"

/* {{{ get_faction_color */
static Color get_faction_color(Faction faction) {
    switch (faction) {
        case FACTION_MERCHANT:  return COLOR_FACTION_MERCHANT;
        case FACTION_WILDS:     return COLOR_FACTION_WILDS;
        case FACTION_KINGDOM:   return COLOR_FACTION_KINGDOM;
        case FACTION_ARTIFICER: return COLOR_FACTION_ARTIFICER;
        default:                return COLOR_FACTION_NEUTRAL;
    }
}
/* }}} */

/* {{{ card_render */
void card_render(const Card* card, int x, int y, int w, int h, CardState state) {
    Color faction_color = get_faction_color(card->faction);

    /* Card background */
    draw_rect(x, y, w, h, COLOR_BG_CARD);

    /* Faction border */
    draw_rect_outline(x, y, w, h, faction_color, 2);

    /* Card name */
    draw_text(x + 4, y + 16, card->name, faction_color);

    /* Cost badge */
    if (card->cost > 0) {
        char cost_str[4];
        snprintf(cost_str, sizeof(cost_str), "%d", card->cost);
        draw_text(x + w - 16, y + 16, cost_str, COLOR_TRADE);
    }

    /* Hover/selected state */
    if (state == CARD_STATE_HOVER) {
        draw_rect_outline(x - 2, y - 2, w + 4, h + 4, COLOR_TEXT_ACCENT, 2);
    } else if (state == CARD_STATE_SELECTED) {
        draw_rect_outline(x - 2, y - 2, w + 4, h + 4, COLOR_TEXT_PRIMARY, 3);
    }

    /* Upgrade badges */
    int badge_x = x + w - 20;
    int badge_y = y + h - 20;

    if (card->attack_bonus > 0) {
        draw_circle(badge_x, badge_y, 8, COLOR_COMBAT);
        badge_x -= 18;
    }
    if (card->trade_bonus > 0) {
        draw_circle(badge_x, badge_y, 8, COLOR_TRADE);
        badge_x -= 18;
    }
    if (card->authority_bonus > 0) {
        draw_circle(badge_x, badge_y, 8, COLOR_AUTHORITY);
    }
}
/* }}} */
/* }}} */
```

#### 3.2 Animation System

**Current JS (animation.js):**
```javascript
var EASING = {
    linear: function(t) { return t; },
    easeOutQuad: function(t) { return t * (2 - t); },
    easeInOutCubic: function(t) { /* ... */ }
};
```

**Target C (src/client/wasm/animation.c):**
```c
/* {{{ animation system */
#include <math.h>
#include <stdlib.h>

/* {{{ easing functions */
typedef float (*EasingFn)(float t);

static float ease_linear(float t) {
    return t;
}

static float ease_out_quad(float t) {
    return t * (2.0f - t);
}

static float ease_in_out_cubic(float t) {
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    }
    float f = (2.0f * t) - 2.0f;
    return 0.5f * f * f * f + 1.0f;
}

static float ease_out_bounce(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    }
    t -= 2.625f / 2.75f;
    return 7.5625f * t * t + 0.984375f;
}
/* }}} */

/* {{{ animation structure */
typedef enum {
    ANIM_MOVE,
    ANIM_FADE,
    ANIM_SCALE,
    ANIM_COLOR
} AnimationType;

typedef struct Animation {
    AnimationType type;
    float start_value[4];   /* x, y, w, h or r, g, b, a */
    float end_value[4];
    float duration_ms;
    float elapsed_ms;
    EasingFn easing;
    void* target;           /* Pointer to animated object */
    void (*on_complete)(struct Animation*);
    struct Animation* next;
} Animation;

typedef struct {
    Animation* head;
    Animation* tail;
    int count;
    int max_concurrent;
} AnimationQueue;
/* }}} */

/* {{{ animation_queue_create */
AnimationQueue* animation_queue_create(int max_concurrent) {
    AnimationQueue* queue = malloc(sizeof(AnimationQueue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    queue->max_concurrent = max_concurrent;
    return queue;
}
/* }}} */

/* {{{ animation_queue_add */
void animation_queue_add(AnimationQueue* queue, Animation* anim) {
    anim->next = NULL;
    if (queue->tail) {
        queue->tail->next = anim;
        queue->tail = anim;
    } else {
        queue->head = queue->tail = anim;
    }
    queue->count++;
}
/* }}} */

/* {{{ animation_queue_update */
void animation_queue_update(AnimationQueue* queue, float dt_ms) {
    Animation* prev = NULL;
    Animation* curr = queue->head;

    while (curr) {
        curr->elapsed_ms += dt_ms;
        float t = curr->elapsed_ms / curr->duration_ms;

        if (t >= 1.0f) {
            /* Animation complete */
            if (curr->on_complete) {
                curr->on_complete(curr);
            }

            /* Remove from queue */
            Animation* to_free = curr;
            if (prev) {
                prev->next = curr->next;
            } else {
                queue->head = curr->next;
            }
            if (curr == queue->tail) {
                queue->tail = prev;
            }
            curr = curr->next;
            free(to_free);
            queue->count--;
        } else {
            /* Apply easing and update */
            float eased = curr->easing(t);
            /* Apply interpolation to target based on type */
            /* ... */
            prev = curr;
            curr = curr->next;
        }
    }
}
/* }}} */
/* }}} */
```

#### 3.3 Input Handling

**Current JS (input-handler.js):**
```javascript
canvas.addEventListener('click', function(e) {
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;
    /* ... */
});
```

**Target C (src/client/wasm/input.c):**
```c
/* {{{ input handling */
#include <emscripten/html5.h>

/* {{{ input state */
typedef struct {
    int mouse_x, mouse_y;
    int mouse_down;
    int mouse_clicked;
    char keys_pressed[256];
    char keys_down[256];
} InputState;

static InputState input_state = {0};
/* }}} */

/* {{{ mouse callback */
static EM_BOOL mouse_callback(int type, const EmscriptenMouseEvent* e, void* data) {
    (void)data;

    input_state.mouse_x = e->targetX;
    input_state.mouse_y = e->targetY;

    if (type == EMSCRIPTEN_EVENT_MOUSEDOWN) {
        input_state.mouse_down = 1;
    } else if (type == EMSCRIPTEN_EVENT_MOUSEUP) {
        if (input_state.mouse_down) {
            input_state.mouse_clicked = 1;
        }
        input_state.mouse_down = 0;
    }

    return EM_TRUE;
}
/* }}} */

/* {{{ key callback */
static EM_BOOL key_callback(int type, const EmscriptenKeyboardEvent* e, void* data) {
    (void)data;

    int code = e->keyCode;
    if (code < 256) {
        if (type == EMSCRIPTEN_EVENT_KEYDOWN) {
            if (!input_state.keys_down[code]) {
                input_state.keys_pressed[code] = 1;
            }
            input_state.keys_down[code] = 1;
        } else if (type == EMSCRIPTEN_EVENT_KEYUP) {
            input_state.keys_down[code] = 0;
        }
    }

    return EM_TRUE;
}
/* }}} */

/* {{{ input_init */
void input_init(const char* canvas_id) {
    emscripten_set_mousedown_callback(canvas_id, NULL, EM_TRUE, mouse_callback);
    emscripten_set_mouseup_callback(canvas_id, NULL, EM_TRUE, mouse_callback);
    emscripten_set_mousemove_callback(canvas_id, NULL, EM_TRUE, mouse_callback);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, EM_TRUE, key_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, EM_TRUE, key_callback);
}
/* }}} */

/* {{{ input_update (call at end of frame) */
void input_update(void) {
    input_state.mouse_clicked = 0;
    memset(input_state.keys_pressed, 0, sizeof(input_state.keys_pressed));
}
/* }}} */

/* {{{ input getters */
int input_mouse_x(void) { return input_state.mouse_x; }
int input_mouse_y(void) { return input_state.mouse_y; }
int input_mouse_clicked(void) { return input_state.mouse_clicked; }
int input_key_pressed(int key) { return input_state.keys_pressed[key]; }
int input_key_down(int key) { return input_state.keys_down[key]; }
/* }}} */
/* }}} */
```

#### 3.4 WebSocket (Keep Minimal JS Bridge)

WebSocket API is not directly available in pure WASM. However, we can use
Emscripten's WebSocket library which compiles to minimal inline JS:

**Target C (src/client/wasm/websocket.c):**
```c
/* {{{ websocket */
#include <emscripten/websocket.h>

static EMSCRIPTEN_WEBSOCKET_T ws = 0;
static int ws_connected = 0;

/* {{{ ws_onopen */
static EM_BOOL ws_onopen(int type, const EmscriptenWebSocketOpenEvent* e, void* data) {
    (void)type; (void)e; (void)data;
    ws_connected = 1;
    return EM_TRUE;
}
/* }}} */

/* {{{ ws_onclose */
static EM_BOOL ws_onclose(int type, const EmscriptenWebSocketCloseEvent* e, void* data) {
    (void)type; (void)e; (void)data;
    ws_connected = 0;
    return EM_TRUE;
}
/* }}} */

/* {{{ ws_onmessage */
static EM_BOOL ws_onmessage(int type, const EmscriptenWebSocketMessageEvent* e, void* data) {
    (void)type; (void)data;
    if (e->isText) {
        /* Handle text message */
        client_handle_message((const char*)e->data);
    }
    return EM_TRUE;
}
/* }}} */

/* {{{ websocket_connect */
int websocket_connect(const char* url) {
    EmscriptenWebSocketCreateAttributes attrs = {
        .url = url,
        .protocols = NULL,
        .createOnMainThread = EM_TRUE
    };

    ws = emscripten_websocket_new(&attrs);
    if (ws <= 0) return -1;

    emscripten_websocket_set_onopen_callback(ws, NULL, ws_onopen);
    emscripten_websocket_set_onclose_callback(ws, NULL, ws_onclose);
    emscripten_websocket_set_onmessage_callback(ws, NULL, ws_onmessage);

    return 0;
}
/* }}} */

/* {{{ websocket_send */
int websocket_send(const char* message) {
    if (!ws_connected) return -1;
    return emscripten_websocket_send_utf8_text(ws, message);
}
/* }}} */

/* {{{ websocket_is_connected */
int websocket_is_connected(void) {
    return ws_connected;
}
/* }}} */
/* }}} */
```

#### 3.5 LocalStorage

**Target C (src/client/wasm/storage.c):**
```c
/* {{{ localStorage access */
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

/* {{{ storage_set */
void storage_set(const char* key, const char* value) {
    EM_ASM({
        localStorage.setItem(UTF8ToString($0), UTF8ToString($1));
    }, key, value);
}
/* }}} */

/* {{{ storage_get */
/* Returns malloc'd string, caller must free */
char* storage_get(const char* key) {
    int len = EM_ASM_INT({
        var val = localStorage.getItem(UTF8ToString($0));
        if (val === null) return 0;
        return lengthBytesUTF8(val) + 1;
    }, key);

    if (len == 0) return NULL;

    char* buf = malloc(len);
    EM_ASM({
        var val = localStorage.getItem(UTF8ToString($0));
        stringToUTF8(val, $1, $2);
    }, key, buf, len);

    return buf;
}
/* }}} */

/* {{{ storage_remove */
void storage_remove(const char* key) {
    EM_ASM({
        localStorage.removeItem(UTF8ToString($0));
    }, key);
}
/* }}} */
/* }}} */
```

### Phase 4: Eliminate External JavaScript Files

#### 4.1 Minimal HTML Shell

**Target index.html:**
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Symbeline Realms</title>
    <style>
        /* Minimal inline styles - only what's needed for canvas sizing */
        * { margin: 0; padding: 0; box-sizing: border-box; }
        html, body { width: 100%; height: 100%; overflow: hidden; }
        #game-canvas { width: 100%; height: 100%; display: block; }
    </style>
</head>
<body>
    <canvas id="game-canvas"></canvas>
    <script src="symbeline.js"></script>
    <script>
        SymbelineRealms().then(function(Module) {
            Module._client_init();
        });
    </script>
</body>
</html>
```

Note: The `symbeline.js` here is Emscripten's generated loader, not hand-written JS.
It's required by Emscripten but contains only WASM bootstrap code.

#### 4.2 Updated Makefile.wasm

```makefile
# {{{ Makefile.wasm - Pure WASM build
CC = emcc
CFLAGS = -O3 -Wall -Wextra

EMCC_FLAGS = \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS="['_client_init','_client_cleanup','_malloc','_free']" \
    -s EXPORTED_RUNTIME_METHODS="['UTF8ToString','stringToUTF8','lengthBytesUTF8']" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="SymbelineRealms" \
    -s NO_EXIT_RUNTIME=1 \
    -s FILESYSTEM=0 \
    -s ENVIRONMENT=web \
    -s USE_WEBGL2=1 \
    -lwebsocket.js

# All rendering, input, animation in C
WASM_SOURCES = \
    src/client/wasm/main.c \
    src/client/wasm/canvas.c \
    src/client/wasm/draw2d.c \
    src/client/wasm/input.c \
    src/client/wasm/animation.c \
    src/client/wasm/websocket.c \
    src/client/wasm/storage.c \
    src/client/wasm/card-renderer.c \
    src/client/wasm/zone-renderer.c \
    src/client/wasm/panel-renderer.c \
    src/client/wasm/ui-dialogs.c

CORE_SOURCES = \
    src/core/01-card.c \
    src/core/02-deck.c \
    src/core/03-player.c \
    src/core/04-trade-row.c \
    src/core/05-game.c \
    src/core/06-combat.c \
    src/core/07-effects.c

ALL_SOURCES = $(WASM_SOURCES) $(CORE_SOURCES)

all: assets/web/symbeline.js

assets/web/symbeline.js: $(ALL_SOURCES)
    $(CC) $(CFLAGS) $(EMCC_FLAGS) -o $@ $(ALL_SOURCES)
# }}}
```

---

## Module-by-Module Transition Checklist

### P0 - Bootstrap (Must Have for Any WASM)

| JS Module | C Replacement | Status |
|-----------|---------------|--------|
| `symbeline.js` | Emscripten loader (auto-generated) | Planned |

### P1 - Core Rendering (Minimum Playable)

| JS Module | C Replacement | Status |
|-----------|---------------|--------|
| `canvas.js` | `src/client/wasm/canvas.c` | Planned |
| `card-renderer.js` | `src/client/wasm/card-renderer.c` | Planned |
| `zone-renderer.js` | `src/client/wasm/zone-renderer.c` | Planned |
| `panel-renderer.js` | `src/client/wasm/panel-renderer.c` | Planned |
| `input-handler.js` | `src/client/wasm/input.c` | Planned |

### P2 - Polish (Better UX)

| JS Module | C Replacement | Status |
|-----------|---------------|--------|
| `animation.js` | `src/client/wasm/animation.c` | Planned |
| `card-animations.js` | `src/client/wasm/card-anim.c` | Planned |
| `effects.js` | `src/client/wasm/effects.c` | Planned |
| `narrative.js` | `src/client/wasm/narrative.c` | Planned |

### P3 - Features (Full Feature Parity)

| JS Module | C Replacement | Status |
|-----------|---------------|--------|
| `preferences.js` | `src/client/wasm/storage.c` | Planned |
| `preferences-ui.js` | `src/client/wasm/ui-prefs.c` | Planned |
| `draw-order.js` | `src/client/wasm/ui-draw-order.c` | Planned |

### P4 - AI Integration (ComfyUI)

| JS Module | C Replacement | Status |
|-----------|---------------|--------|
| `art-tracker.js` | `src/client/wasm/art-tracker.c` | Planned |
| `style-merger.js` | Inline in card-renderer | Planned |
| `generation-queue.js` | `src/client/wasm/gen-queue.c` | Planned |
| `image-cache.js` | `src/client/wasm/img-cache.c` | Planned |
| `upgrade-viz.js` | `src/client/wasm/upgrade-viz.c` | Planned |
| `battle-canvas.js` | `src/client/wasm/battle-canvas.c` | Planned |
| `region-selector.js` | `src/client/wasm/region-sel.c` | Planned |
| `scene-composition.js` | `src/client/wasm/scene-comp.c` | Planned |
| `style-transfer.js` | `src/visual/02-card-prompts.c` (exists) | Partial |

### CSS Elimination

| CSS Section | C Replacement | Status |
|-------------|---------------|--------|
| Colors | `src/client/wasm/theme.h` | Planned |
| Layout | `src/client/wasm/layout.h` | Planned |
| Fonts | Inline in draw calls | Planned |
| Animations | `src/client/wasm/animation.c` | Planned |
| Responsive | Canvas resize handler | Planned |

---

## IndexedDB Alternative

For `image-cache.js` which uses IndexedDB, we have two options:

### Option A: Emscripten IDBFS (Recommended)

```c
/* {{{ IDBFS for persistent image cache */
#include <emscripten.h>

void cache_init(void) {
    EM_ASM(
        FS.mkdir('/cache');
        FS.mount(IDBFS, {}, '/cache');
        FS.syncfs(true, function(err) {
            if (err) console.error('Cache sync error:', err);
        });
    );
}

void cache_save(void) {
    EM_ASM(
        FS.syncfs(false, function(err) {
            if (err) console.error('Cache save error:', err);
        });
    );
}
/* }}} */
```

### Option B: Server-Side Caching

Move image caching to the game server, with WASM client requesting images via
WebSocket. This eliminates browser storage complexity.

---

## Build Size Estimates

| Component | Estimated Size |
|-----------|----------------|
| Core game logic | ~50 KB |
| Rendering code | ~30 KB |
| Animation system | ~10 KB |
| Input handling | ~5 KB |
| UI dialogs | ~15 KB |
| Theme/layout data | ~5 KB |
| **Total WASM** | **~115 KB** |
| Emscripten glue JS | ~25 KB |
| **Total Download** | **~140 KB gzipped** |

Current JS/CSS total: ~80 KB + WASM

Post-transition: ~140 KB (single binary, no external dependencies)

---

## Migration Timeline

This document does not include time estimates. The work should be prioritized as:

1. **P1 Core Rendering** - Must complete before any other browser work
2. **P0 Bootstrap** - Automatic with P1 completion
3. **P2 Polish** - Can be done incrementally
4. **P3 Features** - Can be done incrementally
5. **P4 AI Integration** - Last priority, requires ComfyUI backend

Each module can be migrated independently, with the JS fallback remaining until
the C version is tested and stable.

---

## Testing Strategy

For each migrated module:

1. Create C test harness that exercises the module
2. Build WASM with new module
3. Visual comparison with JS version
4. Remove JS file from index.html
5. Commit migration

---

## References

- [Emscripten HTML5 API](https://emscripten.org/docs/api_reference/html5.h.html)
- [Emscripten WebSocket API](https://emscripten.org/docs/api_reference/emscripten.h.html#websocket)
- [Emscripten IDBFS](https://emscripten.org/docs/api_reference/Filesystem-API.html#idbfs)
- [WebGL via Emscripten](https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html)
