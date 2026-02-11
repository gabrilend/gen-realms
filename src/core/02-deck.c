/* 02-deck.c - Deck management implementation
 *
 * Implements card zone management with support for the draw order choice
 * mechanic. Players can choose which positions to draw from, allowing
 * strategic decisions based on trade row state before revealing cards.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "02-deck.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static bool s_rng_initialized = false;

/* ========================================================================== */
/*                             Internal Helpers                               */
/* ========================================================================== */

/* {{{ ensure_capacity
 * Grows a card array if needed. Returns false on allocation failure.
 */
static bool ensure_capacity(CardInstance*** array, int* capacity, int required) {
    if (required <= *capacity) {
        return true;
    }

    int new_capacity = *capacity * 2;
    if (new_capacity < required) {
        new_capacity = required;
    }
    if (new_capacity < DECK_DEFAULT_CAPACITY) {
        new_capacity = DECK_DEFAULT_CAPACITY;
    }

    CardInstance** new_array = realloc(*array, new_capacity * sizeof(CardInstance*));
    if (!new_array) {
        return false;
    }

    *array = new_array;
    *capacity = new_capacity;
    return true;
}
/* }}} */

/* {{{ remove_from_array
 * Removes a card from an array by shifting remaining elements.
 * Returns the removed card or NULL if not found.
 */
static CardInstance* remove_from_array(CardInstance** array, int* count,
                                        CardInstance* card) {
    for (int i = 0; i < *count; i++) {
        if (array[i] == card) {
            CardInstance* removed = array[i];
            /* Shift remaining elements down */
            for (int j = i; j < *count - 1; j++) {
                array[j] = array[j + 1];
            }
            (*count)--;
            return removed;
        }
    }
    return NULL;
}
/* }}} */

/* {{{ remove_at_index
 * Removes a card at a specific index. Returns the removed card.
 */
static CardInstance* remove_at_index(CardInstance** array, int* count, int index) {
    if (index < 0 || index >= *count) {
        return NULL;
    }

    CardInstance* removed = array[index];
    /* Shift remaining elements down */
    for (int i = index; i < *count - 1; i++) {
        array[i] = array[i + 1];
    }
    (*count)--;
    return removed;
}
/* }}} */

/* {{{ array_contains
 * Returns true if the card pointer exists in the array.
 */
static bool array_contains(CardInstance** array, int count, CardInstance* card) {
    for (int i = 0; i < count; i++) {
        if (array[i] == card) {
            return true;
        }
    }
    return false;
}
/* }}} */

/* {{{ find_by_instance_id
 * Searches an array for a card with matching instance_id.
 */
static CardInstance* find_by_instance_id(CardInstance** array, int count,
                                          const char* instance_id) {
    if (!instance_id) {
        return NULL;
    }
    for (int i = 0; i < count; i++) {
        if (array[i] && array[i]->instance_id &&
            strcmp(array[i]->instance_id, instance_id) == 0) {
            return array[i];
        }
    }
    return NULL;
}
/* }}} */

/* ========================================================================== */
/*                             Deck Lifecycle                                 */
/* ========================================================================== */

/* {{{ deck_create
 * Allocates a new empty deck with default capacity for all zones.
 */
Deck* deck_create(void) {
    Deck* deck = calloc(1, sizeof(Deck));
    if (!deck) {
        return NULL;
    }

    /* Initialize all zones with default capacity */
    deck->draw_pile = calloc(DECK_DEFAULT_CAPACITY, sizeof(CardInstance*));
    deck->hand = calloc(DECK_DEFAULT_CAPACITY, sizeof(CardInstance*));
    deck->discard = calloc(DECK_DEFAULT_CAPACITY, sizeof(CardInstance*));
    deck->played = calloc(DECK_DEFAULT_CAPACITY, sizeof(CardInstance*));
    deck->bases = calloc(DECK_DEFAULT_CAPACITY, sizeof(CardInstance*));

    if (!deck->draw_pile || !deck->hand || !deck->discard ||
        !deck->played || !deck->bases) {
        deck_free(deck);
        return NULL;
    }

    deck->draw_pile_capacity = DECK_DEFAULT_CAPACITY;
    deck->hand_capacity = DECK_DEFAULT_CAPACITY;
    deck->discard_capacity = DECK_DEFAULT_CAPACITY;
    deck->played_capacity = DECK_DEFAULT_CAPACITY;
    deck->base_capacity = DECK_DEFAULT_CAPACITY;

    return deck;
}
/* }}} */

/* {{{ deck_free
 * Frees all cards in all zones, then frees the deck structure.
 */
void deck_free(Deck* deck) {
    if (!deck) {
        return;
    }

    /* Free all card instances in each zone */
    for (int i = 0; i < deck->draw_pile_count; i++) {
        card_instance_free(deck->draw_pile[i]);
    }
    for (int i = 0; i < deck->hand_count; i++) {
        card_instance_free(deck->hand[i]);
    }
    for (int i = 0; i < deck->discard_count; i++) {
        card_instance_free(deck->discard[i]);
    }
    for (int i = 0; i < deck->played_count; i++) {
        card_instance_free(deck->played[i]);
    }
    for (int i = 0; i < deck->base_count; i++) {
        card_instance_free(deck->bases[i]);
    }

    /* Free the arrays */
    free(deck->draw_pile);
    free(deck->hand);
    free(deck->discard);
    free(deck->played);
    free(deck->bases);

    free(deck);
}
/* }}} */

/* ========================================================================== */
/*                            Adding Cards                                    */
/* ========================================================================== */

/* {{{ deck_add_to_draw_pile
 * Adds a card to the bottom of the draw pile. Returns false on failure.
 */
bool deck_add_to_draw_pile(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }
    if (!ensure_capacity(&deck->draw_pile, &deck->draw_pile_capacity,
                         deck->draw_pile_count + 1)) {
        return false;
    }
    deck->draw_pile[deck->draw_pile_count++] = card;
    return true;
}
/* }}} */

/* {{{ deck_add_to_hand
 * Adds a card to the player's hand. Returns false on failure.
 */
bool deck_add_to_hand(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }
    if (!ensure_capacity(&deck->hand, &deck->hand_capacity,
                         deck->hand_count + 1)) {
        return false;
    }
    deck->hand[deck->hand_count++] = card;
    return true;
}
/* }}} */

/* {{{ deck_add_to_discard
 * Adds a card to the discard pile. Returns false on failure.
 */
bool deck_add_to_discard(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }
    if (!ensure_capacity(&deck->discard, &deck->discard_capacity,
                         deck->discard_count + 1)) {
        return false;
    }
    deck->discard[deck->discard_count++] = card;
    return true;
}
/* }}} */

/* {{{ deck_add_to_played
 * Adds a card to the played zone. Returns false on failure.
 */
bool deck_add_to_played(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }
    if (!ensure_capacity(&deck->played, &deck->played_capacity,
                         deck->played_count + 1)) {
        return false;
    }
    deck->played[deck->played_count++] = card;
    return true;
}
/* }}} */

/* {{{ deck_add_base
 * Adds a base to the persistent base zone. Returns false on failure.
 */
bool deck_add_base(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }
    if (!ensure_capacity(&deck->bases, &deck->base_capacity,
                         deck->base_count + 1)) {
        return false;
    }
    deck->bases[deck->base_count++] = card;
    return true;
}
/* }}} */

/* ========================================================================== */
/*                         Shuffle and Draw                                   */
/* ========================================================================== */

/* {{{ deck_shuffle
 * Randomizes the draw pile using Fisher-Yates shuffle.
 * Also marks all cards for art regeneration (they're entering a new shuffle).
 */
void deck_shuffle(Deck* deck) {
    if (!deck || deck->draw_pile_count < 2) {
        return;
    }

    if (!s_rng_initialized) {
        srand((unsigned int)time(NULL));
        s_rng_initialized = true;
    }

    /* Fisher-Yates shuffle */
    for (int i = deck->draw_pile_count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        CardInstance* temp = deck->draw_pile[i];
        deck->draw_pile[i] = deck->draw_pile[j];
        deck->draw_pile[j] = temp;
    }

    /* Reset regeneration flags - cards have been shuffled */
    for (int i = 0; i < deck->draw_pile_count; i++) {
        if (deck->draw_pile[i]->needs_regen) {
            /* Generate new seed for art variety */
            deck->draw_pile[i]->image_seed = (uint32_t)rand();
        }
        /* Reset draw effect spent flag for new shuffle cycle */
        deck->draw_pile[i]->draw_effect_spent = false;
    }
}
/* }}} */

/* {{{ deck_reshuffle_discard
 * Moves all cards from discard to draw pile and shuffles.
 * Called automatically when draw pile is empty and drawing is needed.
 */
void deck_reshuffle_discard(Deck* deck) {
    if (!deck) {
        return;
    }

    /* Move all discard to draw pile */
    for (int i = 0; i < deck->discard_count; i++) {
        deck_add_to_draw_pile(deck, deck->discard[i]);
    }
    deck->discard_count = 0;

    deck_shuffle(deck);
}
/* }}} */

/* {{{ deck_draw_top
 * Draws the top card from the draw pile (index 0).
 * Triggers reshuffle if draw pile is empty.
 * Returns NULL if both draw pile and discard are empty.
 */
CardInstance* deck_draw_top(Deck* deck) {
    if (!deck) {
        return NULL;
    }

    if (deck->draw_pile_count == 0) {
        if (deck->discard_count == 0) {
            return NULL;  /* No cards available to draw */
        }
        deck_reshuffle_discard(deck);
    }

    CardInstance* card = remove_at_index(deck->draw_pile, &deck->draw_pile_count, 0);
    if (card) {
        deck_add_to_hand(deck, card);
    }
    return card;
}
/* }}} */

/* {{{ deck_draw_at
 * Draws a card from a specific position in the draw pile.
 * Supports the draw order choice mechanic.
 * Returns NULL if index is invalid or draw pile is empty.
 */
CardInstance* deck_draw_at(Deck* deck, int index) {
    if (!deck) {
        return NULL;
    }

    if (deck->draw_pile_count == 0) {
        if (deck->discard_count == 0) {
            return NULL;
        }
        deck_reshuffle_discard(deck);
    }

    if (index < 0 || index >= deck->draw_pile_count) {
        return NULL;
    }

    CardInstance* card = remove_at_index(deck->draw_pile, &deck->draw_pile_count, index);
    if (card) {
        deck_add_to_hand(deck, card);
    }
    return card;
}
/* }}} */

/* {{{ deck_draw_ordered
 * Draws multiple cards in a specified order.
 * Order array contains indices into the CURRENT draw pile (adjusted as draws happen).
 * Returns true if all draws succeeded, false otherwise.
 */
bool deck_draw_ordered(Deck* deck, int* order, int count) {
    if (!deck || !order || count <= 0) {
        return false;
    }

    /* Validate we have enough cards (may need to reshuffle) */
    int available = deck->draw_pile_count + deck->discard_count;
    if (count > available) {
        return false;
    }

    /* Draw in specified order */
    for (int i = 0; i < count; i++) {
        /* Reshuffle if needed */
        if (deck->draw_pile_count == 0) {
            deck_reshuffle_discard(deck);
        }

        int index = order[i];

        /* Adjust index for cards already drawn
         * The order array refers to original positions, so we need to adjust
         * for any indices that were less than previous draws
         */
        for (int j = 0; j < i; j++) {
            if (order[j] < order[i]) {
                index--;
            }
        }

        if (index < 0 || index >= deck->draw_pile_count) {
            /* Index invalid after adjustments - draw from top as fallback */
            deck_draw_top(deck);
        } else {
            deck_draw_at(deck, index);
        }
    }

    return true;
}
/* }}} */

/* {{{ deck_draw_pile_count
 * Returns the number of cards in the draw pile.
 */
int deck_draw_pile_count(Deck* deck) {
    if (!deck) {
        return 0;
    }
    return deck->draw_pile_count;
}
/* }}} */

/* ========================================================================== */
/*                            Card Movement                                   */
/* ========================================================================== */

/* {{{ deck_play_from_hand
 * Moves a card from hand to played zone.
 * For bases, moves to the base zone instead.
 */
bool deck_play_from_hand(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }

    CardInstance* removed = remove_from_array(deck->hand, &deck->hand_count, card);
    if (!removed) {
        return false;
    }

    /* Bases go to persistent zone, others to played */
    if (card->type && card->type->kind == CARD_KIND_BASE) {
        return deck_add_base(deck, card);
    } else {
        return deck_add_to_played(deck, card);
    }
}
/* }}} */

/* {{{ deck_discard_from_hand
 * Moves a card from hand to discard pile.
 */
bool deck_discard_from_hand(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }

    CardInstance* removed = remove_from_array(deck->hand, &deck->hand_count, card);
    if (!removed) {
        return false;
    }

    return deck_add_to_discard(deck, card);
}
/* }}} */

/* {{{ deck_discard_from_played
 * Moves a card from played zone to discard pile.
 */
bool deck_discard_from_played(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }

    CardInstance* removed = remove_from_array(deck->played, &deck->played_count, card);
    if (!removed) {
        return false;
    }

    return deck_add_to_discard(deck, card);
}
/* }}} */

/* {{{ deck_remove_base
 * Removes a base from play (when destroyed). Returns the card for cleanup.
 */
CardInstance* deck_remove_base(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return NULL;
    }
    return remove_from_array(deck->bases, &deck->base_count, card);
}
/* }}} */

/* ========================================================================== */
/*                           Turn Management                                  */
/* ========================================================================== */

/* {{{ deck_end_turn
 * Moves all cards from hand and played zones to discard pile.
 * Bases remain in play.
 */
void deck_end_turn(Deck* deck) {
    if (!deck) {
        return;
    }

    /* Move all played cards to discard */
    for (int i = 0; i < deck->played_count; i++) {
        deck_add_to_discard(deck, deck->played[i]);
    }
    deck->played_count = 0;

    /* Move remaining hand cards to discard */
    for (int i = 0; i < deck->hand_count; i++) {
        deck_add_to_discard(deck, deck->hand[i]);
    }
    deck->hand_count = 0;
}
/* }}} */

/* {{{ deck_clear_hand
 * Moves all cards from hand to discard without affecting played zone.
 */
void deck_clear_hand(Deck* deck) {
    if (!deck) {
        return;
    }

    for (int i = 0; i < deck->hand_count; i++) {
        deck_add_to_discard(deck, deck->hand[i]);
    }
    deck->hand_count = 0;
}
/* }}} */

/* ========================================================================== */
/*                               Scrap                                        */
/* ========================================================================== */

/* {{{ deck_scrap_from_hand
 * Removes a card from hand entirely (scrap ability used).
 * Returns the card for the caller to free or process.
 */
CardInstance* deck_scrap_from_hand(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return NULL;
    }
    return remove_from_array(deck->hand, &deck->hand_count, card);
}
/* }}} */

/* {{{ deck_scrap_from_discard
 * Removes a card from discard pile entirely.
 * Returns the card for the caller to free or process.
 */
CardInstance* deck_scrap_from_discard(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return NULL;
    }
    return remove_from_array(deck->discard, &deck->discard_count, card);
}
/* }}} */

/* {{{ deck_scrap_from_played
 * Removes a card from played zone entirely.
 * Returns the card for the caller to free or process.
 */
CardInstance* deck_scrap_from_played(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return NULL;
    }
    return remove_from_array(deck->played, &deck->played_count, card);
}
/* }}} */

/* ========================================================================== */
/*                            Query Functions                                 */
/* ========================================================================== */

/* {{{ deck_hand_contains
 * Returns true if the card is in the hand.
 */
bool deck_hand_contains(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }
    return array_contains(deck->hand, deck->hand_count, card);
}
/* }}} */

/* {{{ deck_discard_contains
 * Returns true if the card is in the discard pile.
 */
bool deck_discard_contains(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }
    return array_contains(deck->discard, deck->discard_count, card);
}
/* }}} */

/* {{{ deck_find_in_hand
 * Finds a card in hand by instance ID. Returns NULL if not found.
 */
CardInstance* deck_find_in_hand(Deck* deck, const char* instance_id) {
    if (!deck) {
        return NULL;
    }
    return find_by_instance_id(deck->hand, deck->hand_count, instance_id);
}
/* }}} */

/* {{{ deck_find_in_discard
 * Finds a card in discard by instance ID. Returns NULL if not found.
 */
CardInstance* deck_find_in_discard(Deck* deck, const char* instance_id) {
    if (!deck) {
        return NULL;
    }
    return find_by_instance_id(deck->discard, deck->discard_count, instance_id);
}
/* }}} */

/* {{{ deck_total_card_count
 * Returns total number of cards across all zones.
 */
int deck_total_card_count(Deck* deck) {
    if (!deck) {
        return 0;
    }
    return deck->draw_pile_count + deck->hand_count + deck->discard_count +
           deck->played_count + deck->base_count;
}
/* }}} */

/* ========================================================================== */
/*                         Top Deck Manipulation                              */
/* ========================================================================== */

/* {{{ deck_put_on_top
 * Puts a card on top of the draw pile (position 0).
 * Used for effects that let you put a card from discard on top.
 * The card should already be removed from its current zone.
 */
bool deck_put_on_top(Deck* deck, CardInstance* card) {
    if (!deck || !card) {
        return false;
    }

    if (!ensure_capacity(&deck->draw_pile, &deck->draw_pile_capacity,
                         deck->draw_pile_count + 1)) {
        return false;
    }

    /* Shift all cards down by one position */
    for (int i = deck->draw_pile_count; i > 0; i--) {
        deck->draw_pile[i] = deck->draw_pile[i - 1];
    }

    deck->draw_pile[0] = card;
    deck->draw_pile_count++;
    return true;
}
/* }}} */
