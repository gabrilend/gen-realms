/* 05-game.h - Game state and turn loop type definitions
 *
 * Central game state management including turn phases, player switching,
 * and game end detection. The Game struct is the root object containing
 * all game data and is serialized for network transmission.
 */

#ifndef SYMBELINE_GAME_H
#define SYMBELINE_GAME_H

#include "01-card.h"
#include "02-deck.h"
#include "03-player.h"
#include "04-trade-row.h"
#include <stdbool.h>

/* Maximum number of players */
#define MAX_PLAYERS 4

/* Default starting deck composition */
#define STARTING_SCOUTS 8
#define STARTING_VIPERS 2

/* ========================================================================== */
/*                               Enumerations                                 */
/* ========================================================================== */

/* {{{ GamePhase
 * The current phase of the active player's turn.
 */
typedef enum {
    PHASE_NOT_STARTED,     /* Game created but not yet started */
    PHASE_DRAW_ORDER,      /* Waiting for draw order selection */
    PHASE_MAIN,            /* Player actions (play, buy, attack) */
    PHASE_END,             /* Cleanup phase */
    PHASE_GAME_OVER        /* Game has ended */
} GamePhase;
/* }}} */

/* {{{ ActionType
 * Types of actions a player can take during main phase.
 */
typedef enum {
    ACTION_PLAY_CARD,       /* Play a card from hand */
    ACTION_BUY_CARD,        /* Buy from trade row */
    ACTION_BUY_EXPLORER,    /* Buy an explorer */
    ACTION_ATTACK_PLAYER,   /* Attack opponent's authority */
    ACTION_ATTACK_BASE,     /* Attack opponent's base */
    ACTION_SCRAP_HAND,      /* Scrap a card from hand (if effect allows) */
    ACTION_SCRAP_DISCARD,   /* Scrap a card from discard (if effect allows) */
    ACTION_SCRAP_TRADE_ROW, /* Scrap a card from trade row (if effect allows) */
    ACTION_END_TURN         /* End the main phase */
} ActionType;
/* }}} */

/* ========================================================================== */
/*                                Structures                                  */
/* ========================================================================== */

/* {{{ Action
 * Represents a player action during main phase.
 */
typedef struct {
    ActionType type;
    int slot;               /* For trade row actions: slot index */
    char* card_instance_id; /* For play/scrap: which card */
    int target_player;      /* For attacks: which opponent */
    int amount;             /* For attacks: how much damage */
} Action;
/* }}} */

/* {{{ Game
 * The complete game state. Contains all players, the trade row,
 * and tracking for turn/phase progression.
 */
typedef struct {
    /* Players */
    Player* players[MAX_PLAYERS];
    int player_count;
    int active_player;          /* Index of current player's turn */

    /* Trade row */
    TradeRow* trade_row;

    /* Turn tracking */
    int turn_number;
    GamePhase phase;

    /* Game end state */
    bool game_over;
    int winner;                 /* Player index, -1 if draw/none */

    /* Card database - all card types in the game */
    CardType** card_types;
    int card_type_count;

    /* Starting deck card types (cached for creating new players) */
    CardType* scout_type;
    CardType* viper_type;
    CardType* explorer_type;
} Game;
/* }}} */

/* ========================================================================== */
/*                            Function Prototypes                             */
/* ========================================================================== */

/* {{{ Game lifecycle */
Game* game_create(int player_count);
void game_free(Game* game);
void game_add_player(Game* game, const char* name);
void game_set_card_types(Game* game, CardType** types, int count);
void game_set_starting_types(Game* game, CardType* scout, CardType* viper,
                             CardType* explorer);
/* }}} */

/* {{{ Game flow */
bool game_start(Game* game);
void game_start_turn(Game* game);
void game_submit_draw_order(Game* game, int* order, int count);
void game_skip_draw_order(Game* game);
bool game_process_action(Game* game, Action* action);
void game_end_turn(Game* game);
/* }}} */

/* {{{ Game state queries */
bool game_is_over(Game* game);
Player* game_get_active_player(Game* game);
Player* game_get_opponent(Game* game, int offset);
int game_get_opponent_index(Game* game, int offset);
GamePhase game_get_phase(Game* game);
/* }}} */

/* {{{ Utility */
Action* action_create(ActionType type);
void action_free(Action* action);
const char* game_phase_to_string(GamePhase phase);
const char* action_type_to_string(ActionType type);
/* }}} */

#endif /* SYMBELINE_GAME_H */
