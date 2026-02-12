/*
 * theme.c - Theme and Layout Implementation for WASM Client
 *
 * Implements layout calculation and color lookup functions.
 */

#include "theme.h"
#include <string.h>

/* {{{ theme_calculate_layout */
void theme_calculate_layout(Layout* layout, int canvas_w, int canvas_h) {
    if (!layout) return;

    int w = canvas_w;
    int h = canvas_h;

    /* Determine card size based on screen width */
    if (w < 1000) {
        layout->card_w = CARD_WIDTH_SMALL;
        layout->card_h = CARD_HEIGHT_SMALL;
    } else {
        layout->card_w = CARD_WIDTH;
        layout->card_h = CARD_HEIGHT;
    }

    /* Narrative panel width (right side) */
    int narrative_w = w * NARRATIVE_WIDTH_PCT / 100;
    if (narrative_w < NARRATIVE_MIN_WIDTH) narrative_w = NARRATIVE_MIN_WIDTH;
    if (narrative_w > NARRATIVE_MAX_WIDTH) narrative_w = NARRATIVE_MAX_WIDTH;

    /* Main game area (excluding narrative) */
    int game_w = w - narrative_w - PADDING_NORMAL * 2;
    int game_h = h - STATUS_BAR_HEIGHT - PADDING_NORMAL * 2;

    /* Hand area height (bottom) */
    int hand_h = layout->card_h + PADDING_NORMAL * 2;

    /* Bases area width (sides) */
    int bases_w = layout->card_w * 2 + CARD_SPACING + PADDING_NORMAL;

    /* Status bar */
    layout->status.x = PADDING_NORMAL;
    layout->status.y = PADDING_NORMAL;
    layout->status.w = w - PADDING_NORMAL * 2;
    layout->status.h = STATUS_BAR_HEIGHT;

    /* Trade row (top center) */
    layout->trade_row.x = bases_w + PADDING_NORMAL;
    layout->trade_row.y = STATUS_BAR_HEIGHT + PADDING_NORMAL * 2;
    layout->trade_row.w = game_w - bases_w * 2;
    layout->trade_row.h = layout->card_h + PADDING_NORMAL * 2;

    /* Player's hand (bottom center) */
    layout->hand.x = bases_w + PADDING_NORMAL;
    layout->hand.y = h - hand_h - PADDING_NORMAL;
    layout->hand.w = game_w - bases_w * 2;
    layout->hand.h = hand_h;

    /* Player's bases (bottom left) */
    layout->player_bases.x = PADDING_NORMAL;
    layout->player_bases.y = h - hand_h - PADDING_NORMAL;
    layout->player_bases.w = bases_w;
    layout->player_bases.h = hand_h;

    /* Opponent's bases (top left) */
    layout->opp_bases.x = PADDING_NORMAL;
    layout->opp_bases.y = STATUS_BAR_HEIGHT + PADDING_NORMAL * 2;
    layout->opp_bases.w = bases_w;
    layout->opp_bases.h = layout->card_h + PADDING_NORMAL * 2;

    /* Play area (center) */
    int play_top = layout->trade_row.y + layout->trade_row.h + PADDING_NORMAL;
    int play_bottom = layout->hand.y - PADDING_NORMAL;
    layout->play_area.x = PADDING_NORMAL;
    layout->play_area.y = play_top;
    layout->play_area.w = game_w;
    layout->play_area.h = play_bottom - play_top;

    /* Narrative panel (right side) */
    layout->narrative.x = w - narrative_w - PADDING_NORMAL;
    layout->narrative.y = STATUS_BAR_HEIGHT + PADDING_NORMAL * 2;
    layout->narrative.w = narrative_w;
    layout->narrative.h = h - STATUS_BAR_HEIGHT - PADDING_NORMAL * 3;
}
/* }}} */

/* {{{ theme_get_faction_color */
uint32_t theme_get_faction_color(const char* faction) {
    if (!faction) return THEME_FACTION_NEUTRAL;

    if (strcmp(faction, "merchant") == 0) return THEME_FACTION_MERCHANT;
    if (strcmp(faction, "wilds") == 0) return THEME_FACTION_WILDS;
    if (strcmp(faction, "kingdom") == 0) return THEME_FACTION_KINGDOM;
    if (strcmp(faction, "artificer") == 0) return THEME_FACTION_ARTIFICER;

    return THEME_FACTION_NEUTRAL;
}
/* }}} */

/* {{{ theme_get_faction_color_dark */
uint32_t theme_get_faction_color_dark(const char* faction) {
    if (!faction) return THEME_FACTION_NEUTRAL_DARK;

    if (strcmp(faction, "merchant") == 0) return THEME_FACTION_MERCHANT_DARK;
    if (strcmp(faction, "wilds") == 0) return THEME_FACTION_WILDS_DARK;
    if (strcmp(faction, "kingdom") == 0) return THEME_FACTION_KINGDOM_DARK;
    if (strcmp(faction, "artificer") == 0) return THEME_FACTION_ARTIFICER_DARK;

    return THEME_FACTION_NEUTRAL_DARK;
}
/* }}} */

/* {{{ theme_get_value_color */
uint32_t theme_get_value_color(const char* value_type) {
    if (!value_type) return THEME_TEXT_PRIMARY;

    if (strcmp(value_type, "authority") == 0) return THEME_VALUE_AUTHORITY;
    if (strcmp(value_type, "combat") == 0) return THEME_VALUE_COMBAT;
    if (strcmp(value_type, "trade") == 0) return THEME_VALUE_TRADE;
    if (strcmp(value_type, "draw") == 0) return THEME_VALUE_DRAW;

    return THEME_TEXT_PRIMARY;
}
/* }}} */
