/* 04-trade-row.h - Trade row type definitions
 *
 * The trade row is a marketplace of 5 cards available for purchase.
 * Cards are bought with trade currency and added to the player's discard.
 * An Explorer card is always available as a fallback purchase.
 *
 * The DM hook allows Phase 5's LLM system to influence card selection,
 * enabling narrative-driven card placement with singleton encouragement.
 */

#ifndef SYMBELINE_TRADE_ROW_H
#define SYMBELINE_TRADE_ROW_H

#include "01-card.h"
#include "03-player.h"
#include <stdbool.h>

/* Number of cards visible in the trade row */
#define TRADE_ROW_SLOTS 5

/* Explorer card cost (always available) */
#define EXPLORER_COST 2

/* Forward declaration for DM hook */
typedef struct TradeRow TradeRow;

/* ========================================================================== */
/*                              DM Hook Types                                 */
/* ========================================================================== */

/* {{{ DMSelectFunc
 * Function pointer for LLM DM override of card selection.
 * Returns a CardType to place in the trade row, or NULL for random.
 * The DM can use this to create narrative-appropriate trade rows.
 */
typedef CardType* (*DMSelectFunc)(TradeRow* row, void* context);
/* }}} */

/* ========================================================================== */
/*                                Structures                                  */
/* ========================================================================== */

/* {{{ TradeRow
 * The marketplace where players buy new cards. Contains 5 visible slots
 * plus a deck of remaining cards. Explorer is always available separately.
 */
struct TradeRow {
    /* Visible slots (NULL = empty, waiting to be filled) */
    CardInstance* slots[TRADE_ROW_SLOTS];

    /* Trade deck - remaining cards to draw from */
    CardType** trade_deck;
    int trade_deck_count;
    int trade_deck_capacity;

    /* Explorer - always available card type */
    CardType* explorer_type;

    /* DM hook for LLM-influenced selection (Phase 5) */
    DMSelectFunc dm_select;
    void* dm_context;

    /* Statistics for singleton encouragement */
    int* card_buy_counts;    /* How many times each card type was bought */
    int card_type_count;     /* Size of buy counts array */
};
/* }}} */

/* ========================================================================== */
/*                            Function Prototypes                             */
/* ========================================================================== */

/* {{{ Trade row lifecycle */
TradeRow* trade_row_create(CardType** all_cards, int count, CardType* explorer);
void trade_row_free(TradeRow* row);
/* }}} */

/* {{{ Slot management */
void trade_row_shuffle_deck(TradeRow* row);
void trade_row_fill_slots(TradeRow* row);
CardType* trade_row_select_next(TradeRow* row);
int trade_row_empty_slot_count(TradeRow* row);
/* }}} */

/* {{{ Purchasing */
bool trade_row_can_buy(TradeRow* row, int slot, Player* player);
bool trade_row_can_buy_explorer(TradeRow* row, Player* player);
CardInstance* trade_row_buy(TradeRow* row, int slot, Player* player);
CardInstance* trade_row_buy_explorer(TradeRow* row, Player* player);
int trade_row_get_cost(TradeRow* row, int slot);
/* }}} */

/* {{{ Scrap from trade row */
CardInstance* trade_row_scrap(TradeRow* row, int slot);
/* }}} */

/* {{{ DM hook */
void trade_row_set_dm(TradeRow* row, DMSelectFunc fn, void* context);
void trade_row_clear_dm(TradeRow* row);
/* }}} */

/* {{{ Query functions */
CardInstance* trade_row_get_slot(TradeRow* row, int slot);
int trade_row_deck_remaining(TradeRow* row);
/* }}} */

#endif /* SYMBELINE_TRADE_ROW_H */
