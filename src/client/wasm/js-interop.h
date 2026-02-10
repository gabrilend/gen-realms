/* js-interop.h - JavaScript interoperability declarations for Wasm client
 *
 * Defines the interface between C code and JavaScript. Functions prefixed
 * with js_ call into JavaScript; functions prefixed with client_ are called
 * from JavaScript.
 */

#ifndef JS_INTEROP_H
#define JS_INTEROP_H

#include <emscripten.h>

/* {{{ JavaScript function declarations (C calls into JS)
 * These are implemented using EM_JS in js-interop.c
 */

/* Logging functions */
extern void js_log_info(const char* message);
extern void js_log_error(const char* message);

/* WebSocket functions */
extern void js_send_message(const char* json);
extern int js_is_connected(void);

/* Canvas rendering functions */
extern void js_clear_canvas(void);
extern void js_draw_rect(int x, int y, int w, int h, const char* color);
extern void js_draw_text(int x, int y, const char* text);
extern void js_draw_card(int x, int y, int w, int h, const char* card_json);
extern void js_request_render(void);

/* localStorage functions */
extern void js_storage_set(const char* key, const char* value);
extern const char* js_storage_get(const char* key);

/* }}} */

/* {{{ Internal functions (called from JS via exported wrappers) */

/* Set player action - called from JS input handlers */
void client_set_action(const char* json_action);

/* }}} */

#endif /* JS_INTEROP_H */
