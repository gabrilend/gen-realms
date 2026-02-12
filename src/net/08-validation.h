/* 08-validation.h - Server-side Action Validation
 *
 * Validates all client actions before execution. Prevents illegal moves,
 * handles malformed requests, and returns clear error messages. All game
 * actions must pass validation before being processed by the game engine.
 *
 * Validation checks include:
 * - Turn ownership (is it this player's turn?)
 * - Phase appropriateness (correct phase for action)
 * - Resource sufficiency (enough trade/combat)
 * - Target validity (card in hand, valid slot, etc.)
 */

#ifndef SYMBELINE_VALIDATION_H
#define SYMBELINE_VALIDATION_H

#include "../core/05-game.h"
#include "../core/06-combat.h"
#include "04-protocol.h"
#include <stdbool.h>

/* ========================================================================== */
/*                            Validation Result                               */
/* ========================================================================== */

/* {{{ ValidationResult
 * Result of validating a player action.
 * Contains success/failure flag and error details on failure.
 */
typedef struct {
    bool valid;             /* True if action is valid */
    ProtocolError error;    /* Error code if invalid */
    char error_message[128]; /* Human-readable error description */
} ValidationResult;
/* }}} */

/* ========================================================================== */
/*                         Validation Result Helpers                          */
/* ========================================================================== */

/* {{{ validation_ok
 * Creates a successful validation result.
 */
ValidationResult validation_ok(void);
/* }}} */

/* {{{ validation_fail
 * Creates a failed validation result with error code and message.
 */
ValidationResult validation_fail(ProtocolError error, const char* message);
/* }}} */

/* ========================================================================== */
/*                         Turn and Phase Validation                          */
/* ========================================================================== */

/* {{{ validate_is_player_turn
 * Validates that it is the specified player's turn.
 */
ValidationResult validate_is_player_turn(Game* game, int player_id);
/* }}} */

/* {{{ validate_phase
 * Validates that the game is in the expected phase.
 */
ValidationResult validate_phase(Game* game, GamePhase expected_phase);
/* }}} */

/* {{{ validate_game_in_progress
 * Validates that the game has started and is not over.
 */
ValidationResult validate_game_in_progress(Game* game);
/* }}} */

/* ========================================================================== */
/*                            Action Validators                               */
/* ========================================================================== */

/* {{{ validate_play_card
 * Validates a play_card action.
 * Checks: turn ownership, main phase, card exists in player's hand.
 */
ValidationResult validate_play_card(Game* game, int player_id,
                                    const char* instance_id);
/* }}} */

/* {{{ validate_buy_card
 * Validates a buy_card action.
 * Checks: turn ownership, main phase, slot valid, has enough trade.
 */
ValidationResult validate_buy_card(Game* game, int player_id, int slot);
/* }}} */

/* {{{ validate_buy_explorer
 * Validates a buy_explorer action.
 * Checks: turn ownership, main phase, explorers available, has enough trade.
 */
ValidationResult validate_buy_explorer(Game* game, int player_id);
/* }}} */

/* {{{ validate_attack_player
 * Validates an attack_player action.
 * Checks: turn ownership, main phase, has combat, no outpost blocking,
 *         target is valid opponent, amount <= available combat.
 */
ValidationResult validate_attack_player(Game* game, int player_id,
                                        int target_player, int amount);
/* }}} */

/* {{{ validate_attack_base
 * Validates an attack_base action.
 * Checks: turn ownership, main phase, has combat, base exists,
 *         outpost rules respected, amount <= available combat.
 */
ValidationResult validate_attack_base(Game* game, int player_id,
                                      int target_player,
                                      const char* base_instance_id,
                                      int amount);
/* }}} */

/* {{{ validate_scrap_hand
 * Validates scrapping a card from hand.
 * Checks: turn ownership, main phase or pending action, card in hand.
 */
ValidationResult validate_scrap_hand(Game* game, int player_id,
                                     const char* instance_id);
/* }}} */

/* {{{ validate_scrap_discard
 * Validates scrapping a card from discard pile.
 * Checks: turn ownership, main phase or pending action, card in discard.
 */
ValidationResult validate_scrap_discard(Game* game, int player_id,
                                        const char* instance_id);
/* }}} */

/* {{{ validate_scrap_trade_row
 * Validates scrapping a card from trade row.
 * Checks: turn ownership, main phase or pending action, slot valid.
 */
ValidationResult validate_scrap_trade_row(Game* game, int player_id, int slot);
/* }}} */

/* {{{ validate_end_turn
 * Validates ending the turn.
 * Checks: turn ownership, main phase, no pending actions.
 */
ValidationResult validate_end_turn(Game* game, int player_id);
/* }}} */

/* {{{ validate_draw_order
 * Validates a draw order selection.
 * Checks: turn ownership, draw_order phase, valid indices, correct count.
 */
ValidationResult validate_draw_order(Game* game, int player_id,
                                     int* order, int count);
/* }}} */

/* ========================================================================== */
/*                         High-Level Action Validation                       */
/* ========================================================================== */

/* {{{ validate_action
 * Validates any action from the Action struct.
 * Dispatches to specific validators based on action type.
 */
ValidationResult validate_action(Game* game, int player_id, Action* action);
/* }}} */

/* ========================================================================== */
/*                         Pending Action Validation                          */
/* ========================================================================== */

/* {{{ validate_pending_response
 * Validates a response to a pending action (discard choice, scrap choice, etc.).
 * Checks: player matches pending, response type matches pending type.
 */
ValidationResult validate_pending_response(Game* game, int player_id,
                                           PendingActionType response_type,
                                           const char* card_instance_id);
/* }}} */

/* {{{ validate_pending_skip
 * Validates skipping a pending action.
 * Checks: player matches pending, pending is optional.
 */
ValidationResult validate_pending_skip(Game* game, int player_id);
/* }}} */

#endif /* SYMBELINE_VALIDATION_H */
