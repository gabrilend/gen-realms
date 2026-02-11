/* 03-player.h - Player state management type definitions
 *
 * Tracks all per-player state including health (authority), per-turn resources
 * (trade, combat), the deck flow tracker (d10/d4), and faction bonuses.
 * Each player owns a Deck and has network identification for multiplayer.
 */

#ifndef SYMBELINE_PLAYER_H
#define SYMBELINE_PLAYER_H

#include "02-deck.h"
#include <stdbool.h>

/* Default starting values */
#define PLAYER_STARTING_AUTHORITY 50
#define PLAYER_STARTING_D10 5
#define PLAYER_STARTING_D4 0
#define PLAYER_BASE_HAND_SIZE 5
#define PLAYER_MIN_HAND_SIZE 1

/* ========================================================================== */
/*                                  Structures                                */
/* ========================================================================== */

/* {{{ Player
 * Complete player state for a game session. Tracks resources, deck flow,
 * faction triggers, and network identity.
 */
typedef struct {
    /* Identity */
    int id;                     /* Unique player ID (1, 2, etc.) */
    char* name;                 /* Display name */
    int connection_id;          /* Network connection identifier */

    /* Health */
    int authority;              /* Current health, lose at 0 */

    /* Per-turn resources (reset each turn) */
    int trade;                  /* Coins available for buying cards */
    int combat;                 /* Attack power for damaging opponent */

    /* Deck flow tracker (d10/d4 system) */
    int d10;                    /* 0-9, tracks buy/scrap momentum */
    int d4;                     /* Bonus cards drawn per turn */

    /* Faction tracking for ally abilities */
    bool factions_played[FACTION_COUNT];

    /* Card state */
    Deck* deck;                 /* Player's personal deck zones */
} Player;
/* }}} */

/* ========================================================================== */
/*                              Function Prototypes                           */
/* ========================================================================== */

/* {{{ Player lifecycle */
Player* player_create(const char* name, int id);
void player_free(Player* player);
/* }}} */

/* {{{ Resource management */
void player_add_trade(Player* player, int amount);
void player_add_combat(Player* player, int amount);
void player_add_authority(Player* player, int amount);
void player_take_damage(Player* player, int amount);
bool player_spend_trade(Player* player, int cost);
bool player_spend_combat(Player* player, int amount);
/* }}} */

/* {{{ Deck flow tracker */
void player_d10_increment(Player* player);
void player_d10_decrement(Player* player);
int player_get_hand_size(Player* player);
/* }}} */

/* {{{ Turn management */
void player_reset_turn(Player* player);
void player_mark_faction_played(Player* player, Faction faction);
bool player_has_faction_ally(Player* player, Faction faction);
/* }}} */

/* {{{ State queries */
bool player_is_alive(Player* player);
int player_get_authority(Player* player);
int player_get_trade(Player* player);
int player_get_combat(Player* player);
/* }}} */

/* {{{ Deck operations (convenience wrappers) */
void player_draw_cards(Player* player, int count);
void player_draw_starting_hand(Player* player);
void player_end_turn(Player* player);
/* }}} */

#endif /* SYMBELINE_PLAYER_H */
