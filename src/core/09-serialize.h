/* 09-serialize.h - Game state JSON serialization
 *
 * Converts game state to JSON for network transmission. Supports player-specific
 * views that hide opponent information (hand contents, deck order). Used by the
 * protocol layer to send game state updates to clients.
 *
 * Dependencies: cJSON library (libs/cJSON.h)
 */

#ifndef SYMBELINE_SERIALIZE_H
#define SYMBELINE_SERIALIZE_H

#include "01-card.h"
#include "02-deck.h"
#include "03-player.h"
#include "04-trade-row.h"
#include "05-game.h"
#include "../../libs/cJSON.h"
#include <stdbool.h>

/* ========================================================================== */
/*                            View Perspective                                 */
/* ========================================================================== */

/* {{{ ViewPerspective
 * Determines what information is visible when serializing game state.
 * Used to properly hide opponent hand contents from players.
 */
typedef enum {
    VIEW_SELF,       /* Full info - player sees their own data */
    VIEW_OPPONENT,   /* Limited info - hides hand contents */
    VIEW_SPECTATOR   /* Full info for all players (spectator mode) */
} ViewPerspective;
/* }}} */

/* ========================================================================== */
/*                          Game State Serialization                          */
/* ========================================================================== */

/* {{{ serialize_game_for_player
 * Serializes the full game state from a specific player's perspective.
 * Hides opponent's hand contents (shows only count), but shows all public info.
 * Caller must free the returned cJSON object with cJSON_Delete().
 * Returns NULL on failure.
 */
cJSON* serialize_game_for_player(Game* game, int player_id);
/* }}} */

/* {{{ serialize_game_full
 * Serializes the full game state with all information visible.
 * Used for debugging and server-side logging.
 * Caller must free the returned cJSON object with cJSON_Delete().
 * Returns NULL on failure.
 */
cJSON* serialize_game_full(Game* game);
/* }}} */

/* {{{ serialize_game_for_spectator
 * Serializes the full game state for spectator viewing.
 * Shows all players' hand contents (spectators can see everything).
 * Caller must free the returned cJSON object with cJSON_Delete().
 * Returns NULL on failure.
 */
cJSON* serialize_game_for_spectator(Game* game);
/* }}} */

/* ========================================================================== */
/*                          Component Serialization                           */
/* ========================================================================== */

/* {{{ serialize_card_instance
 * Serializes a single card instance with all its properties.
 * Includes type info, bonuses, placement, and visual state.
 * Caller must free the returned cJSON object with cJSON_Delete().
 */
cJSON* serialize_card_instance(CardInstance* card);
/* }}} */

/* {{{ serialize_card_type
 * Serializes a card type definition (shared card data).
 * Used for initial card database sync.
 * Caller must free the returned cJSON object with cJSON_Delete().
 */
cJSON* serialize_card_type(CardType* type);
/* }}} */

/* {{{ serialize_effect
 * Serializes a single effect.
 * Caller must free the returned cJSON object with cJSON_Delete().
 */
cJSON* serialize_effect(Effect* effect);
/* }}} */

/* {{{ serialize_player_for_view
 * Serializes player information based on viewing perspective.
 * - VIEW_SELF: Full info including hand contents
 * - VIEW_OPPONENT: Public info only (hand count, not contents)
 * - VIEW_SPECTATOR: Full info (spectators see all)
 * Caller must free the returned cJSON object with cJSON_Delete().
 */
cJSON* serialize_player_for_view(Player* player, ViewPerspective view);
/* }}} */

/* {{{ serialize_player_public
 * Serializes publicly visible player information:
 * - authority, d10, d4, bases, hand_count, deck_count, discard pile
 * Used for opponent view in player-specific serialization.
 * Caller must free the returned cJSON object with cJSON_Delete().
 */
cJSON* serialize_player_public(Player* player);
/* }}} */

/* {{{ serialize_player_private
 * Serializes full player information including hand contents.
 * Used for the requesting player's own data.
 * Caller must free the returned cJSON object with cJSON_Delete().
 */
cJSON* serialize_player_private(Player* player);
/* }}} */

/* {{{ serialize_trade_row
 * Serializes the trade row with all visible cards.
 * Includes explorer availability.
 * Caller must free the returned cJSON object with cJSON_Delete().
 */
cJSON* serialize_trade_row(TradeRow* row);
/* }}} */

/* {{{ serialize_card_array
 * Serializes an array of card instances.
 * Caller must free the returned cJSON object with cJSON_Delete().
 */
cJSON* serialize_card_array(CardInstance** cards, int count);
/* }}} */

/* ========================================================================== */
/*                           Action Deserialization                           */
/* ========================================================================== */

/* {{{ deserialize_action
 * Parses a client action from JSON.
 * Returns a newly allocated Action struct, or NULL on parse error.
 * Caller must free with action_free().
 *
 * Expected JSON format:
 * {
 *   "type": "play_card" | "buy_card" | "buy_explorer" | "attack_player" |
 *           "attack_base" | "scrap_hand" | "scrap_discard" |
 *           "scrap_trade_row" | "end_turn",
 *   "slot": 0-4,              // for trade row actions
 *   "card_id": "abc123",      // for play/scrap actions
 *   "target": 1,              // for attacks
 *   "amount": 5               // for attacks
 * }
 */
Action* deserialize_action(cJSON* json);
/* }}} */

/* ========================================================================== */
/*                              Utility Functions                             */
/* ========================================================================== */

/* {{{ game_state_to_string
 * Convenience function: serializes game state and returns JSON string.
 * Caller must free the returned string.
 * Returns NULL on failure.
 */
char* game_state_to_string(Game* game, int player_id);
/* }}} */

/* {{{ game_state_to_string_pretty
 * Same as game_state_to_string but with formatted output.
 * Caller must free the returned string.
 * Returns NULL on failure.
 */
char* game_state_to_string_pretty(Game* game, int player_id);
/* }}} */

#endif /* SYMBELINE_SERIALIZE_H */
