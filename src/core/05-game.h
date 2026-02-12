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

/* Maximum pending actions in queue */
#define MAX_PENDING_ACTIONS 8

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

/* {{{ PendingActionType
 * Types of pending actions that require player input to resolve.
 */
typedef enum {
    PENDING_NONE = 0,           /* No pending action */
    PENDING_DISCARD,            /* Player must discard N cards from hand */
    PENDING_SCRAP_TRADE_ROW,    /* Player may scrap from trade row */
    PENDING_SCRAP_HAND,         /* Player may scrap from hand */
    PENDING_SCRAP_DISCARD,      /* Player may scrap from discard pile */
    PENDING_SCRAP_HAND_DISCARD, /* Player may scrap from hand or discard */
    PENDING_TOP_DECK,           /* Player may put discard card on top of deck */
    PENDING_COPY_SHIP,          /* Player chooses ship to copy */
    PENDING_DESTROY_BASE,       /* Player chooses base to destroy */
    PENDING_UPGRADE,            /* Player chooses card to upgrade */
} PendingActionType;
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

/* Forward declare for PendingAction */
struct CardInstance;
struct Effect;

/* {{{ PendingAction
 * Represents a deferred action waiting for player input.
 * Created by effects that need choices (discard, scrap, etc.).
 */
typedef struct {
    PendingActionType type;     /* Type of pending action */
    int player_id;              /* Which player must respond */
    int count;                  /* How many choices (cards to discard, etc.) */
    int min_count;              /* Minimum required (0 = optional) */
    int resolved_count;         /* How many have been resolved so far */
    bool optional;              /* Can player skip this action? */
    struct CardInstance* source_card;   /* Card that triggered this */
    struct Effect* source_effect;       /* Effect that created this pending */
    int upgrade_type;           /* For upgrades: which stat to boost */
    int upgrade_value;          /* For upgrades: how much to boost */
} PendingAction;
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

    /* Pending action queue for deferred player choices */
    PendingAction pending_actions[MAX_PENDING_ACTIONS];
    int pending_count;
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

/* {{{ Card database */
CardType* game_find_card_type(Game* game, const char* id);
void game_register_card_type(Game* game, CardType* type);
/* }}} */

/* {{{ Base effects */
void game_process_base_effects(Game* game, Player* player);
void game_deploy_new_bases(Game* game, Player* player);
/* }}} */

/* {{{ Utility */
Action* action_create(ActionType type);
void action_free(Action* action);
const char* game_phase_to_string(GamePhase phase);
const char* action_type_to_string(ActionType type);
const char* pending_action_type_to_string(PendingActionType type);
/* }}} */

/* {{{ Pending actions */
bool game_has_pending_action(Game* game);
PendingAction* game_get_pending_action(Game* game);
void game_push_pending_action(Game* game, PendingAction* action);
void game_pop_pending_action(Game* game);
void game_clear_pending_actions(Game* game);

/* Pending action creation helpers */
void game_request_discard(Game* game, int player_id, int count);
void game_request_scrap_trade_row(Game* game, int player_id, int count);
void game_request_scrap_hand(Game* game, int player_id, int count);
void game_request_scrap_discard(Game* game, int player_id, int count);
void game_request_scrap_hand_discard(Game* game, int player_id, int count);
void game_request_top_deck(Game* game, int player_id, int count);

/* Pending action resolution */
bool game_resolve_discard(Game* game, const char* card_instance_id);
bool game_resolve_scrap_trade_row(Game* game, int slot);
bool game_resolve_scrap_hand(Game* game, const char* card_instance_id);
bool game_resolve_scrap_discard(Game* game, const char* card_instance_id);
bool game_resolve_top_deck(Game* game, const char* card_instance_id);
bool game_skip_pending_action(Game* game);

/* Special effect pending actions */
void game_request_copy_ship(Game* game, int player_id);
void game_request_destroy_base(Game* game, int player_id);
bool game_resolve_copy_ship(Game* game, const char* card_instance_id);
bool game_resolve_destroy_base(Game* game, const char* card_instance_id);

/* Upgrade pending actions */
void game_request_upgrade(Game* game, int player_id, int upgrade_type, int upgrade_value);
bool game_resolve_upgrade(Game* game, const char* card_instance_id);

/* Purchase functions with effect context support */
CardInstance* game_buy_card(Game* game, int slot);
CardInstance* game_buy_explorer(Game* game);
/* }}} */

#endif /* SYMBELINE_GAME_H */
