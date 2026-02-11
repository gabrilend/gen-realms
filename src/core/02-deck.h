/* 02-deck.h - Deck management type definitions
 *
 * Manages the various card zones for each player: draw pile, hand, discard,
 * played cards, and persistent bases. Supports the draw order choice mechanic
 * where players select the sequence to draw cards.
 */

#ifndef SYMBELINE_DECK_H
#define SYMBELINE_DECK_H

#include "01-card.h"
#include <stdbool.h>

/* Default capacity for card arrays. Grows dynamically as needed. */
#define DECK_DEFAULT_CAPACITY 20

/* ========================================================================== */
/*                                  Structures                                */
/* ========================================================================== */

/* {{{ Deck
 * Contains all card zones for a player. Each zone is a dynamic array of
 * CardInstance pointers. The deck owns all CardInstance memory within it.
 */
typedef struct {
    /* Draw pile - face down, cards drawn from here */
    CardInstance** draw_pile;
    int draw_pile_count;
    int draw_pile_capacity;

    /* Hand - cards available to play this turn */
    CardInstance** hand;
    int hand_count;
    int hand_capacity;

    /* Discard pile - played/discarded cards, shuffled back when draw empty */
    CardInstance** discard;
    int discard_count;
    int discard_capacity;

    /* Played - cards played this turn, moved to discard at end of turn */
    CardInstance** played;
    int played_count;
    int played_capacity;

    /* Frontier bases - exposed, must all be destroyed before interior */
    CardInstance** frontier_bases;
    int frontier_base_count;
    int frontier_base_capacity;

    /* Interior bases - protected, only attackable when frontier is empty */
    CardInstance** interior_bases;
    int interior_base_count;
    int interior_base_capacity;
} Deck;
/* }}} */

/* ========================================================================== */
/*                              Function Prototypes                           */
/* ========================================================================== */

/* {{{ Deck lifecycle */
Deck* deck_create(void);
void deck_free(Deck* deck);
/* }}} */

/* {{{ Adding cards to deck */
bool deck_add_to_draw_pile(Deck* deck, CardInstance* card);
bool deck_add_to_hand(Deck* deck, CardInstance* card);
bool deck_add_to_discard(Deck* deck, CardInstance* card);
bool deck_add_to_played(Deck* deck, CardInstance* card);
bool deck_add_base_to_frontier(Deck* deck, CardInstance* card);
bool deck_add_base_to_interior(Deck* deck, CardInstance* card);
bool deck_add_base(Deck* deck, CardInstance* card);  /* Uses card->placement */
/* }}} */

/* {{{ Shuffle and draw operations */
void deck_shuffle(Deck* deck);
void deck_reshuffle_discard(Deck* deck);
CardInstance* deck_draw_top(Deck* deck);
CardInstance* deck_draw_at(Deck* deck, int index);
bool deck_draw_ordered(Deck* deck, int* order, int count);
int deck_draw_pile_count(Deck* deck);
/* }}} */

/* {{{ Card movement */
bool deck_play_from_hand(Deck* deck, CardInstance* card);
bool deck_play_base_to_frontier(Deck* deck, CardInstance* card);
bool deck_play_base_to_interior(Deck* deck, CardInstance* card);
bool deck_discard_from_hand(Deck* deck, CardInstance* card);
bool deck_discard_from_played(Deck* deck, CardInstance* card);
CardInstance* deck_remove_base(Deck* deck, CardInstance* card);
CardInstance* deck_remove_from_frontier(Deck* deck, CardInstance* card);
CardInstance* deck_remove_from_interior(Deck* deck, CardInstance* card);
/* }}} */

/* {{{ Turn management */
void deck_end_turn(Deck* deck);
void deck_clear_hand(Deck* deck);
/* }}} */

/* {{{ Scrap (remove from game) */
CardInstance* deck_scrap_from_hand(Deck* deck, CardInstance* card);
CardInstance* deck_scrap_from_discard(Deck* deck, CardInstance* card);
CardInstance* deck_scrap_from_played(Deck* deck, CardInstance* card);
/* }}} */

/* {{{ Query functions */
bool deck_hand_contains(Deck* deck, CardInstance* card);
bool deck_discard_contains(Deck* deck, CardInstance* card);
CardInstance* deck_find_in_hand(Deck* deck, const char* instance_id);
CardInstance* deck_find_in_discard(Deck* deck, const char* instance_id);
int deck_total_card_count(Deck* deck);
int deck_frontier_count(Deck* deck);
int deck_interior_count(Deck* deck);
int deck_total_base_count(Deck* deck);
bool deck_has_frontier_bases(Deck* deck);
/* }}} */

/* {{{ Top deck manipulation */
bool deck_put_on_top(Deck* deck, CardInstance* card);
/* }}} */

#endif /* SYMBELINE_DECK_H */
