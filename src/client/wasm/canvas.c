/*
 * canvas.c - HTML5 Canvas Management for WASM Client
 *
 * Implements canvas initialization, resize handling, and render loop
 * using Emscripten HTML5 API and inline JavaScript via EM_ASM.
 */

#include "canvas.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* {{{ Global state */
static CanvasState g_canvas = {0};
static RenderCallback g_render_callback = NULL;
static char g_canvas_id[64] = "canvas";
/* }}} */

/* {{{ resize_callback
 * Called when the window/container resizes.
 */
static EM_BOOL resize_callback(int event_type, const EmscriptenUiEvent* ui_event, void* user_data) {
    (void)event_type;
    (void)ui_event;
    (void)user_data;

    /* Get container size and resize canvas */
    EM_ASM({
        var canvas = document.getElementById(UTF8ToString($0));
        if (canvas && canvas.parentElement) {
            var container = canvas.parentElement;
            canvas.width = Math.max(container.clientWidth, $1);
            canvas.height = Math.max(container.clientHeight, $2);
        }
    }, g_canvas_id, CANVAS_MIN_WIDTH, CANVAS_MIN_HEIGHT);

    /* Update our stored size */
    EM_ASM({
        var canvas = document.getElementById(UTF8ToString($0));
        if (canvas) {
            setValue($1, canvas.width, 'i32');
            setValue($2, canvas.height, 'i32');
        }
    }, g_canvas_id, &g_canvas.width, &g_canvas.height);

    g_canvas.needs_render = true;

    return EM_TRUE;
}
/* }}} */

/* {{{ main_loop
 * Main render loop callback for emscripten_set_main_loop.
 */
static void main_loop(void) {
    double current_time = emscripten_get_now();

    /* Calculate delta time */
    if (g_canvas.last_frame_time > 0) {
        g_canvas.delta_time = current_time - g_canvas.last_frame_time;
    } else {
        g_canvas.delta_time = 16.67; /* Assume 60fps on first frame */
    }
    g_canvas.last_frame_time = current_time;

    /* Update FPS counter */
    g_canvas.frame_count++;
    if (current_time - g_canvas.fps_update_time >= 1000.0) {
        g_canvas.fps = g_canvas.frame_count;
        g_canvas.frame_count = 0;
        g_canvas.fps_update_time = current_time;
    }

    /* Call render callback */
    if (g_render_callback != NULL) {
        g_render_callback(g_canvas.delta_time);
    }

    g_canvas.needs_render = false;
}
/* }}} */

/* {{{ canvas_init */
bool canvas_init(const char* canvas_id) {
    if (g_canvas.initialized) {
        canvas_cleanup();
    }

    /* Store canvas ID */
    if (canvas_id != NULL) {
        strncpy(g_canvas_id, canvas_id, sizeof(g_canvas_id) - 1);
        g_canvas_id[sizeof(g_canvas_id) - 1] = '\0';
    }

    /* Initialize canvas in browser */
    int success = EM_ASM_INT({
        var canvas = document.getElementById(UTF8ToString($0));
        if (!canvas) {
            console.error('[Canvas] Element not found:', UTF8ToString($0));
            return 0;
        }

        /* Get 2D context */
        var ctx = canvas.getContext('2d', { alpha: false });
        if (!ctx) {
            console.error('[Canvas] Failed to get 2D context');
            return 0;
        }

        /* Store context reference for later use */
        Module.canvas = canvas;
        Module.ctx = ctx;

        /* Initial resize */
        var container = canvas.parentElement;
        if (container) {
            canvas.width = Math.max(container.clientWidth, $1);
            canvas.height = Math.max(container.clientHeight, $2);
        }

        console.log('[Canvas] Initialized', canvas.width + 'x' + canvas.height);
        return 1;
    }, g_canvas_id, CANVAS_MIN_WIDTH, CANVAS_MIN_HEIGHT);

    if (!success) {
        return false;
    }

    /* Get initial size */
    EM_ASM({
        var canvas = Module.canvas;
        if (canvas) {
            setValue($0, canvas.width, 'i32');
            setValue($1, canvas.height, 'i32');
        }
    }, &g_canvas.width, &g_canvas.height);

    /* Register resize handler */
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_TRUE, resize_callback);

    g_canvas.initialized = true;
    g_canvas.needs_render = true;
    g_canvas.fps_update_time = emscripten_get_now();

    return true;
}
/* }}} */

/* {{{ canvas_cleanup */
void canvas_cleanup(void) {
    if (!g_canvas.initialized) return;

    canvas_stop_render_loop();

    /* Clear context reference */
    EM_ASM({
        Module.canvas = null;
        Module.ctx = null;
    });

    memset(&g_canvas, 0, sizeof(g_canvas));
    g_render_callback = NULL;
}
/* }}} */

/* {{{ canvas_get_size */
void canvas_get_size(int* out_width, int* out_height) {
    if (out_width) *out_width = g_canvas.width;
    if (out_height) *out_height = g_canvas.height;
}
/* }}} */

/* {{{ canvas_set_render_callback */
void canvas_set_render_callback(RenderCallback callback) {
    g_render_callback = callback;
}
/* }}} */

/* {{{ canvas_start_render_loop */
void canvas_start_render_loop(void) {
    if (!g_canvas.initialized) return;

    g_canvas.last_frame_time = 0;
    g_canvas.frame_count = 0;
    g_canvas.fps = 0;
    g_canvas.fps_update_time = emscripten_get_now();

    /* Use 0 for fps to let browser control frame rate (requestAnimationFrame) */
    emscripten_set_main_loop(main_loop, 0, 0);
}
/* }}} */

/* {{{ canvas_stop_render_loop */
void canvas_stop_render_loop(void) {
    emscripten_cancel_main_loop();
}
/* }}} */

/* {{{ canvas_request_render */
void canvas_request_render(void) {
    g_canvas.needs_render = true;
}
/* }}} */

/* {{{ canvas_clear */
void canvas_clear(uint32_t color) {
    if (!g_canvas.initialized) return;

    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($0 >> 16) & 0xFF;
        var g = ($0 >> 8) & 0xFF;
        var b = $0 & 0xFF;

        ctx.fillStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.fillRect(0, 0, Module.canvas.width, Module.canvas.height);
    }, color);
}
/* }}} */

/* {{{ canvas_get_fps */
int canvas_get_fps(void) {
    return g_canvas.fps;
}
/* }}} */

/* {{{ canvas_get_delta_time */
double canvas_get_delta_time(void) {
    return g_canvas.delta_time;
}
/* }}} */
