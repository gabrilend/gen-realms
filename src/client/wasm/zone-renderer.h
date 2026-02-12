/*
 * zone-renderer.h - Zone Rendering for WASM Client
 *
 * Renders game zones: trade row, hand, bases, play area.
 * Uses card-renderer for individual cards.
 */

#ifndef WASM_ZONE_RENDERER_H
#define WASM_ZONE_RENDERER_H

#include <stdbool.h>
#include <stdint.h>
#include "theme.h"
#include "card-renderer.h"

/* {{{ Maximum cards per zone */
#define MAX_HAND_CARDS 12
#define MAX_TRADE_ROW_CARDS 5
#define MAX_BASES 4
#define MAX_PLAY_AREA_CARDS 10
/* }}} */

/* {{{ ZoneCard
 * Card data within a zone.
 */
typedef struct {
    CardRenderData render_data;
    int id;                        /* Card ID for interaction */
    bool visible;                  /* Whether card is face-up */
} ZoneCard;
/* }}} */

/* {{{ HandZoneData
 * Data for the player's hand zone.
 */
typedef struct {
    ZoneCard cards[MAX_HAND_CARDS];
    int card_count;
    int selected_index;            /* -1 if none selected */
    int hover_index;               /* -1 if none hovered */
} HandZoneData;
/* }}} */

/* {{{ TradeRowData
 * Data for the trade row zone.
 */
typedef struct {
    ZoneCard cards[MAX_TRADE_ROW_CARDS];
    int card_count;
    int deck_count;                /* Cards remaining in trade deck */
    int hover_index;               /* -1 if none hovered */
    bool show_explorer;            /* Show explorer pile */
} TradeRowData;
/* }}} */

/* {{{ BasesZoneData
 * Data for a bases zone (player or opponent).
 */
typedef struct {
    ZoneCard bases[MAX_BASES];
    int base_count;
    int hover_index;               /* -1 if none hovered */
    int targeted_index;            /* -1 if none targeted */
} BasesZoneData;
/* }}} */

/* {{{ PlayAreaData
 * Data for the central play area.
 */
typedef struct {
    ZoneCard cards[MAX_PLAY_AREA_CARDS];
    int card_count;
    int hover_index;               /* -1 if none hovered */
} PlayAreaData;
/* }}} */

/* {{{ zone_render_hand
 * Render the player's hand zone.
 * @param zone - Zone boundaries
 * @param data - Hand zone data
 * @param card_w, card_h - Card dimensions
 */
void zone_render_hand(const Zone* zone, const HandZoneData* data, int card_w, int card_h);
/* }}} */

/* {{{ zone_render_trade_row
 * Render the trade row zone.
 * @param zone - Zone boundaries
 * @param data - Trade row data
 * @param card_w, card_h - Card dimensions
 */
void zone_render_trade_row(const Zone* zone, const TradeRowData* data, int card_w, int card_h);
/* }}} */

/* {{{ zone_render_bases
 * Render a bases zone.
 * @param zone - Zone boundaries
 * @param data - Bases zone data
 * @param card_w, card_h - Card dimensions
 * @param is_opponent - true if rendering opponent's bases
 */
void zone_render_bases(const Zone* zone, const BasesZoneData* data,
                       int card_w, int card_h, bool is_opponent);
/* }}} */

/* {{{ zone_render_play_area
 * Render the central play area.
 * @param zone - Zone boundaries
 * @param data - Play area data
 * @param card_w, card_h - Card dimensions
 */
void zone_render_play_area(const Zone* zone, const PlayAreaData* data, int card_w, int card_h);
/* }}} */

/* {{{ zone_get_card_x
 * Calculate the X position for a card at a given index.
 * Centers cards within the zone.
 * @param zone_x - Zone X position
 * @param zone_w - Zone width
 * @param card_w - Card width
 * @param card_count - Total cards in zone
 * @param index - Card index
 * @return X position for the card
 */
int zone_get_card_x(int zone_x, int zone_w, int card_w, int card_count, int index);
/* }}} */

/* {{{ zone_get_card_y
 * Calculate the Y position for cards in a zone.
 * Centers cards vertically with padding.
 * @param zone_y - Zone Y position
 * @param zone_h - Zone height
 * @param card_h - Card height
 * @return Y position for cards
 */
int zone_get_card_y(int zone_y, int zone_h, int card_h);
/* }}} */

#endif /* WASM_ZONE_RENDERER_H */
