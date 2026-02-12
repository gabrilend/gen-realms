/*
 * card-renderer.h - Card Rendering for WASM Client
 *
 * Renders individual cards with faction colors, effect badges,
 * upgrade indicators, and selection states.
 */

#ifndef WASM_CARD_RENDERER_H
#define WASM_CARD_RENDERER_H

#include <stdbool.h>
#include <stdint.h>

/* {{{ CardRenderState
 * Visual state of a card.
 */
typedef enum {
    CARD_STATE_NORMAL,
    CARD_STATE_HOVER,
    CARD_STATE_SELECTED,
    CARD_STATE_DISABLED,
    CARD_STATE_ALLY_ACTIVE,    /* Ally ability triggered */
    CARD_STATE_EMPOWERED,      /* Has upgrades */
    CARD_STATE_TARGETED        /* Being targeted for attack */
} CardRenderState;
/* }}} */

/* {{{ CardRenderData
 * Data needed to render a card.
 */
typedef struct {
    const char* name;
    const char* faction;       /* "merchant", "wilds", etc. */
    const char* card_type;     /* "ship", "base", "unit" */
    int cost;

    /* Effects to show as badges */
    int trade_value;
    int combat_value;
    int authority_value;
    int draw_value;

    /* For bases */
    int defense;
    bool is_outpost;

    /* Upgrades */
    int upgrade_attack;
    int upgrade_trade;
    int upgrade_authority;

    /* State */
    CardRenderState state;
} CardRenderData;
/* }}} */

/* {{{ card_render
 * Render a card at the specified position.
 * @param x, y - Top-left corner position
 * @param w, h - Card dimensions
 * @param data - Card data to render
 */
void card_render(int x, int y, int w, int h, const CardRenderData* data);
/* }}} */

/* {{{ card_render_back
 * Render a face-down card (card back).
 * @param x, y - Top-left corner position
 * @param w, h - Card dimensions
 */
void card_render_back(int x, int y, int w, int h);
/* }}} */

/* {{{ card_render_empty_slot
 * Render an empty card slot (dashed border).
 * @param x, y - Top-left corner position
 * @param w, h - Card dimensions
 */
void card_render_empty_slot(int x, int y, int w, int h);
/* }}} */

/* {{{ card_render_placeholder
 * Render a card placeholder with cost indicator.
 * @param x, y - Top-left corner position
 * @param w, h - Card dimensions
 * @param cost - Cost to display, or -1 for none
 */
void card_render_placeholder(int x, int y, int w, int h, int cost);
/* }}} */

#endif /* WASM_CARD_RENDERER_H */
