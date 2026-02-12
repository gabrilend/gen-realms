/*
 * draw2d.h - 2D Drawing Primitives for WASM Client
 *
 * Provides low-level drawing functions for rectangles, text, lines,
 * and other shapes. Uses inline JavaScript via EM_ASM to call
 * the canvas 2D context methods.
 */

#ifndef WASM_DRAW2D_H
#define WASM_DRAW2D_H

#include <stdint.h>

/* {{{ Text alignment */
typedef enum {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
} TextAlign;

typedef enum {
    TEXT_BASELINE_TOP,
    TEXT_BASELINE_MIDDLE,
    TEXT_BASELINE_BOTTOM
} TextBaseline;
/* }}} */

/* {{{ draw_rect
 * Draw a filled rectangle.
 * @param x, y - Top-left corner
 * @param w, h - Width and height
 * @param color - Fill color (RGB uint32_t)
 */
void draw_rect(int x, int y, int w, int h, uint32_t color);
/* }}} */

/* {{{ draw_rect_outline
 * Draw a rectangle outline (stroke only).
 * @param x, y - Top-left corner
 * @param w, h - Width and height
 * @param color - Stroke color (RGB uint32_t)
 * @param line_width - Line thickness
 */
void draw_rect_outline(int x, int y, int w, int h, uint32_t color, int line_width);
/* }}} */

/* {{{ draw_rounded_rect
 * Draw a filled rectangle with rounded corners.
 * @param x, y - Top-left corner
 * @param w, h - Width and height
 * @param radius - Corner radius
 * @param color - Fill color (RGB uint32_t)
 */
void draw_rounded_rect(int x, int y, int w, int h, int radius, uint32_t color);
/* }}} */

/* {{{ draw_rounded_rect_outline
 * Draw a rounded rectangle outline.
 * @param x, y - Top-left corner
 * @param w, h - Width and height
 * @param radius - Corner radius
 * @param color - Stroke color (RGB uint32_t)
 * @param line_width - Line thickness
 */
void draw_rounded_rect_outline(int x, int y, int w, int h, int radius,
                                uint32_t color, int line_width);
/* }}} */

/* {{{ draw_line
 * Draw a line between two points.
 * @param x1, y1 - Start point
 * @param x2, y2 - End point
 * @param color - Line color (RGB uint32_t)
 * @param line_width - Line thickness
 */
void draw_line(int x1, int y1, int x2, int y2, uint32_t color, int line_width);
/* }}} */

/* {{{ draw_circle
 * Draw a filled circle.
 * @param cx, cy - Center point
 * @param radius - Radius
 * @param color - Fill color (RGB uint32_t)
 */
void draw_circle(int cx, int cy, int radius, uint32_t color);
/* }}} */

/* {{{ draw_circle_outline
 * Draw a circle outline.
 * @param cx, cy - Center point
 * @param radius - Radius
 * @param color - Stroke color (RGB uint32_t)
 * @param line_width - Line thickness
 */
void draw_circle_outline(int cx, int cy, int radius, uint32_t color, int line_width);
/* }}} */

/* {{{ draw_arc
 * Draw an arc (partial circle).
 * @param cx, cy - Center point
 * @param radius - Radius
 * @param start_angle - Start angle in radians
 * @param end_angle - End angle in radians
 * @param color - Stroke color (RGB uint32_t)
 * @param line_width - Line thickness
 */
void draw_arc(int cx, int cy, int radius, float start_angle, float end_angle,
              uint32_t color, int line_width);
/* }}} */

/* {{{ draw_text
 * Draw text at a position.
 * @param x, y - Position
 * @param text - Text to draw
 * @param color - Text color (RGB uint32_t)
 */
void draw_text(int x, int y, const char* text, uint32_t color);
/* }}} */

/* {{{ draw_text_ex
 * Draw text with extended options.
 * @param x, y - Position
 * @param text - Text to draw
 * @param color - Text color (RGB uint32_t)
 * @param font_size - Font size in pixels
 * @param align - Horizontal alignment
 * @param baseline - Vertical alignment
 */
void draw_text_ex(int x, int y, const char* text, uint32_t color,
                  int font_size, TextAlign align, TextBaseline baseline);
/* }}} */

/* {{{ draw_text_bold
 * Draw bold text.
 * @param x, y - Position
 * @param text - Text to draw
 * @param color - Text color (RGB uint32_t)
 * @param font_size - Font size in pixels
 */
void draw_text_bold(int x, int y, const char* text, uint32_t color, int font_size);
/* }}} */

/* {{{ draw_measure_text
 * Measure the width of text.
 * @param text - Text to measure
 * @param font_size - Font size in pixels
 * @return Width in pixels
 */
int draw_measure_text(const char* text, int font_size);
/* }}} */

/* {{{ draw_set_clip
 * Set a clipping rectangle.
 * @param x, y - Top-left corner
 * @param w, h - Width and height
 */
void draw_set_clip(int x, int y, int w, int h);
/* }}} */

/* {{{ draw_clear_clip
 * Clear the clipping rectangle.
 */
void draw_clear_clip(void);
/* }}} */

/* {{{ draw_set_alpha
 * Set the global alpha (transparency).
 * @param alpha - Alpha value 0.0-1.0
 */
void draw_set_alpha(float alpha);
/* }}} */

/* {{{ draw_reset_alpha
 * Reset alpha to 1.0 (fully opaque).
 */
void draw_reset_alpha(void);
/* }}} */

#endif /* WASM_DRAW2D_H */
