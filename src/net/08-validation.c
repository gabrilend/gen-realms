/* 08-validation.c - Server-side Action Validation Implementation
 *
 * All client actions are validated before execution to prevent cheating,
 * handle malformed requests, and provide helpful error messages.
 */

#include "08-validation.h"
#include <string.h>
#include <stdio.h>

/* ========================================================================== */
/*                         Validation Result Helpers                          */
/* ========================================================================== */

/* {{{ validation_ok
 * Creates a successful validation result.
 */
ValidationResult validation_ok(void) {
    ValidationResult result;
    result.valid = true;
    result.error = PROTOCOL_OK;
    result.error_message[0] = '\0';
    return result;
}
/* }}} */

/* {{{ validation_fail
 * Creates a failed validation result with error code and message.
 */
ValidationResult validation_fail(ProtocolError error, const char* message) {
    ValidationResult result;
    result.valid = false;
    result.error = error;
    if (message) {
        strncpy(result.error_message, message, sizeof(result.error_message) - 1);
        result.error_message[sizeof(result.error_message) - 1] = '\0';
    } else {
        result.error_message[0] = '\0';
    }
    return result;
}
/* }}} */

/* ========================================================================== */
/*                         Turn and Phase Validation                          */
/* ========================================================================== */

/* {{{ validate_is_player_turn
 * Validates that it is the specified player's turn.
 */
ValidationResult validate_is_player_turn(Game* game, int player_id) {
    if (!game) {
        return validation_fail(PROTOCOL_ERROR_GAME_NOT_STARTED,
                               "Game not initialized");
    }

    if (player_id < 0 || player_id >= game->player_count) {
        return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                               "Invalid player ID");
    }

    if (game->active_player != player_id) {
        return validation_fail(PROTOCOL_ERROR_NOT_YOUR_TURN,
                               "It is not your turn");
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_phase
 * Validates that the game is in the expected phase.
 */
ValidationResult validate_phase(Game* game, GamePhase expected_phase) {
    if (!game) {
        return validation_fail(PROTOCOL_ERROR_GAME_NOT_STARTED,
                               "Game not initialized");
    }

    if (game->phase != expected_phase) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Invalid phase: expected %s, got %s",
                 game_phase_to_string(expected_phase),
                 game_phase_to_string(game->phase));
        return validation_fail(PROTOCOL_ERROR_INVALID_PHASE, msg);
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_game_in_progress
 * Validates that the game has started and is not over.
 */
ValidationResult validate_game_in_progress(Game* game) {
    if (!game) {
        return validation_fail(PROTOCOL_ERROR_GAME_NOT_STARTED,
                               "Game not initialized");
    }

    if (game->phase == PHASE_NOT_STARTED) {
        return validation_fail(PROTOCOL_ERROR_GAME_NOT_STARTED,
                               "Game has not started");
    }

    if (game->phase == PHASE_GAME_OVER || game->game_over) {
        return validation_fail(PROTOCOL_ERROR_INVALID_PHASE,
                               "Game is over");
    }

    return validation_ok();
}
/* }}} */

/* ========================================================================== */
/*                            Action Validators                               */
/* ========================================================================== */

/* {{{ validate_play_card
 * Validates a play_card action.
 * Checks: turn ownership, main phase, card exists in player's hand.
 */
ValidationResult validate_play_card(Game* game, int player_id,
                                    const char* instance_id) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase */
    result = validate_phase(game, PHASE_MAIN);
    if (!result.valid) return result;

    /* Check instance_id provided */
    if (!instance_id || instance_id[0] == '\0') {
        return validation_fail(PROTOCOL_ERROR_MISSING_FIELD,
                               "Missing card instance ID");
    }

    /* Check card is in player's hand */
    Player* player = game->players[player_id];
    if (!player || !player->deck) {
        return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                               "Player state not found");
    }

    CardInstance* card = deck_find_in_hand(player->deck, instance_id);
    if (!card) {
        return validation_fail(PROTOCOL_ERROR_CARD_NOT_IN_HAND,
                               "Card not found in hand");
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_buy_card
 * Validates a buy_card action.
 * Checks: turn ownership, main phase, slot valid, has enough trade.
 */
ValidationResult validate_buy_card(Game* game, int player_id, int slot) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase */
    result = validate_phase(game, PHASE_MAIN);
    if (!result.valid) return result;

    /* Check slot range */
    if (slot < 0 || slot >= TRADE_ROW_SLOTS) {
        return validation_fail(PROTOCOL_ERROR_INVALID_SLOT,
                               "Invalid trade row slot (must be 0-4)");
    }

    /* Check trade row exists */
    if (!game->trade_row) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Trade row not available");
    }

    /* Check slot has a card */
    CardInstance* card = trade_row_get_slot(game->trade_row, slot);
    if (!card) {
        return validation_fail(PROTOCOL_ERROR_INVALID_SLOT,
                               "Trade row slot is empty");
    }

    /* Check player has enough trade */
    Player* player = game->players[player_id];
    if (!player) {
        return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                               "Player state not found");
    }

    int cost = trade_row_get_cost(game->trade_row, slot);
    if (player->trade < cost) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Not enough trade: need %d, have %d",
                 cost, player->trade);
        return validation_fail(PROTOCOL_ERROR_INSUFFICIENT_TRADE, msg);
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_buy_explorer
 * Validates a buy_explorer action.
 * Checks: turn ownership, main phase, explorers available, has enough trade.
 */
ValidationResult validate_buy_explorer(Game* game, int player_id) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase */
    result = validate_phase(game, PHASE_MAIN);
    if (!result.valid) return result;

    /* Check trade row exists */
    if (!game->trade_row) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Trade row not available");
    }

    /* Check explorer type exists */
    if (!game->trade_row->explorer_type) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Explorers not available");
    }

    /* Check player has enough trade */
    Player* player = game->players[player_id];
    if (!player) {
        return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                               "Player state not found");
    }

    if (player->trade < EXPLORER_COST) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Not enough trade: need %d, have %d",
                 EXPLORER_COST, player->trade);
        return validation_fail(PROTOCOL_ERROR_INSUFFICIENT_TRADE, msg);
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_attack_player
 * Validates an attack_player action.
 * Checks: turn ownership, main phase, has combat, no outpost blocking,
 *         target is valid opponent, amount <= available combat.
 */
ValidationResult validate_attack_player(Game* game, int player_id,
                                        int target_player, int amount) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase */
    result = validate_phase(game, PHASE_MAIN);
    if (!result.valid) return result;

    /* Check target is valid opponent */
    if (target_player < 0 || target_player >= game->player_count) {
        return validation_fail(PROTOCOL_ERROR_INVALID_TARGET,
                               "Invalid target player");
    }

    if (target_player == player_id) {
        return validation_fail(PROTOCOL_ERROR_INVALID_TARGET,
                               "Cannot attack yourself");
    }

    /* Check amount is positive */
    if (amount <= 0) {
        return validation_fail(PROTOCOL_ERROR_INVALID_VALUE,
                               "Attack amount must be positive");
    }

    /* Check player has enough combat */
    Player* player = game->players[player_id];
    if (!player) {
        return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                               "Player state not found");
    }

    if (player->combat < amount) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Not enough combat: need %d, have %d",
                 amount, player->combat);
        return validation_fail(PROTOCOL_ERROR_INSUFFICIENT_COMBAT, msg);
    }

    /* Check no outpost is blocking */
    if (!combat_can_attack_player(game, target_player)) {
        return validation_fail(PROTOCOL_ERROR_INVALID_TARGET,
                               "Cannot attack player: outpost must be destroyed first");
    }

    return validation_ok();
}
/* }}} */

/* {{{ find_base_by_id
 * Helper to find a base by instance_id in a player's bases.
 */
static CardInstance* find_base_by_id(Player* player, const char* instance_id) {
    if (!player || !player->deck || !instance_id) return NULL;

    /* Check frontier bases */
    for (int i = 0; i < player->deck->frontier_base_count; i++) {
        CardInstance* base = player->deck->frontier_bases[i];
        if (base && base->instance_id &&
            strcmp(base->instance_id, instance_id) == 0) {
            return base;
        }
    }

    /* Check interior bases */
    for (int i = 0; i < player->deck->interior_base_count; i++) {
        CardInstance* base = player->deck->interior_bases[i];
        if (base && base->instance_id &&
            strcmp(base->instance_id, instance_id) == 0) {
            return base;
        }
    }

    return NULL;
}
/* }}} */

/* {{{ is_base_in_frontier
 * Helper to check if base is in frontier (outpost rules).
 */
static bool is_base_in_frontier(Player* player, CardInstance* base) {
    if (!player || !player->deck || !base) return false;

    for (int i = 0; i < player->deck->frontier_base_count; i++) {
        if (player->deck->frontier_bases[i] == base) {
            return true;
        }
    }

    return false;
}
/* }}} */

/* {{{ validate_attack_base
 * Validates an attack_base action.
 * Checks: turn ownership, main phase, has combat, base exists,
 *         outpost rules respected, amount <= available combat.
 */
ValidationResult validate_attack_base(Game* game, int player_id,
                                      int target_player,
                                      const char* base_instance_id,
                                      int amount) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase */
    result = validate_phase(game, PHASE_MAIN);
    if (!result.valid) return result;

    /* Check target is valid opponent */
    if (target_player < 0 || target_player >= game->player_count) {
        return validation_fail(PROTOCOL_ERROR_INVALID_TARGET,
                               "Invalid target player");
    }

    if (target_player == player_id) {
        return validation_fail(PROTOCOL_ERROR_INVALID_TARGET,
                               "Cannot attack your own base");
    }

    /* Check base_instance_id provided */
    if (!base_instance_id || base_instance_id[0] == '\0') {
        return validation_fail(PROTOCOL_ERROR_MISSING_FIELD,
                               "Missing base instance ID");
    }

    /* Check amount is positive */
    if (amount <= 0) {
        return validation_fail(PROTOCOL_ERROR_INVALID_VALUE,
                               "Attack amount must be positive");
    }

    /* Check player has enough combat */
    Player* player = game->players[player_id];
    if (!player) {
        return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                               "Player state not found");
    }

    if (player->combat < amount) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Not enough combat: need %d, have %d",
                 amount, player->combat);
        return validation_fail(PROTOCOL_ERROR_INSUFFICIENT_COMBAT, msg);
    }

    /* Check base exists on target player */
    Player* target = game->players[target_player];
    if (!target) {
        return validation_fail(PROTOCOL_ERROR_INVALID_TARGET,
                               "Target player not found");
    }

    CardInstance* base = find_base_by_id(target, base_instance_id);
    if (!base) {
        return validation_fail(PROTOCOL_ERROR_CARD_NOT_FOUND,
                               "Base not found on target player");
    }

    /* Check outpost rules: if target has frontier bases, must attack frontier */
    if (deck_has_frontier_bases(target->deck)) {
        if (!is_base_in_frontier(target, base)) {
            return validation_fail(PROTOCOL_ERROR_INVALID_TARGET,
                                   "Must attack frontier base first");
        }
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_scrap_hand
 * Validates scrapping a card from hand.
 * Checks: turn ownership, main phase or pending action, card in hand.
 */
ValidationResult validate_scrap_hand(Game* game, int player_id,
                                     const char* instance_id) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase - allow in main phase */
    if (game->phase != PHASE_MAIN) {
        return validation_fail(PROTOCOL_ERROR_INVALID_PHASE,
                               "Can only scrap during main phase");
    }

    /* Check instance_id provided */
    if (!instance_id || instance_id[0] == '\0') {
        return validation_fail(PROTOCOL_ERROR_MISSING_FIELD,
                               "Missing card instance ID");
    }

    /* Check card is in player's hand */
    Player* player = game->players[player_id];
    if (!player || !player->deck) {
        return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                               "Player state not found");
    }

    CardInstance* card = deck_find_in_hand(player->deck, instance_id);
    if (!card) {
        return validation_fail(PROTOCOL_ERROR_CARD_NOT_IN_HAND,
                               "Card not found in hand");
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_scrap_discard
 * Validates scrapping a card from discard pile.
 * Checks: turn ownership, main phase or pending action, card in discard.
 */
ValidationResult validate_scrap_discard(Game* game, int player_id,
                                        const char* instance_id) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase */
    if (game->phase != PHASE_MAIN) {
        return validation_fail(PROTOCOL_ERROR_INVALID_PHASE,
                               "Can only scrap during main phase");
    }

    /* Check instance_id provided */
    if (!instance_id || instance_id[0] == '\0') {
        return validation_fail(PROTOCOL_ERROR_MISSING_FIELD,
                               "Missing card instance ID");
    }

    /* Check card is in player's discard */
    Player* player = game->players[player_id];
    if (!player || !player->deck) {
        return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                               "Player state not found");
    }

    CardInstance* card = deck_find_in_discard(player->deck, instance_id);
    if (!card) {
        return validation_fail(PROTOCOL_ERROR_CARD_NOT_FOUND,
                               "Card not found in discard pile");
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_scrap_trade_row
 * Validates scrapping a card from trade row.
 * Checks: turn ownership, main phase or pending action, slot valid.
 */
ValidationResult validate_scrap_trade_row(Game* game, int player_id, int slot) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase */
    if (game->phase != PHASE_MAIN) {
        return validation_fail(PROTOCOL_ERROR_INVALID_PHASE,
                               "Can only scrap during main phase");
    }

    /* Check slot range */
    if (slot < 0 || slot >= TRADE_ROW_SLOTS) {
        return validation_fail(PROTOCOL_ERROR_INVALID_SLOT,
                               "Invalid trade row slot (must be 0-4)");
    }

    /* Check trade row exists */
    if (!game->trade_row) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Trade row not available");
    }

    /* Check slot has a card */
    CardInstance* card = trade_row_get_slot(game->trade_row, slot);
    if (!card) {
        return validation_fail(PROTOCOL_ERROR_INVALID_SLOT,
                               "Trade row slot is empty");
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_end_turn
 * Validates ending the turn.
 * Checks: turn ownership, main phase, no pending actions.
 */
ValidationResult validate_end_turn(Game* game, int player_id) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase */
    if (game->phase != PHASE_MAIN) {
        return validation_fail(PROTOCOL_ERROR_INVALID_PHASE,
                               "Can only end turn during main phase");
    }

    /* Check no pending actions */
    if (game_has_pending_action(game)) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Must resolve pending action before ending turn");
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_draw_order
 * Validates a draw order selection.
 * Checks: turn ownership, draw_order phase, valid indices, correct count.
 */
ValidationResult validate_draw_order(Game* game, int player_id,
                                     int* order, int count) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check turn ownership */
    result = validate_is_player_turn(game, player_id);
    if (!result.valid) return result;

    /* Check phase */
    result = validate_phase(game, PHASE_DRAW_ORDER);
    if (!result.valid) return result;

    /* Check order array provided */
    if (!order || count <= 0) {
        return validation_fail(PROTOCOL_ERROR_MISSING_FIELD,
                               "Missing draw order array");
    }

    /* Get expected hand size */
    Player* player = game->players[player_id];
    if (!player) {
        return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                               "Player state not found");
    }

    int expected_count = player_get_hand_size(player);
    int available = player->deck ? player->deck->draw_pile_count : 0;
    if (expected_count > available) {
        expected_count = available;
    }

    /* Check count matches expected */
    if (count != expected_count) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Wrong draw count: expected %d, got %d",
                 expected_count, count);
        return validation_fail(PROTOCOL_ERROR_INVALID_DRAW_ORDER, msg);
    }

    /* Check indices are valid and unique */
    bool used[32] = {false};  /* Assuming max draw <= 32 */
    for (int i = 0; i < count; i++) {
        int idx = order[i];

        /* Check range */
        if (idx < 0 || idx >= available) {
            char msg[128];
            snprintf(msg, sizeof(msg), "Invalid draw index %d (max %d)",
                     idx, available - 1);
            return validation_fail(PROTOCOL_ERROR_INVALID_DRAW_ORDER, msg);
        }

        /* Check uniqueness */
        if (used[idx]) {
            char msg[128];
            snprintf(msg, sizeof(msg), "Duplicate draw index %d", idx);
            return validation_fail(PROTOCOL_ERROR_INVALID_DRAW_ORDER, msg);
        }
        used[idx] = true;
    }

    return validation_ok();
}
/* }}} */

/* ========================================================================== */
/*                         High-Level Action Validation                       */
/* ========================================================================== */

/* {{{ validate_action
 * Validates any action from the Action struct.
 * Dispatches to specific validators based on action type.
 */
ValidationResult validate_action(Game* game, int player_id, Action* action) {
    if (!action) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Null action");
    }

    switch (action->type) {
        case ACTION_PLAY_CARD:
            return validate_play_card(game, player_id,
                                      action->card_instance_id);

        case ACTION_BUY_CARD:
            return validate_buy_card(game, player_id, action->slot);

        case ACTION_BUY_EXPLORER:
            return validate_buy_explorer(game, player_id);

        case ACTION_ATTACK_PLAYER:
            return validate_attack_player(game, player_id,
                                          action->target_player,
                                          action->amount);

        case ACTION_ATTACK_BASE:
            return validate_attack_base(game, player_id,
                                        action->target_player,
                                        action->card_instance_id,
                                        action->amount);

        case ACTION_SCRAP_HAND:
            return validate_scrap_hand(game, player_id,
                                       action->card_instance_id);

        case ACTION_SCRAP_DISCARD:
            return validate_scrap_discard(game, player_id,
                                          action->card_instance_id);

        case ACTION_SCRAP_TRADE_ROW:
            return validate_scrap_trade_row(game, player_id, action->slot);

        case ACTION_END_TURN:
            return validate_end_turn(game, player_id);

        default:
            return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                                   "Unknown action type");
    }
}
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
                                           const char* card_instance_id) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check there is a pending action */
    if (!game_has_pending_action(game)) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "No pending action to respond to");
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Failed to get pending action");
    }

    /* Check player matches */
    if (pending->player_id != player_id) {
        return validation_fail(PROTOCOL_ERROR_NOT_YOUR_TURN,
                               "Pending action is for a different player");
    }

    /* Check response type matches pending type */
    bool type_matches = false;
    switch (pending->type) {
        case PENDING_DISCARD:
            type_matches = (response_type == PENDING_DISCARD);
            break;
        case PENDING_SCRAP_TRADE_ROW:
            type_matches = (response_type == PENDING_SCRAP_TRADE_ROW);
            break;
        case PENDING_SCRAP_HAND:
            type_matches = (response_type == PENDING_SCRAP_HAND);
            break;
        case PENDING_SCRAP_DISCARD:
            type_matches = (response_type == PENDING_SCRAP_DISCARD);
            break;
        case PENDING_SCRAP_HAND_DISCARD:
            type_matches = (response_type == PENDING_SCRAP_HAND ||
                           response_type == PENDING_SCRAP_DISCARD);
            break;
        case PENDING_TOP_DECK:
            type_matches = (response_type == PENDING_TOP_DECK);
            break;
        case PENDING_COPY_SHIP:
            type_matches = (response_type == PENDING_COPY_SHIP);
            break;
        case PENDING_DESTROY_BASE:
            type_matches = (response_type == PENDING_DESTROY_BASE);
            break;
        case PENDING_UPGRADE:
            type_matches = (response_type == PENDING_UPGRADE);
            break;
        default:
            break;
    }

    if (!type_matches) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Response type does not match pending action");
    }

    /* Validate card_instance_id if required */
    if (card_instance_id && card_instance_id[0] != '\0') {
        Player* player = game->players[player_id];
        if (!player || !player->deck) {
            return validation_fail(PROTOCOL_ERROR_NOT_IN_GAME,
                                   "Player state not found");
        }

        /* Check card location based on pending type */
        CardInstance* card = NULL;
        switch (response_type) {
            case PENDING_DISCARD:
            case PENDING_SCRAP_HAND:
                card = deck_find_in_hand(player->deck, card_instance_id);
                if (!card) {
                    return validation_fail(PROTOCOL_ERROR_CARD_NOT_IN_HAND,
                                           "Card not found in hand");
                }
                break;
            case PENDING_SCRAP_DISCARD:
            case PENDING_TOP_DECK:
                card = deck_find_in_discard(player->deck, card_instance_id);
                if (!card) {
                    return validation_fail(PROTOCOL_ERROR_CARD_NOT_FOUND,
                                           "Card not found in discard pile");
                }
                break;
            default:
                /* Other pending types have different validation */
                break;
        }
    }

    return validation_ok();
}
/* }}} */

/* {{{ validate_pending_skip
 * Validates skipping a pending action.
 * Checks: player matches pending, pending is optional.
 */
ValidationResult validate_pending_skip(Game* game, int player_id) {
    ValidationResult result;

    /* Check game state */
    result = validate_game_in_progress(game);
    if (!result.valid) return result;

    /* Check there is a pending action */
    if (!game_has_pending_action(game)) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "No pending action to skip");
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Failed to get pending action");
    }

    /* Check player matches */
    if (pending->player_id != player_id) {
        return validation_fail(PROTOCOL_ERROR_NOT_YOUR_TURN,
                               "Pending action is for a different player");
    }

    /* Check action is optional */
    if (!pending->optional && pending->min_count > 0) {
        return validation_fail(PROTOCOL_ERROR_INVALID_ACTION,
                               "Cannot skip: pending action is mandatory");
    }

    return validation_ok();
}
/* }}} */
