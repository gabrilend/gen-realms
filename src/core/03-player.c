/* 03-player.c - Player state management implementation
 *
 * Implements player resource tracking and the d10/d4 deck flow system.
 * The d10 tracks buy/scrap momentum: buying cards increments, scrapping
 * decrements. When d10 overflows (9->0), d4 increases giving bonus draws.
 * When d10 underflows (0->9), d4 decreases reducing hand size.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "03-player.h"
#include <stdlib.h>
#include <string.h>

/* ========================================================================== */
/*                            Player Lifecycle                                */
/* ========================================================================== */

/* {{{ player_create
 * Allocates a new player with default starting values.
 * The player owns their deck which is created here.
 */
Player* player_create(const char* name, int id) {
    Player* player = calloc(1, sizeof(Player));
    if (!player) {
        return NULL;
    }

    player->id = id;
    player->name = name ? strdup(name) : strdup("Unknown");
    player->connection_id = -1;  /* Not yet connected */

    /* Starting values */
    player->authority = PLAYER_STARTING_AUTHORITY;
    player->trade = 0;
    player->combat = 0;
    player->d10 = PLAYER_STARTING_D10;
    player->d4 = PLAYER_STARTING_D4;

    /* No factions played initially */
    for (int i = 0; i < FACTION_COUNT; i++) {
        player->factions_played[i] = false;
    }

    /* Create empty deck */
    player->deck = deck_create();
    if (!player->deck) {
        free(player->name);
        free(player);
        return NULL;
    }

    return player;
}
/* }}} */

/* {{{ player_free
 * Frees the player and all associated resources including deck.
 */
void player_free(Player* player) {
    if (!player) {
        return;
    }

    free(player->name);
    deck_free(player->deck);
    free(player);
}
/* }}} */

/* ========================================================================== */
/*                           Resource Management                              */
/* ========================================================================== */

/* {{{ player_add_trade
 * Adds trade (coins) for this turn. Used when playing cards with trade effects.
 */
void player_add_trade(Player* player, int amount) {
    if (!player || amount < 0) {
        return;
    }
    player->trade += amount;
}
/* }}} */

/* {{{ player_add_combat
 * Adds combat (attack power) for this turn.
 */
void player_add_combat(Player* player, int amount) {
    if (!player || amount < 0) {
        return;
    }
    player->combat += amount;
}
/* }}} */

/* {{{ player_add_authority
 * Adds authority (health). Can exceed starting value.
 */
void player_add_authority(Player* player, int amount) {
    if (!player || amount < 0) {
        return;
    }
    player->authority += amount;
}
/* }}} */

/* {{{ player_take_damage
 * Reduces authority by damage amount. Cannot go below 0.
 */
void player_take_damage(Player* player, int amount) {
    if (!player || amount < 0) {
        return;
    }
    player->authority -= amount;
    if (player->authority < 0) {
        player->authority = 0;
    }
}
/* }}} */

/* {{{ player_spend_trade
 * Attempts to spend trade on a purchase. Returns false if insufficient.
 */
bool player_spend_trade(Player* player, int cost) {
    if (!player || cost < 0) {
        return false;
    }
    if (player->trade < cost) {
        return false;
    }
    player->trade -= cost;
    return true;
}
/* }}} */

/* {{{ player_spend_combat
 * Attempts to spend combat on an attack. Returns false if insufficient.
 */
bool player_spend_combat(Player* player, int amount) {
    if (!player || amount < 0) {
        return false;
    }
    if (player->combat < amount) {
        return false;
    }
    player->combat -= amount;
    return true;
}
/* }}} */

/* ========================================================================== */
/*                           Deck Flow Tracker                                */
/* ========================================================================== */

/* {{{ player_d10_increment
 * Increments d10 (called when buying a card).
 * On overflow (9->0), d4 increases giving bonus hand size.
 */
void player_d10_increment(Player* player) {
    if (!player) {
        return;
    }

    player->d10++;
    if (player->d10 > 9) {
        player->d10 = 0;
        player->d4++;  /* Overflow: gain bonus draw */
    }
}
/* }}} */

/* {{{ player_d10_decrement
 * Decrements d10 (called when scrapping a card).
 * On underflow (0->9), d4 decreases reducing hand size.
 */
void player_d10_decrement(Player* player) {
    if (!player) {
        return;
    }

    player->d10--;
    if (player->d10 < 0) {
        player->d10 = 9;
        player->d4--;  /* Underflow: lose a draw */
    }
}
/* }}} */

/* {{{ player_get_hand_size
 * Returns the number of cards to draw each turn.
 * Base hand size (5) + d4 bonus, with minimum of 1.
 */
int player_get_hand_size(Player* player) {
    if (!player) {
        return PLAYER_MIN_HAND_SIZE;
    }

    int size = PLAYER_BASE_HAND_SIZE + player->d4;
    if (size < PLAYER_MIN_HAND_SIZE) {
        size = PLAYER_MIN_HAND_SIZE;
    }
    return size;
}
/* }}} */

/* ========================================================================== */
/*                            Turn Management                                 */
/* ========================================================================== */

/* {{{ player_reset_turn
 * Resets per-turn resources at the start of a new turn.
 * Authority and d10/d4 persist across turns.
 */
void player_reset_turn(Player* player) {
    if (!player) {
        return;
    }

    player->trade = 0;
    player->combat = 0;

    /* Reset faction tracking for ally abilities */
    for (int i = 0; i < FACTION_COUNT; i++) {
        player->factions_played[i] = false;
    }
}
/* }}} */

/* {{{ player_mark_faction_played
 * Records that a card of this faction was played this turn.
 * Used for ally ability triggering.
 */
void player_mark_faction_played(Player* player, Faction faction) {
    if (!player || faction < 0 || faction >= FACTION_COUNT) {
        return;
    }
    player->factions_played[faction] = true;
}
/* }}} */

/* {{{ player_has_faction_ally
 * Returns true if a card of this faction was already played this turn.
 * Used to determine if ally abilities should trigger.
 */
bool player_has_faction_ally(Player* player, Faction faction) {
    if (!player || faction < 0 || faction >= FACTION_COUNT) {
        return false;
    }
    return player->factions_played[faction];
}
/* }}} */

/* ========================================================================== */
/*                             State Queries                                  */
/* ========================================================================== */

/* {{{ player_is_alive
 * Returns true if player has authority remaining.
 */
bool player_is_alive(Player* player) {
    if (!player) {
        return false;
    }
    return player->authority > 0;
}
/* }}} */

/* {{{ player_get_authority
 * Returns current authority (health).
 */
int player_get_authority(Player* player) {
    if (!player) {
        return 0;
    }
    return player->authority;
}
/* }}} */

/* {{{ player_get_trade
 * Returns current trade available this turn.
 */
int player_get_trade(Player* player) {
    if (!player) {
        return 0;
    }
    return player->trade;
}
/* }}} */

/* {{{ player_get_combat
 * Returns current combat available this turn.
 */
int player_get_combat(Player* player) {
    if (!player) {
        return 0;
    }
    return player->combat;
}
/* }}} */

/* ========================================================================== */
/*                       Deck Operations (Convenience)                        */
/* ========================================================================== */

/* {{{ player_draw_cards
 * Draws specified number of cards from deck to hand.
 * Triggers reshuffle if needed.
 */
void player_draw_cards(Player* player, int count) {
    if (!player || !player->deck || count <= 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        CardInstance* card = deck_draw_top(player->deck);
        if (!card) {
            break;  /* No more cards available */
        }
    }
}
/* }}} */

/* {{{ player_draw_starting_hand
 * Draws the player's starting hand based on current hand size.
 * Should be called at the start of each turn.
 */
void player_draw_starting_hand(Player* player) {
    if (!player) {
        return;
    }

    int hand_size = player_get_hand_size(player);
    player_draw_cards(player, hand_size);
}
/* }}} */

/* {{{ player_end_turn
 * Ends the player's turn: moves played/hand cards to discard.
 * Does not reset resources - that happens at start of next turn.
 */
void player_end_turn(Player* player) {
    if (!player || !player->deck) {
        return;
    }

    deck_end_turn(player->deck);
}
/* }}} */
