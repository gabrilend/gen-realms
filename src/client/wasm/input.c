/*
 * input.c - Input Handling Implementation for WASM Client
 *
 * Uses Emscripten HTML5 callbacks for mouse and keyboard events.
 */

#include "input.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <string.h>

/* {{{ Global state */
static MouseState g_mouse = {0};
static KeyboardState g_keyboard = {0};
static bool g_initialized = false;
/* }}} */

/* {{{ mouse_callback
 * Handles all mouse events.
 */
static EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent* e, void* user_data) {
    (void)user_data;

    /* Update position */
    g_mouse.x = e->targetX;
    g_mouse.y = e->targetY;

    switch (event_type) {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
            if (e->button < 3) {
                g_mouse.buttons[e->button] = true;
                g_mouse.clicked[e->button] = true;
            }
            break;

        case EMSCRIPTEN_EVENT_MOUSEUP:
            if (e->button < 3) {
                g_mouse.buttons[e->button] = false;
                g_mouse.released[e->button] = true;
            }
            break;

        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            /* Position already updated above */
            break;

        default:
            break;
    }

    return EM_TRUE;
}
/* }}} */

/* {{{ wheel_callback
 * Handles mouse wheel events.
 */
static EM_BOOL wheel_callback(int event_type, const EmscriptenWheelEvent* e, void* user_data) {
    (void)event_type;
    (void)user_data;

    /* Normalize wheel delta */
    if (e->deltaY > 0) {
        g_mouse.wheel_delta = 1;
    } else if (e->deltaY < 0) {
        g_mouse.wheel_delta = -1;
    }

    return EM_TRUE;
}
/* }}} */

/* {{{ key_callback
 * Handles keyboard events.
 */
static EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent* e, void* user_data) {
    (void)user_data;

    int key = e->keyCode;
    if (key < 0 || key >= 256) return EM_TRUE;

    /* Update modifier state */
    g_keyboard.shift = e->shiftKey;
    g_keyboard.ctrl = e->ctrlKey;
    g_keyboard.alt = e->altKey;

    switch (event_type) {
        case EMSCRIPTEN_EVENT_KEYDOWN:
            if (!g_keyboard.keys_down[key]) {
                g_keyboard.keys_pressed[key] = true;
            }
            g_keyboard.keys_down[key] = true;
            break;

        case EMSCRIPTEN_EVENT_KEYUP:
            g_keyboard.keys_down[key] = false;
            g_keyboard.keys_released[key] = true;
            break;

        default:
            break;
    }

    /* Prevent default for game keys to avoid browser actions */
    if (key == KEY_SPACE || key == KEY_TAB ||
        (key >= KEY_LEFT && key <= KEY_DOWN)) {
        return EM_FALSE; /* Returning false prevents default */
    }

    return EM_TRUE;
}
/* }}} */

/* {{{ input_init */
bool input_init(const char* canvas_id) {
    if (g_initialized) {
        input_cleanup();
    }

    memset(&g_mouse, 0, sizeof(g_mouse));
    memset(&g_keyboard, 0, sizeof(g_keyboard));

    /* Register mouse callbacks on canvas */
    emscripten_set_mousedown_callback(canvas_id, NULL, EM_TRUE, mouse_callback);
    emscripten_set_mouseup_callback(canvas_id, NULL, EM_TRUE, mouse_callback);
    emscripten_set_mousemove_callback(canvas_id, NULL, EM_TRUE, mouse_callback);
    emscripten_set_wheel_callback(canvas_id, NULL, EM_TRUE, wheel_callback);

    /* Register keyboard callbacks on document */
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, EM_TRUE, key_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, EM_TRUE, key_callback);

    g_initialized = true;
    return true;
}
/* }}} */

/* {{{ input_cleanup */
void input_cleanup(void) {
    if (!g_initialized) return;

    /* Unregister callbacks */
    emscripten_set_mousedown_callback(NULL, NULL, EM_FALSE, NULL);
    emscripten_set_mouseup_callback(NULL, NULL, EM_FALSE, NULL);
    emscripten_set_mousemove_callback(NULL, NULL, EM_FALSE, NULL);
    emscripten_set_wheel_callback(NULL, NULL, EM_FALSE, NULL);
    emscripten_set_keydown_callback(NULL, NULL, EM_FALSE, NULL);
    emscripten_set_keyup_callback(NULL, NULL, EM_FALSE, NULL);

    g_initialized = false;
}
/* }}} */

/* {{{ input_update */
void input_update(void) {
    /* Clear single-frame events */
    memset(g_mouse.clicked, 0, sizeof(g_mouse.clicked));
    memset(g_mouse.released, 0, sizeof(g_mouse.released));
    g_mouse.wheel_delta = 0;

    memset(g_keyboard.keys_pressed, 0, sizeof(g_keyboard.keys_pressed));
    memset(g_keyboard.keys_released, 0, sizeof(g_keyboard.keys_released));
}
/* }}} */

/* {{{ input_get_mouse */
const MouseState* input_get_mouse(void) {
    return &g_mouse;
}
/* }}} */

/* {{{ input_get_keyboard */
const KeyboardState* input_get_keyboard(void) {
    return &g_keyboard;
}
/* }}} */

/* {{{ input_is_key_down */
bool input_is_key_down(int key_code) {
    if (key_code < 0 || key_code >= 256) return false;
    return g_keyboard.keys_down[key_code];
}
/* }}} */

/* {{{ input_is_key_pressed */
bool input_is_key_pressed(int key_code) {
    if (key_code < 0 || key_code >= 256) return false;
    return g_keyboard.keys_pressed[key_code];
}
/* }}} */

/* {{{ input_is_mouse_down */
bool input_is_mouse_down(int button) {
    if (button < 0 || button >= 3) return false;
    return g_mouse.buttons[button];
}
/* }}} */

/* {{{ input_is_mouse_clicked */
bool input_is_mouse_clicked(int button) {
    if (button < 0 || button >= 3) return false;
    return g_mouse.clicked[button];
}
/* }}} */

/* {{{ input_hit_test_zone */
bool input_hit_test_zone(int x, int y, const Zone* zone) {
    if (!zone) return false;
    return x >= zone->x && x < zone->x + zone->w &&
           y >= zone->y && y < zone->y + zone->h;
}
/* }}} */

/* {{{ input_get_card_at */
int input_get_card_at(int x, int zone_w, int card_w, int card_count) {
    if (card_count <= 0) return -1;

    /* Calculate card spacing */
    int total_width = card_count * card_w + (card_count - 1) * CARD_SPACING;

    /* Center cards in zone */
    int start_x = (zone_w - total_width) / 2;
    if (start_x < PADDING_NORMAL) start_x = PADDING_NORMAL;

    /* Check if x is before first card */
    if (x < start_x) return -1;

    /* Find which card slot */
    int rel_x = x - start_x;
    int slot = rel_x / (card_w + CARD_SPACING);

    /* Check if within card bounds (not in spacing) */
    int card_start = slot * (card_w + CARD_SPACING);
    if (rel_x > card_start + card_w) return -1;

    /* Check bounds */
    if (slot < 0 || slot >= card_count) return -1;

    return slot;
}
/* }}} */

/* {{{ input_hit_test */
HitTarget input_hit_test(const Layout* layout) {
    HitTarget result = { HIT_NONE, -1, 0, 0 };

    if (!layout) return result;

    int mx = g_mouse.x;
    int my = g_mouse.y;

    /* Check each zone in priority order */
    if (input_hit_test_zone(mx, my, &layout->hand)) {
        result.type = HIT_HAND_CARD;
        result.x = mx - layout->hand.x;
        result.y = my - layout->hand.y;
        /* Caller should use input_get_card_at to get card index */
        return result;
    }

    if (input_hit_test_zone(mx, my, &layout->trade_row)) {
        result.type = HIT_TRADE_ROW_CARD;
        result.x = mx - layout->trade_row.x;
        result.y = my - layout->trade_row.y;
        return result;
    }

    if (input_hit_test_zone(mx, my, &layout->player_bases)) {
        result.type = HIT_PLAYER_BASE;
        result.x = mx - layout->player_bases.x;
        result.y = my - layout->player_bases.y;
        return result;
    }

    if (input_hit_test_zone(mx, my, &layout->opp_bases)) {
        result.type = HIT_OPP_BASE;
        result.x = mx - layout->opp_bases.x;
        result.y = my - layout->opp_bases.y;
        return result;
    }

    if (input_hit_test_zone(mx, my, &layout->narrative)) {
        result.type = HIT_NARRATIVE;
        result.x = mx - layout->narrative.x;
        result.y = my - layout->narrative.y;
        return result;
    }

    if (input_hit_test_zone(mx, my, &layout->play_area)) {
        result.type = HIT_PLAY_AREA;
        result.x = mx - layout->play_area.x;
        result.y = my - layout->play_area.y;
        return result;
    }

    if (input_hit_test_zone(mx, my, &layout->status)) {
        result.type = HIT_STATUS;
        result.x = mx - layout->status.x;
        result.y = my - layout->status.y;
        return result;
    }

    return result;
}
/* }}} */
