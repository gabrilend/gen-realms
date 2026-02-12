/*
 * canvas.h - HTML5 Canvas Management for WASM Client
 *
 * Provides canvas initialization, resize handling, and render loop
 * management. Uses Emscripten HTML5 API directly without external JS.
 */

#ifndef WASM_CANVAS_H
#define WASM_CANVAS_H

#include <stdbool.h>
#include <stdint.h>

/* {{{ Configuration */
#define CANVAS_MIN_WIDTH  800
#define CANVAS_MIN_HEIGHT 600
#define CANVAS_TARGET_FPS 60
/* }}} */

/* {{{ Color helpers
 * Colors are stored as RGB uint32_t: 0xRRGGBB
 */
#define RGB(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))
#define RGB_R(c) (((c) >> 16) & 0xFF)
#define RGB_G(c) (((c) >> 8) & 0xFF)
#define RGB_B(c) ((c) & 0xFF)
/* }}} */

/* {{{ Standard colors */
#define COLOR_BLACK       RGB(0x00, 0x00, 0x00)
#define COLOR_WHITE       RGB(0xFF, 0xFF, 0xFF)
#define COLOR_BG_DARK     RGB(0x1a, 0x1a, 0x2e)
#define COLOR_GRID        RGB(0x2a, 0x2a, 0x4e)
#define COLOR_BORDER      RGB(0x44, 0x44, 0x44)
#define COLOR_TEXT        RGB(0xCC, 0xCC, 0xCC)
#define COLOR_TEXT_DIM    RGB(0x88, 0x88, 0x88)

/* Faction colors */
#define COLOR_MERCHANT    RGB(0xd4, 0xa0, 0x17)
#define COLOR_WILDS       RGB(0x2d, 0x7a, 0x2d)
#define COLOR_KINGDOM     RGB(0x33, 0x66, 0xcc)
#define COLOR_ARTIFICER   RGB(0xcc, 0x33, 0x33)
#define COLOR_NEUTRAL     RGB(0x88, 0x88, 0x88)

/* Value colors */
#define COLOR_AUTHORITY   RGB(0x44, 0xcc, 0xcc)
#define COLOR_COMBAT      RGB(0xcc, 0x66, 0xcc)
#define COLOR_TRADE       RGB(0xd4, 0xa0, 0x17)
/* }}} */

/* {{{ CanvasState
 * Internal canvas state structure.
 */
typedef struct {
    int width;
    int height;
    bool initialized;
    bool needs_render;

    /* Render loop */
    double last_frame_time;
    double delta_time;
    int frame_count;
    int fps;
    double fps_update_time;
} CanvasState;
/* }}} */

/* {{{ Render callback type */
typedef void (*RenderCallback)(double delta_time);
/* }}} */

/* {{{ canvas_init
 * Initialize the canvas system.
 * @param canvas_id - HTML element ID (e.g., "#canvas" or "canvas")
 * @return true on success
 */
bool canvas_init(const char* canvas_id);
/* }}} */

/* {{{ canvas_cleanup
 * Release canvas resources.
 */
void canvas_cleanup(void);
/* }}} */

/* {{{ canvas_get_size
 * Get current canvas dimensions.
 * @param out_width - Output: canvas width
 * @param out_height - Output: canvas height
 */
void canvas_get_size(int* out_width, int* out_height);
/* }}} */

/* {{{ canvas_set_render_callback
 * Set the function called each frame to render.
 * @param callback - Function to call each frame
 */
void canvas_set_render_callback(RenderCallback callback);
/* }}} */

/* {{{ canvas_start_render_loop
 * Start the render loop using emscripten_set_main_loop.
 */
void canvas_start_render_loop(void);
/* }}} */

/* {{{ canvas_stop_render_loop
 * Stop the render loop.
 */
void canvas_stop_render_loop(void);
/* }}} */

/* {{{ canvas_request_render
 * Request a render on the next frame.
 */
void canvas_request_render(void);
/* }}} */

/* {{{ canvas_clear
 * Clear the canvas with the background color.
 * @param color - Background color (RGB uint32_t)
 */
void canvas_clear(uint32_t color);
/* }}} */

/* {{{ canvas_get_fps
 * Get current frames per second.
 * @return FPS value
 */
int canvas_get_fps(void);
/* }}} */

/* {{{ canvas_get_delta_time
 * Get time since last frame in milliseconds.
 * @return Delta time in ms
 */
double canvas_get_delta_time(void);
/* }}} */

#endif /* WASM_CANVAS_H */
