/*
 * input.h - Input Handling for WASM Client
 *
 * Handles mouse and keyboard input using Emscripten HTML5 callbacks.
 * Provides both event-driven and polling-based input access.
 */

#ifndef WASM_INPUT_H
#define WASM_INPUT_H

#include <stdbool.h>
#include "theme.h"

/* {{{ Mouse buttons */
#define MOUSE_BUTTON_LEFT   0
#define MOUSE_BUTTON_MIDDLE 1
#define MOUSE_BUTTON_RIGHT  2
/* }}} */

/* {{{ Key codes (common ones) */
#define KEY_ESCAPE    27
#define KEY_ENTER     13
#define KEY_SPACE     32
#define KEY_BACKSPACE 8
#define KEY_TAB       9

#define KEY_LEFT      37
#define KEY_UP        38
#define KEY_RIGHT     39
#define KEY_DOWN      40

#define KEY_0         48
#define KEY_1         49
#define KEY_2         50
#define KEY_3         51
#define KEY_4         52
#define KEY_5         53
#define KEY_6         54
#define KEY_7         55
#define KEY_8         56
#define KEY_9         57

#define KEY_A         65
#define KEY_B         66
#define KEY_C         67
#define KEY_D         68
#define KEY_E         69
#define KEY_N         78
#define KEY_P         80
#define KEY_S         83
#define KEY_W         87
/* }}} */

/* {{{ MouseState
 * Current state of the mouse.
 */
typedef struct {
    int x;                    /* Current X position */
    int y;                    /* Current Y position */
    bool buttons[3];          /* Currently held buttons */
    bool clicked[3];          /* Clicked this frame (cleared after read) */
    bool released[3];         /* Released this frame (cleared after read) */
    int wheel_delta;          /* Scroll wheel delta (cleared after read) */
} MouseState;
/* }}} */

/* {{{ KeyboardState
 * Current state of the keyboard.
 */
typedef struct {
    bool keys_down[256];      /* Currently held keys */
    bool keys_pressed[256];   /* Pressed this frame (cleared after read) */
    bool keys_released[256];  /* Released this frame (cleared after read) */
    bool shift;               /* Shift key held */
    bool ctrl;                /* Control key held */
    bool alt;                 /* Alt key held */
} KeyboardState;
/* }}} */

/* {{{ HitTarget
 * Result of a hit test.
 */
typedef enum {
    HIT_NONE,
    HIT_HAND_CARD,
    HIT_TRADE_ROW_CARD,
    HIT_PLAYER_BASE,
    HIT_OPP_BASE,
    HIT_PLAY_AREA,
    HIT_NARRATIVE,
    HIT_STATUS
} HitTargetType;

typedef struct {
    HitTargetType type;
    int index;                /* Card/base index if applicable */
    int x, y;                 /* Position within target zone */
} HitTarget;
/* }}} */

/* {{{ input_init
 * Initialize input handling.
 * @param canvas_id - Canvas element ID for mouse events
 * @return true on success
 */
bool input_init(const char* canvas_id);
/* }}} */

/* {{{ input_cleanup
 * Release input resources.
 */
void input_cleanup(void);
/* }}} */

/* {{{ input_update
 * Call at start of each frame to update input state.
 * Clears single-frame events from previous frame.
 */
void input_update(void);
/* }}} */

/* {{{ input_get_mouse
 * Get current mouse state.
 * @return Pointer to mouse state (do not free)
 */
const MouseState* input_get_mouse(void);
/* }}} */

/* {{{ input_get_keyboard
 * Get current keyboard state.
 * @return Pointer to keyboard state (do not free)
 */
const KeyboardState* input_get_keyboard(void);
/* }}} */

/* {{{ input_is_key_down
 * Check if a key is currently held.
 * @param key_code - Key code to check
 * @return true if key is held
 */
bool input_is_key_down(int key_code);
/* }}} */

/* {{{ input_is_key_pressed
 * Check if a key was pressed this frame.
 * @param key_code - Key code to check
 * @return true if key was just pressed
 */
bool input_is_key_pressed(int key_code);
/* }}} */

/* {{{ input_is_mouse_down
 * Check if a mouse button is currently held.
 * @param button - Button index (MOUSE_BUTTON_*)
 * @return true if button is held
 */
bool input_is_mouse_down(int button);
/* }}} */

/* {{{ input_is_mouse_clicked
 * Check if a mouse button was clicked this frame.
 * @param button - Button index (MOUSE_BUTTON_*)
 * @return true if button was just clicked
 */
bool input_is_mouse_clicked(int button);
/* }}} */

/* {{{ input_hit_test
 * Test what the mouse is over.
 * @param layout - Current layout for zone boundaries
 * @return Hit target information
 */
HitTarget input_hit_test(const Layout* layout);
/* }}} */

/* {{{ input_hit_test_zone
 * Test if a point is within a zone.
 * @param x, y - Point to test
 * @param zone - Zone to test against
 * @return true if point is in zone
 */
bool input_hit_test_zone(int x, int y, const Zone* zone);
/* }}} */

/* {{{ input_get_card_at
 * Get the card index at a position within a zone.
 * @param x - X position relative to zone
 * @param zone_w - Zone width
 * @param card_w - Card width
 * @param card_count - Number of cards
 * @return Card index, or -1 if none
 */
int input_get_card_at(int x, int zone_w, int card_w, int card_count);
/* }}} */

#endif /* WASM_INPUT_H */
