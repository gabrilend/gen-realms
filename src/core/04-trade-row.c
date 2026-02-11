/* 04-trade-row.c - Trade row implementation
 *
 * Manages the 5-slot marketplace where players purchase cards.
 * Supports DM override for narrative-driven card selection.
 * Explorer cards are always available as a fallback purchase.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "04-trade-row.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static bool s_rng_initialized = false;

/* ========================================================================== */
/*                           Internal Helpers                                 */
/* ========================================================================== */

/* {{{ init_rng */
static void init_rng(void) {
    if (!s_rng_initialized) {
        srand((unsigned int)time(NULL));
        s_rng_initialized = true;
    }
}
/* }}} */

/* ========================================================================== */
/*                          Trade Row Lifecycle                               */
/* ========================================================================== */

/* {{{ trade_row_create
 * Creates a new trade row with the given card types as the trade deck.
 * Does NOT include starting cards - only cards available for purchase.
 * The explorer type is stored separately for infinite availability.
 */
TradeRow* trade_row_create(CardType** all_cards, int count, CardType* explorer) {
    if (!all_cards || count <= 0) {
        return NULL;
    }

    TradeRow* row = calloc(1, sizeof(TradeRow));
    if (!row) {
        return NULL;
    }

    /* Initialize slots to empty */
    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        row->slots[i] = NULL;
    }

    /* Copy card types to trade deck */
    row->trade_deck_capacity = count;
    row->trade_deck = malloc(count * sizeof(CardType*));
    if (!row->trade_deck) {
        free(row);
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        row->trade_deck[i] = all_cards[i];
    }
    row->trade_deck_count = count;

    /* Store explorer type */
    row->explorer_type = explorer;

    /* No DM hook initially */
    row->dm_select = NULL;
    row->dm_context = NULL;

    /* Initialize buy count tracking */
    row->card_type_count = count;
    row->card_buy_counts = calloc(count, sizeof(int));

    /* Shuffle and fill initial slots */
    trade_row_shuffle_deck(row);
    trade_row_fill_slots(row);

    return row;
}
/* }}} */

/* {{{ trade_row_free
 * Frees the trade row and all card instances in slots.
 * Does NOT free the CardType pointers (they're owned by card database).
 */
void trade_row_free(TradeRow* row) {
    if (!row) {
        return;
    }

    /* Free card instances in slots */
    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        if (row->slots[i]) {
            card_instance_free(row->slots[i]);
        }
    }

    free(row->trade_deck);
    free(row->card_buy_counts);
    free(row);
}
/* }}} */

/* ========================================================================== */
/*                            Slot Management                                 */
/* ========================================================================== */

/* {{{ trade_row_shuffle_deck
 * Randomizes the order of the trade deck using Fisher-Yates shuffle.
 */
void trade_row_shuffle_deck(TradeRow* row) {
    if (!row || row->trade_deck_count < 2) {
        return;
    }

    init_rng();

    for (int i = row->trade_deck_count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        CardType* temp = row->trade_deck[i];
        row->trade_deck[i] = row->trade_deck[j];
        row->trade_deck[j] = temp;
    }
}
/* }}} */

/* {{{ trade_row_select_next
 * Selects the next card type for the trade row.
 * Uses DM hook if set, otherwise pops from trade deck.
 */
CardType* trade_row_select_next(TradeRow* row) {
    if (!row) {
        return NULL;
    }

    /* Try DM hook first */
    if (row->dm_select) {
        CardType* dm_choice = row->dm_select(row, row->dm_context);
        if (dm_choice) {
            /* Remove from trade deck if present */
            for (int i = 0; i < row->trade_deck_count; i++) {
                if (row->trade_deck[i] == dm_choice) {
                    /* Shift remaining cards down */
                    for (int j = i; j < row->trade_deck_count - 1; j++) {
                        row->trade_deck[j] = row->trade_deck[j + 1];
                    }
                    row->trade_deck_count--;
                    return dm_choice;
                }
            }
            /* DM chose a card not in deck - fall through to random */
        }
    }

    /* Default: pop from trade deck */
    if (row->trade_deck_count <= 0) {
        return NULL;  /* Deck exhausted */
    }

    CardType* selected = row->trade_deck[row->trade_deck_count - 1];
    row->trade_deck_count--;
    return selected;
}
/* }}} */

/* {{{ trade_row_fill_slots
 * Fills any empty slots with new cards from the trade deck.
 */
void trade_row_fill_slots(TradeRow* row) {
    if (!row) {
        return;
    }

    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        if (row->slots[i] == NULL) {
            CardType* type = trade_row_select_next(row);
            if (type) {
                row->slots[i] = card_instance_create(type);
            }
            /* If deck exhausted, slot stays NULL */
        }
    }
}
/* }}} */

/* {{{ trade_row_empty_slot_count
 * Returns the number of empty slots in the trade row.
 */
int trade_row_empty_slot_count(TradeRow* row) {
    if (!row) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        if (row->slots[i] == NULL) {
            count++;
        }
    }
    return count;
}
/* }}} */

/* ========================================================================== */
/*                              Purchasing                                    */
/* ========================================================================== */

/* {{{ trade_row_get_cost
 * Returns the cost of the card in the given slot, or -1 if invalid.
 */
int trade_row_get_cost(TradeRow* row, int slot) {
    if (!row || slot < 0 || slot >= TRADE_ROW_SLOTS) {
        return -1;
    }

    CardInstance* card = row->slots[slot];
    if (!card || !card->type) {
        return -1;
    }

    return card->type->cost;
}
/* }}} */

/* {{{ trade_row_can_buy
 * Returns true if the player can afford the card in the slot.
 */
bool trade_row_can_buy(TradeRow* row, int slot, Player* player) {
    if (!row || !player || slot < 0 || slot >= TRADE_ROW_SLOTS) {
        return false;
    }

    CardInstance* card = row->slots[slot];
    if (!card || !card->type) {
        return false;
    }

    return player->trade >= card->type->cost;
}
/* }}} */

/* {{{ trade_row_can_buy_explorer
 * Returns true if the player can afford an explorer.
 */
bool trade_row_can_buy_explorer(TradeRow* row, Player* player) {
    if (!row || !player || !row->explorer_type) {
        return false;
    }
    return player->trade >= EXPLORER_COST;
}
/* }}} */

/* {{{ trade_row_buy
 * Purchases a card from the trade row.
 * Deducts trade, adds card to player's discard, refills slot.
 * Returns the purchased card instance (now owned by player's deck).
 */
CardInstance* trade_row_buy(TradeRow* row, int slot, Player* player) {
    if (!trade_row_can_buy(row, slot, player)) {
        return NULL;
    }

    CardInstance* card = row->slots[slot];
    int cost = card->type->cost;

    /* Deduct trade */
    player->trade -= cost;

    /* Increment d10 (buy momentum) */
    player_d10_increment(player);

    /* Add to player's discard pile */
    deck_add_to_discard(player->deck, card);

    /* Clear slot and refill */
    row->slots[slot] = NULL;
    trade_row_fill_slots(row);

    return card;
}
/* }}} */

/* {{{ trade_row_buy_explorer
 * Purchases an explorer card (always available).
 * Creates a new instance since explorers are infinite.
 */
CardInstance* trade_row_buy_explorer(TradeRow* row, Player* player) {
    if (!trade_row_can_buy_explorer(row, player)) {
        return NULL;
    }

    /* Deduct trade */
    player->trade -= EXPLORER_COST;

    /* Increment d10 (buy momentum) */
    player_d10_increment(player);

    /* Create new explorer instance */
    CardInstance* explorer = card_instance_create(row->explorer_type);
    if (!explorer) {
        return NULL;
    }

    /* Add to player's discard pile */
    deck_add_to_discard(player->deck, explorer);

    return explorer;
}
/* }}} */

/* ========================================================================== */
/*                         Scrap from Trade Row                               */
/* ========================================================================== */

/* {{{ trade_row_scrap
 * Removes a card from the trade row (some effects allow this).
 * Returns the card instance for caller to handle/free.
 * Refills the slot from the trade deck.
 */
CardInstance* trade_row_scrap(TradeRow* row, int slot) {
    if (!row || slot < 0 || slot >= TRADE_ROW_SLOTS) {
        return NULL;
    }

    CardInstance* card = row->slots[slot];
    if (!card) {
        return NULL;
    }

    /* Remove from slot */
    row->slots[slot] = NULL;

    /* Refill */
    trade_row_fill_slots(row);

    return card;
}
/* }}} */

/* ========================================================================== */
/*                               DM Hook                                      */
/* ========================================================================== */

/* {{{ trade_row_set_dm
 * Sets the DM hook function for LLM-influenced card selection.
 * The context pointer is passed to the function on each call.
 */
void trade_row_set_dm(TradeRow* row, DMSelectFunc fn, void* context) {
    if (!row) {
        return;
    }
    row->dm_select = fn;
    row->dm_context = context;
}
/* }}} */

/* {{{ trade_row_clear_dm
 * Clears the DM hook, reverting to random selection.
 */
void trade_row_clear_dm(TradeRow* row) {
    if (!row) {
        return;
    }
    row->dm_select = NULL;
    row->dm_context = NULL;
}
/* }}} */

/* ========================================================================== */
/*                            Query Functions                                 */
/* ========================================================================== */

/* {{{ trade_row_get_slot
 * Returns the card instance in the given slot, or NULL.
 */
CardInstance* trade_row_get_slot(TradeRow* row, int slot) {
    if (!row || slot < 0 || slot >= TRADE_ROW_SLOTS) {
        return NULL;
    }
    return row->slots[slot];
}
/* }}} */

/* {{{ trade_row_deck_remaining
 * Returns the number of cards remaining in the trade deck.
 */
int trade_row_deck_remaining(TradeRow* row) {
    if (!row) {
        return 0;
    }
    return row->trade_deck_count;
}
/* }}} */
