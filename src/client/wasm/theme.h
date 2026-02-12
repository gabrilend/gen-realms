/*
 * theme.h - Theme and Layout Constants for WASM Client
 *
 * Defines all colors, font sizes, spacing, and layout zones as
 * compile-time constants. Replaces CSS styling for the WASM client.
 */

#ifndef WASM_THEME_H
#define WASM_THEME_H

#include "canvas.h"

/* {{{ Font sizes */
#define FONT_SIZE_SMALL   10
#define FONT_SIZE_NORMAL  14
#define FONT_SIZE_LARGE   18
#define FONT_SIZE_TITLE   24
/* }}} */

/* {{{ Spacing */
#define PADDING_SMALL     5
#define PADDING_NORMAL    10
#define PADDING_LARGE     20

#define CARD_SPACING      10
#define ZONE_SPACING      10
/* }}} */

/* {{{ Card dimensions */
#define CARD_WIDTH        100
#define CARD_HEIGHT       140
#define CARD_WIDTH_SMALL  80
#define CARD_HEIGHT_SMALL 112
#define CARD_CORNER_RADIUS 6
/* }}} */

/* {{{ Status bar */
#define STATUS_BAR_HEIGHT 40
/* }}} */

/* {{{ Narrative panel */
#define NARRATIVE_MIN_WIDTH   200
#define NARRATIVE_MAX_WIDTH   300
#define NARRATIVE_WIDTH_PCT   25  /* 25% of screen width */
/* }}} */

/* {{{ Background colors */
#define THEME_BG_DARK       RGB(0x0d, 0x0d, 0x1a)
#define THEME_BG_PANEL      RGB(0x1a, 0x1a, 0x2e)
#define THEME_BG_CARD       RGB(0x22, 0x22, 0x38)
#define THEME_BG_HIGHLIGHT  RGB(0x2a, 0x2a, 0x4e)
/* }}} */

/* {{{ Border colors */
#define THEME_BORDER_DIM    RGB(0x33, 0x33, 0x44)
#define THEME_BORDER_NORMAL RGB(0x44, 0x44, 0x55)
#define THEME_BORDER_BRIGHT RGB(0x66, 0x66, 0x77)
/* }}} */

/* {{{ Text colors */
#define THEME_TEXT_PRIMARY   RGB(0xee, 0xee, 0xee)
#define THEME_TEXT_SECONDARY RGB(0xcc, 0xcc, 0xcc)
#define THEME_TEXT_DIM       RGB(0x88, 0x88, 0x88)
#define THEME_TEXT_DISABLED  RGB(0x55, 0x55, 0x55)
/* }}} */

/* {{{ Faction colors (primary) */
#define THEME_FACTION_MERCHANT  RGB(0xd4, 0xa0, 0x17)
#define THEME_FACTION_WILDS     RGB(0x2d, 0x7a, 0x2d)
#define THEME_FACTION_KINGDOM   RGB(0x33, 0x66, 0xcc)
#define THEME_FACTION_ARTIFICER RGB(0xcc, 0x33, 0x33)
#define THEME_FACTION_NEUTRAL   RGB(0x88, 0x88, 0x88)
/* }}} */

/* {{{ Faction colors (secondary/darker) */
#define THEME_FACTION_MERCHANT_DARK  RGB(0x8b, 0x69, 0x14)
#define THEME_FACTION_WILDS_DARK     RGB(0x1d, 0x4d, 0x1d)
#define THEME_FACTION_KINGDOM_DARK   RGB(0x22, 0x44, 0x88)
#define THEME_FACTION_ARTIFICER_DARK RGB(0x88, 0x22, 0x22)
#define THEME_FACTION_NEUTRAL_DARK   RGB(0x55, 0x55, 0x55)
/* }}} */

/* {{{ Value colors */
#define THEME_VALUE_AUTHORITY RGB(0x44, 0xcc, 0xcc)
#define THEME_VALUE_COMBAT    RGB(0xcc, 0x66, 0xcc)
#define THEME_VALUE_TRADE     RGB(0xd4, 0xa0, 0x17)
#define THEME_VALUE_DRAW      RGB(0x66, 0xcc, 0x66)
/* }}} */

/* {{{ Status colors */
#define THEME_STATUS_SUCCESS  RGB(0x44, 0xcc, 0x44)
#define THEME_STATUS_WARNING  RGB(0xcc, 0xcc, 0x44)
#define THEME_STATUS_ERROR    RGB(0xcc, 0x44, 0x44)
#define THEME_STATUS_INFO     RGB(0x44, 0x88, 0xcc)
/* }}} */

/* {{{ Animation colors */
#define THEME_GLOW_ALLY       RGB(0xff, 0xd7, 0x00)
#define THEME_GLOW_EMPOWERED  RGB(0xff, 0x44, 0xff)
#define THEME_GLOW_TARGETED   RGB(0xff, 0x44, 0x44)
/* }}} */

/* {{{ Layout Zone
 * Represents a rectangular area on screen.
 */
typedef struct {
    int x;
    int y;
    int w;
    int h;
} Zone;
/* }}} */

/* {{{ Layout
 * All game layout zones, calculated based on canvas size.
 */
typedef struct {
    Zone status;        /* Status bar at top */
    Zone hand;          /* Player's hand (bottom center) */
    Zone trade_row;     /* Trade row (top center) */
    Zone player_bases;  /* Player's bases (bottom left) */
    Zone opp_bases;     /* Opponent's bases (top left) */
    Zone narrative;     /* Narrative panel (right side) */
    Zone play_area;     /* Play area (center) */

    /* Calculated card size (adapts to screen) */
    int card_w;
    int card_h;
} Layout;
/* }}} */

/* {{{ theme_calculate_layout
 * Calculate layout zones based on canvas size.
 * @param layout - Layout structure to fill
 * @param canvas_w - Canvas width
 * @param canvas_h - Canvas height
 */
void theme_calculate_layout(Layout* layout, int canvas_w, int canvas_h);
/* }}} */

/* {{{ theme_get_faction_color
 * Get the primary color for a faction.
 * @param faction - Faction name ("merchant", "wilds", etc.)
 * @return Color as RGB uint32_t
 */
uint32_t theme_get_faction_color(const char* faction);
/* }}} */

/* {{{ theme_get_faction_color_dark
 * Get the secondary/dark color for a faction.
 * @param faction - Faction name
 * @return Color as RGB uint32_t
 */
uint32_t theme_get_faction_color_dark(const char* faction);
/* }}} */

/* {{{ theme_get_value_color
 * Get the color for a value type.
 * @param value_type - "authority", "combat", "trade", "draw"
 * @return Color as RGB uint32_t
 */
uint32_t theme_get_value_color(const char* value_type);
/* }}} */

#endif /* WASM_THEME_H */
