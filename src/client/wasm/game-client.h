/*
 * game-client.h - Main Game Client for WASM
 *
 * Integrates all WASM modules into a unified game client.
 * Manages game state, rendering, input, and network communication.
 */

#ifndef WASM_GAME_CLIENT_H
#define WASM_GAME_CLIENT_H

#include <stdbool.h>
#include <stdint.h>
#include "theme.h"
#include "zone-renderer.h"
#include "panel-renderer.h"

/* {{{ Game states */
typedef enum {
    GAME_STATE_LOADING,
    GAME_STATE_MENU,
    GAME_STATE_CONNECTING,
    GAME_STATE_WAITING,
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER,
    GAME_STATE_ERROR
} GameState;
/* }}} */

/* {{{ Action modes */
typedef enum {
    ACTION_NONE,
    ACTION_SELECT_HAND_CARD,
    ACTION_SELECT_TRADE_CARD,
    ACTION_SELECT_TARGET_BASE,
    ACTION_CONFIRM_ATTACK,
    ACTION_CONFIRM_PURCHASE
} ActionMode;
/* }}} */

/* {{{ GameClientConfig
 * Configuration for game client.
 */
typedef struct {
    const char* canvas_id;
    const char* server_url;
    const char* player_name;
    bool enable_narrative;
    bool enable_ai_opponent;
} GameClientConfig;
/* }}} */

/* {{{ GameData
 * Current game data from server.
 */
typedef struct {
    /* Player info */
    PlayerStats player;
    PlayerStats opponent;

    /* Turn state */
    TurnInfo turn;

    /* Zones */
    HandZoneData hand;
    TradeRowData trade_row;
    BasesZoneData player_bases;
    BasesZoneData opp_bases;
    PlayAreaData play_area;

    /* Narrative */
    NarrativeData narrative;
} GameData;
/* }}} */

/* {{{ game_init
 * Initialize the game client.
 * @param config - Client configuration
 * @return true on success
 */
bool game_init(const GameClientConfig* config);
/* }}} */

/* {{{ game_cleanup
 * Clean up game client resources.
 */
void game_cleanup(void);
/* }}} */

/* {{{ game_start
 * Start the game loop.
 * This sets up the main loop with emscripten.
 */
void game_start(void);
/* }}} */

/* {{{ game_stop
 * Stop the game loop.
 */
void game_stop(void);
/* }}} */

/* {{{ game_connect
 * Connect to game server.
 * @param url - Server WebSocket URL
 * @return true if connection initiated
 */
bool game_connect(const char* url);
/* }}} */

/* {{{ game_disconnect
 * Disconnect from game server.
 */
void game_disconnect(void);
/* }}} */

/* {{{ game_get_state
 * Get current game state.
 * @return Current state
 */
GameState game_get_state(void);
/* }}} */

/* {{{ game_get_data
 * Get current game data.
 * @return Pointer to game data (read-only)
 */
const GameData* game_get_data(void);
/* }}} */

/* {{{ game_send_action
 * Send a player action to server.
 * @param action_type - Action type string
 * @param target_id - Target ID (-1 if none)
 * @return true if sent
 */
bool game_send_action(const char* action_type, int target_id);
/* }}} */

/* {{{ game_end_turn
 * End the current turn.
 * @return true if sent
 */
bool game_end_turn(void);
/* }}} */

/* {{{ game_play_card
 * Play a card from hand.
 * @param hand_index - Index in hand
 * @return true if sent
 */
bool game_play_card(int hand_index);
/* }}} */

/* {{{ game_buy_card
 * Buy a card from trade row.
 * @param trade_index - Index in trade row
 * @return true if sent
 */
bool game_buy_card(int trade_index);
/* }}} */

/* {{{ game_attack_base
 * Attack an opponent's base.
 * @param base_index - Index of target base
 * @return true if sent
 */
bool game_attack_base(int base_index);
/* }}} */

/* {{{ game_attack_authority
 * Attack opponent's authority directly.
 * @param amount - Combat amount to use
 * @return true if sent
 */
bool game_attack_authority(int amount);
/* }}} */

/* {{{ game_set_action_mode
 * Set the current action mode.
 * @param mode - New action mode
 */
void game_set_action_mode(ActionMode mode);
/* }}} */

/* {{{ game_cancel_action
 * Cancel current action and reset mode.
 */
void game_cancel_action(void);
/* }}} */

/* {{{ Entry point (called from HTML/JS) */
void game_main(void);
/* }}} */

#endif /* WASM_GAME_CLIENT_H */
