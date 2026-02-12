/*
 * zone-renderer.c - Zone Rendering Implementation for WASM Client
 *
 * Renders game zones with cards, backgrounds, and visual states.
 * Zones include: trade row, hand, bases, and play area.
 */

#include "zone-renderer.h"
#include "card-renderer.h"
#include "draw2d.h"
#include "theme.h"
#include <stdio.h>

/* {{{ zone_get_card_x */
int zone_get_card_x(int zone_x, int zone_w, int card_w, int card_count, int index) {
    if (card_count <= 0) return zone_x + PADDING_NORMAL;

    /* Total width needed for all cards */
    int total_width = card_count * card_w + (card_count - 1) * CARD_SPACING;

    /* Center cards in zone */
    int start_x = zone_x + (zone_w - total_width) / 2;

    /* Ensure minimum padding */
    if (start_x < zone_x + PADDING_NORMAL) {
        start_x = zone_x + PADDING_NORMAL;
    }

    return start_x + index * (card_w + CARD_SPACING);
}
/* }}} */

/* {{{ zone_get_card_y */
int zone_get_card_y(int zone_y, int zone_h, int card_h) {
    return zone_y + (zone_h - card_h) / 2;
}
/* }}} */

/* {{{ render_zone_background
 * Draw a subtle background for a zone.
 */
static void render_zone_background(const Zone* zone, uint32_t border_color) {
    /* Very dark semi-transparent background */
    draw_rounded_rect(zone->x, zone->y, zone->w, zone->h,
                      PANEL_CORNER_RADIUS, THEME_BG_PANEL);
    draw_rounded_rect_outline(zone->x, zone->y, zone->w, zone->h,
                              PANEL_CORNER_RADIUS, border_color, 1);
}
/* }}} */

/* {{{ render_deck_pile
 * Render a deck pile (face-down cards stack).
 */
static void render_deck_pile(int x, int y, int w, int h, int count, const char* label) {
    if (count <= 0) {
        /* Empty deck slot */
        card_render_empty_slot(x, y, w, h);
    } else {
        /* Stacked cards effect */
        int stack_offset = 2;
        int stack_cards = count > 5 ? 5 : count;

        for (int i = stack_cards - 1; i >= 0; i--) {
            card_render_back(x + i * stack_offset, y - i * stack_offset, w, h);
        }
    }

    /* Count label below */
    if (label) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%s: %d", label, count);
        draw_text_ex(x + w/2, y + h + 10, buf, THEME_TEXT_DIM,
                     FONT_SIZE_SMALL, TEXT_ALIGN_CENTER, TEXT_BASELINE_TOP);
    }
}
/* }}} */

/* {{{ zone_render_hand */
void zone_render_hand(const Zone* zone, const HandZoneData* data, int card_w, int card_h) {
    if (!zone || !data) return;

    render_zone_background(zone, THEME_BORDER_DIM);

    /* Zone label */
    draw_text_ex(zone->x + PADDING_SMALL, zone->y + PADDING_SMALL,
                 "Hand", THEME_TEXT_DIM, FONT_SIZE_SMALL,
                 TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);

    int card_y = zone_get_card_y(zone->y, zone->h, card_h);

    for (int i = 0; i < data->card_count && i < MAX_HAND_CARDS; i++) {
        int card_x = zone_get_card_x(zone->x, zone->w, card_w, data->card_count, i);

        /* Copy render data and update state based on hover/selection */
        CardRenderData render = data->cards[i].render_data;

        if (i == data->selected_index) {
            render.state = CARD_STATE_SELECTED;
            /* Lift selected card */
            card_render(card_x, card_y - 15, card_w, card_h, &render);
        } else if (i == data->hover_index) {
            render.state = CARD_STATE_HOVER;
            /* Lift hovered card slightly */
            card_render(card_x, card_y - 8, card_w, card_h, &render);
        } else {
            card_render(card_x, card_y, card_w, card_h, &render);
        }
    }

    /* Card count indicator */
    char count_buf[8];
    snprintf(count_buf, sizeof(count_buf), "%d", data->card_count);
    draw_text_ex(zone->x + zone->w - PADDING_SMALL, zone->y + PADDING_SMALL,
                 count_buf, THEME_TEXT_DIM, FONT_SIZE_SMALL,
                 TEXT_ALIGN_RIGHT, TEXT_BASELINE_TOP);
}
/* }}} */

/* {{{ zone_render_trade_row */
void zone_render_trade_row(const Zone* zone, const TradeRowData* data, int card_w, int card_h) {
    if (!zone || !data) return;

    render_zone_background(zone, THEME_VALUE_TRADE);

    /* Zone label */
    draw_text_ex(zone->x + PADDING_SMALL, zone->y + PADDING_SMALL,
                 "Trade Row", THEME_VALUE_TRADE, FONT_SIZE_SMALL,
                 TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);

    int card_y = zone_get_card_y(zone->y, zone->h, card_h);

    /* Trade deck on left */
    int deck_x = zone->x + PADDING_NORMAL;
    render_deck_pile(deck_x, card_y, card_w, card_h, data->deck_count, "Deck");

    /* Trade row cards - offset to account for deck */
    int row_start_x = deck_x + card_w + CARD_SPACING * 2;
    int available_width = zone->w - (row_start_x - zone->x) - PADDING_NORMAL;

    /* Calculate card positions within available space */
    int total_cards_width = data->card_count * card_w + (data->card_count - 1) * CARD_SPACING;
    int cards_start_x = row_start_x + (available_width - total_cards_width) / 2;
    if (cards_start_x < row_start_x) cards_start_x = row_start_x;

    for (int i = 0; i < data->card_count && i < MAX_TRADE_ROW_CARDS; i++) {
        int card_x = cards_start_x + i * (card_w + CARD_SPACING);

        CardRenderData render = data->cards[i].render_data;

        if (i == data->hover_index) {
            render.state = CARD_STATE_HOVER;
            card_render(card_x, card_y - 5, card_w, card_h, &render);
        } else {
            card_render(card_x, card_y, card_w, card_h, &render);
        }
    }

    /* Explorer pile indicator on right (always available) */
    if (data->show_explorer) {
        int explorer_x = zone->x + zone->w - card_w - PADDING_NORMAL;
        CardRenderData explorer = {
            .name = "Explorer",
            .faction = "neutral",
            .card_type = "ship",
            .cost = 2,
            .trade_value = 2,
            .combat_value = 0,
            .authority_value = 0,
            .draw_value = 0,
            .defense = 0,
            .is_outpost = false,
            .state = CARD_STATE_NORMAL
        };
        card_render(explorer_x, card_y, card_w, card_h, &explorer);

        draw_text_ex(explorer_x + card_w/2, card_y + card_h + 8,
                     "∞", THEME_TEXT_DIM, FONT_SIZE_SMALL,
                     TEXT_ALIGN_CENTER, TEXT_BASELINE_TOP);
    }
}
/* }}} */

/* {{{ zone_render_bases */
void zone_render_bases(const Zone* zone, const BasesZoneData* data,
                       int card_w, int card_h, bool is_opponent) {
    if (!zone || !data) return;

    uint32_t border_color = is_opponent ? THEME_STATUS_DANGER : THEME_STATUS_SUCCESS;
    render_zone_background(zone, border_color);

    /* Zone label */
    const char* label = is_opponent ? "Enemy Bases" : "Your Bases";
    draw_text_ex(zone->x + PADDING_SMALL, zone->y + PADDING_SMALL,
                 label, border_color, FONT_SIZE_SMALL,
                 TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);

    int card_y = zone_get_card_y(zone->y, zone->h, card_h);

    /* Render bases with smaller spacing (they're stacked closer) */
    int base_spacing = card_w / 3;

    for (int i = 0; i < data->base_count && i < MAX_BASES; i++) {
        int base_x = zone->x + PADDING_NORMAL + i * base_spacing;

        CardRenderData render = data->bases[i].render_data;

        if (i == data->targeted_index) {
            render.state = CARD_STATE_TARGETED;
        } else if (i == data->hover_index) {
            render.state = CARD_STATE_HOVER;
        }

        card_render(base_x, card_y, card_w, card_h, &render);
    }

    /* Empty slot indicators */
    for (int i = data->base_count; i < MAX_BASES; i++) {
        int slot_x = zone->x + PADDING_NORMAL + i * base_spacing;
        card_render_empty_slot(slot_x, card_y, card_w, card_h);
    }
}
/* }}} */

/* {{{ zone_render_play_area */
void zone_render_play_area(const Zone* zone, const PlayAreaData* data, int card_w, int card_h) {
    if (!zone || !data) return;

    render_zone_background(zone, THEME_BORDER_NORMAL);

    /* Zone label */
    draw_text_ex(zone->x + PADDING_SMALL, zone->y + PADDING_SMALL,
                 "In Play", THEME_TEXT_DIM, FONT_SIZE_SMALL,
                 TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);

    if (data->card_count == 0) {
        /* Empty play area message */
        draw_text_ex(zone->x + zone->w/2, zone->y + zone->h/2,
                     "Play cards from your hand", THEME_TEXT_DIM,
                     FONT_SIZE_NORMAL, TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);
        return;
    }

    /* Center cards vertically */
    int card_y = zone_get_card_y(zone->y, zone->h, card_h);

    /* Calculate card positions */
    int total_width = data->card_count * card_w + (data->card_count - 1) * CARD_SPACING;
    int start_x = zone->x + (zone->w - total_width) / 2;

    if (start_x < zone->x + PADDING_NORMAL) {
        /* Too many cards - overlap them */
        int available = zone->w - PADDING_NORMAL * 2 - card_w;
        int spacing = available / (data->card_count - 1);
        if (spacing > card_w + CARD_SPACING) spacing = card_w + CARD_SPACING;

        for (int i = 0; i < data->card_count && i < MAX_PLAY_AREA_CARDS; i++) {
            int card_x = zone->x + PADDING_NORMAL + i * spacing;

            CardRenderData render = data->cards[i].render_data;
            if (i == data->hover_index) {
                render.state = CARD_STATE_HOVER;
            }
            card_render(card_x, card_y, card_w, card_h, &render);
        }
    } else {
        /* Normal spacing */
        for (int i = 0; i < data->card_count && i < MAX_PLAY_AREA_CARDS; i++) {
            int card_x = start_x + i * (card_w + CARD_SPACING);

            CardRenderData render = data->cards[i].render_data;
            if (i == data->hover_index) {
                render.state = CARD_STATE_HOVER;
            }
            card_render(card_x, card_y, card_w, card_h, &render);
        }
    }
}
/* }}} */
