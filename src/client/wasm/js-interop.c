/* js-interop.c - JavaScript interoperability implementation
 *
 * Implements the EM_JS functions that allow C code to call into JavaScript.
 * These functions bridge the gap between the Wasm game logic and the
 * browser's DOM, WebSocket, and Canvas APIs.
 */

#include "js-interop.h"
#include <emscripten.h>

/* {{{ Logging functions
 * Log messages to the browser console with appropriate severity levels.
 */

EM_JS(void, js_log_info, (const char* message), {
    console.log('[Symbeline]', UTF8ToString(message));
});

EM_JS(void, js_log_error, (const char* message), {
    console.error('[Symbeline]', UTF8ToString(message));
});

/* }}} */

/* {{{ WebSocket functions
 * Interface with the WebSocket connection managed by JavaScript.
 * The actual WebSocket object is created and managed in symbeline.js.
 */

EM_JS(void, js_send_message, (const char* json), {
    if (typeof window.symbeline !== 'undefined' &&
        typeof window.symbeline.sendMessage === 'function') {
        window.symbeline.sendMessage(UTF8ToString(json));
    } else {
        console.error('[Symbeline] WebSocket not initialized');
    }
});

EM_JS(int, js_is_connected, (void), {
    if (typeof window.symbeline !== 'undefined' &&
        typeof window.symbeline.isConnected === 'function') {
        return window.symbeline.isConnected() ? 1 : 0;
    }
    return 0;
});

/* }}} */

/* {{{ Canvas rendering functions
 * Interface with the HTML5 Canvas for rendering the game UI.
 * The canvas context is managed by JavaScript in symbeline.js.
 */

EM_JS(void, js_clear_canvas, (void), {
    if (typeof window.symbeline !== 'undefined' &&
        typeof window.symbeline.clearCanvas === 'function') {
        window.symbeline.clearCanvas();
    }
});

EM_JS(void, js_draw_rect, (int x, int y, int w, int h, const char* color), {
    if (typeof window.symbeline !== 'undefined' &&
        typeof window.symbeline.drawRect === 'function') {
        window.symbeline.drawRect(x, y, w, h, UTF8ToString(color));
    }
});

EM_JS(void, js_draw_text, (int x, int y, const char* text), {
    if (typeof window.symbeline !== 'undefined' &&
        typeof window.symbeline.drawText === 'function') {
        window.symbeline.drawText(x, y, UTF8ToString(text));
    }
});

EM_JS(void, js_draw_card, (int x, int y, int w, int h, const char* card_json), {
    if (typeof window.symbeline !== 'undefined' &&
        typeof window.symbeline.drawCard === 'function') {
        window.symbeline.drawCard(x, y, w, h, UTF8ToString(card_json));
    }
});

EM_JS(void, js_request_render, (void), {
    if (typeof window.symbeline !== 'undefined' &&
        typeof window.symbeline.requestRender === 'function') {
        window.symbeline.requestRender();
    }
});

/* }}} */

/* {{{ localStorage functions
 * Persist client preferences and cached data using browser localStorage.
 * Used for style preferences, cached narratives, etc.
 */

EM_JS(void, js_storage_set, (const char* key, const char* value), {
    try {
        var k = 'symbeline_' + UTF8ToString(key);
        localStorage.setItem(k, UTF8ToString(value));
    } catch (e) {
        console.error('[Symbeline] localStorage write failed:', e);
    }
});

EM_JS(const char*, js_storage_get, (const char* key), {
    try {
        var k = 'symbeline_' + UTF8ToString(key);
        var value = localStorage.getItem(k);
        if (value === null) return 0;

        /* Allocate memory for the string in Wasm heap */
        var len = lengthBytesUTF8(value) + 1;
        var ptr = _malloc(len);
        stringToUTF8(value, ptr, len);
        return ptr;
    } catch (e) {
        console.error('[Symbeline] localStorage read failed:', e);
        return 0;
    }
});

/* }}} */
