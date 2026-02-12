/*
 * draw2d.c - 2D Drawing Primitives for WASM Client
 *
 * Implements low-level drawing functions using inline JavaScript
 * via EM_ASM to call canvas 2D context methods.
 */

#include "draw2d.h"
#include <emscripten.h>

/* {{{ draw_rect */
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($4 >> 16) & 0xFF;
        var g = ($4 >> 8) & 0xFF;
        var b = $4 & 0xFF;

        ctx.fillStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.fillRect($0, $1, $2, $3);
    }, x, y, w, h, color);
}
/* }}} */

/* {{{ draw_rect_outline */
void draw_rect_outline(int x, int y, int w, int h, uint32_t color, int line_width) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($4 >> 16) & 0xFF;
        var g = ($4 >> 8) & 0xFF;
        var b = $4 & 0xFF;

        ctx.strokeStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.lineWidth = $5;
        ctx.strokeRect($0 + 0.5, $1 + 0.5, $2 - 1, $3 - 1);
    }, x, y, w, h, color, line_width);
}
/* }}} */

/* {{{ draw_rounded_rect */
void draw_rounded_rect(int x, int y, int w, int h, int radius, uint32_t color) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($5 >> 16) & 0xFF;
        var g = ($5 >> 8) & 0xFF;
        var b = $5 & 0xFF;

        ctx.fillStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.beginPath();

        /* Use roundRect if available, otherwise fallback */
        if (ctx.roundRect) {
            ctx.roundRect($0, $1, $2, $3, $4);
        } else {
            /* Manual rounded rect path */
            var x = $0, y = $1, w = $2, h = $3, rad = $4;
            ctx.moveTo(x + rad, y);
            ctx.lineTo(x + w - rad, y);
            ctx.quadraticCurveTo(x + w, y, x + w, y + rad);
            ctx.lineTo(x + w, y + h - rad);
            ctx.quadraticCurveTo(x + w, y + h, x + w - rad, y + h);
            ctx.lineTo(x + rad, y + h);
            ctx.quadraticCurveTo(x, y + h, x, y + h - rad);
            ctx.lineTo(x, y + rad);
            ctx.quadraticCurveTo(x, y, x + rad, y);
        }

        ctx.fill();
    }, x, y, w, h, radius, color);
}
/* }}} */

/* {{{ draw_rounded_rect_outline */
void draw_rounded_rect_outline(int x, int y, int w, int h, int radius,
                                uint32_t color, int line_width) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($5 >> 16) & 0xFF;
        var g = ($5 >> 8) & 0xFF;
        var b = $5 & 0xFF;

        ctx.strokeStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.lineWidth = $6;
        ctx.beginPath();

        /* Use roundRect if available, otherwise fallback */
        if (ctx.roundRect) {
            ctx.roundRect($0, $1, $2, $3, $4);
        } else {
            /* Manual rounded rect path */
            var x = $0, y = $1, w = $2, h = $3, rad = $4;
            ctx.moveTo(x + rad, y);
            ctx.lineTo(x + w - rad, y);
            ctx.quadraticCurveTo(x + w, y, x + w, y + rad);
            ctx.lineTo(x + w, y + h - rad);
            ctx.quadraticCurveTo(x + w, y + h, x + w - rad, y + h);
            ctx.lineTo(x + rad, y + h);
            ctx.quadraticCurveTo(x, y + h, x, y + h - rad);
            ctx.lineTo(x, y + rad);
            ctx.quadraticCurveTo(x, y, x + rad, y);
        }

        ctx.stroke();
    }, x, y, w, h, radius, color, line_width);
}
/* }}} */

/* {{{ draw_line */
void draw_line(int x1, int y1, int x2, int y2, uint32_t color, int line_width) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($4 >> 16) & 0xFF;
        var g = ($4 >> 8) & 0xFF;
        var b = $4 & 0xFF;

        ctx.strokeStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.lineWidth = $5;
        ctx.beginPath();
        ctx.moveTo($0 + 0.5, $1 + 0.5);
        ctx.lineTo($2 + 0.5, $3 + 0.5);
        ctx.stroke();
    }, x1, y1, x2, y2, color, line_width);
}
/* }}} */

/* {{{ draw_circle */
void draw_circle(int cx, int cy, int radius, uint32_t color) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($3 >> 16) & 0xFF;
        var g = ($3 >> 8) & 0xFF;
        var b = $3 & 0xFF;

        ctx.fillStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.beginPath();
        ctx.arc($0, $1, $2, 0, Math.PI * 2);
        ctx.fill();
    }, cx, cy, radius, color);
}
/* }}} */

/* {{{ draw_circle_outline */
void draw_circle_outline(int cx, int cy, int radius, uint32_t color, int line_width) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($3 >> 16) & 0xFF;
        var g = ($3 >> 8) & 0xFF;
        var b = $3 & 0xFF;

        ctx.strokeStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.lineWidth = $4;
        ctx.beginPath();
        ctx.arc($0, $1, $2, 0, Math.PI * 2);
        ctx.stroke();
    }, cx, cy, radius, color, line_width);
}
/* }}} */

/* {{{ draw_arc */
void draw_arc(int cx, int cy, int radius, float start_angle, float end_angle,
              uint32_t color, int line_width) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($5 >> 16) & 0xFF;
        var g = ($5 >> 8) & 0xFF;
        var b = $5 & 0xFF;

        ctx.strokeStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.lineWidth = $6;
        ctx.beginPath();
        ctx.arc($0, $1, $2, $3, $4);
        ctx.stroke();
    }, cx, cy, radius, start_angle, end_angle, color, line_width);
}
/* }}} */

/* {{{ draw_text */
void draw_text(int x, int y, const char* text, uint32_t color) {
    draw_text_ex(x, y, text, color, 14, TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);
}
/* }}} */

/* {{{ draw_text_ex */
void draw_text_ex(int x, int y, const char* text, uint32_t color,
                  int font_size, TextAlign align, TextBaseline baseline) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($3 >> 16) & 0xFF;
        var g = ($3 >> 8) & 0xFF;
        var b = $3 & 0xFF;

        ctx.fillStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.font = $4 + 'px monospace';

        /* Text alignment */
        var alignStr = 'left';
        if ($5 === 1) alignStr = 'center';
        else if ($5 === 2) alignStr = 'right';
        ctx.textAlign = alignStr;

        /* Text baseline */
        var baselineStr = 'top';
        if ($6 === 1) baselineStr = 'middle';
        else if ($6 === 2) baselineStr = 'bottom';
        ctx.textBaseline = baselineStr;

        ctx.fillText(UTF8ToString($2), $0, $1);
    }, x, y, text, color, font_size, align, baseline);
}
/* }}} */

/* {{{ draw_text_bold */
void draw_text_bold(int x, int y, const char* text, uint32_t color, int font_size) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        var r = ($3 >> 16) & 0xFF;
        var g = ($3 >> 8) & 0xFF;
        var b = $3 & 0xFF;

        ctx.fillStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        ctx.font = 'bold ' + $4 + 'px monospace';
        ctx.textAlign = 'left';
        ctx.textBaseline = 'top';
        ctx.fillText(UTF8ToString($2), $0, $1);
    }, x, y, text, color, font_size);
}
/* }}} */

/* {{{ draw_measure_text */
int draw_measure_text(const char* text, int font_size) {
    return EM_ASM_INT({
        var ctx = Module.ctx;
        if (!ctx) return 0;

        ctx.font = $1 + 'px monospace';
        var metrics = ctx.measureText(UTF8ToString($0));
        return Math.ceil(metrics.width);
    }, text, font_size);
}
/* }}} */

/* {{{ draw_set_clip */
void draw_set_clip(int x, int y, int w, int h) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        ctx.save();
        ctx.beginPath();
        ctx.rect($0, $1, $2, $3);
        ctx.clip();
    }, x, y, w, h);
}
/* }}} */

/* {{{ draw_clear_clip */
void draw_clear_clip(void) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        ctx.restore();
    });
}
/* }}} */

/* {{{ draw_set_alpha */
void draw_set_alpha(float alpha) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        ctx.globalAlpha = $0;
    }, alpha);
}
/* }}} */

/* {{{ draw_reset_alpha */
void draw_reset_alpha(void) {
    EM_ASM({
        var ctx = Module.ctx;
        if (!ctx) return;

        ctx.globalAlpha = 1.0;
    });
}
/* }}} */
