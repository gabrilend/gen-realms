/* 05-game.c - Game state and turn loop implementation
 *
 * Manages the complete game lifecycle from creation through game end.
 * Handles turn phases, player actions, and win condition detection.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "05-game.h"
#include "08-auto-draw.h"
#include <stdlib.h>
#include <string.h>

/* ========================================================================== */
/*                             Game Lifecycle                                 */
/* ========================================================================== */

/* {{{ game_create
 * Creates a new game with the specified number of players.
 * Players are added later via game_add_player().
 */
Game* game_create(int player_count) {
    if (player_count < 2 || player_count > MAX_PLAYERS) {
        return NULL;
    }

    Game* game = calloc(1, sizeof(Game));
    if (!game) {
        return NULL;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        game->players[i] = NULL;
    }

    game->player_count = 0;  /* Incremented by game_add_player */
    game->active_player = 0;
    game->trade_row = NULL;
    game->turn_number = 0;
    game->phase = PHASE_NOT_STARTED;
    game->game_over = false;
    game->winner = -1;

    game->card_types = NULL;
    game->card_type_count = 0;
    game->scout_type = NULL;
    game->viper_type = NULL;
    game->explorer_type = NULL;

    return game;
}
/* }}} */

/* {{{ game_free
 * Frees the game and all owned resources.
 */
void game_free(Game* game) {
    if (!game) {
        return;
    }

    /* Free players */
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i]) {
            player_free(game->players[i]);
        }
    }

    /* Free trade row */
    if (game->trade_row) {
        trade_row_free(game->trade_row);
    }

    /* Free card types - game owns these */
    for (int i = 0; i < game->card_type_count; i++) {
        if (game->card_types[i]) {
            card_type_free(game->card_types[i]);
        }
    }
    free(game->card_types);

    free(game);
}
/* }}} */

/* {{{ game_add_player
 * Adds a player to the game. Must be called before game_start().
 */
void game_add_player(Game* game, const char* name) {
    if (!game || game->player_count >= MAX_PLAYERS) {
        return;
    }
    if (game->phase != PHASE_NOT_STARTED) {
        return;  /* Can't add players after game starts */
    }

    Player* player = player_create(name, game->player_count + 1);
    if (!player) {
        return;
    }

    game->players[game->player_count] = player;
    game->player_count++;
}
/* }}} */

/* {{{ game_set_card_types
 * Sets the card database for the game. Game takes ownership of the array.
 * This should be called before game_start() with all trade row card types.
 */
void game_set_card_types(Game* game, CardType** types, int count) {
    if (!game || !types || count <= 0) {
        return;
    }

    /* Free existing if any */
    for (int i = 0; i < game->card_type_count; i++) {
        if (game->card_types[i]) {
            card_type_free(game->card_types[i]);
        }
    }
    free(game->card_types);

    game->card_types = types;
    game->card_type_count = count;
}
/* }}} */

/* {{{ game_set_starting_types
 * Sets the starting deck card types (scout, viper, explorer).
 * These are used for player starting decks and explorer purchases.
 */
void game_set_starting_types(Game* game, CardType* scout, CardType* viper,
                             CardType* explorer) {
    if (!game) {
        return;
    }
    game->scout_type = scout;
    game->viper_type = viper;
    game->explorer_type = explorer;
}
/* }}} */

/* ========================================================================== */
/*                              Game Flow                                     */
/* ========================================================================== */

/* {{{ game_init_player_deck
 * Internal: Creates the starting deck for a player.
 */
static void game_init_player_deck(Game* game, Player* player) {
    if (!game || !player || !game->scout_type || !game->viper_type) {
        return;
    }

    /* Add scouts */
    for (int i = 0; i < STARTING_SCOUTS; i++) {
        CardInstance* scout = card_instance_create(game->scout_type);
        if (scout) {
            deck_add_to_draw_pile(player->deck, scout);
        }
    }

    /* Add vipers */
    for (int i = 0; i < STARTING_VIPERS; i++) {
        CardInstance* viper = card_instance_create(game->viper_type);
        if (viper) {
            deck_add_to_draw_pile(player->deck, viper);
        }
    }

    /* Shuffle the starting deck */
    deck_shuffle(player->deck);
}
/* }}} */

/* {{{ game_start
 * Starts the game. Sets up starting decks, trade row, draws initial hands.
 * Returns false if game cannot start (not enough players, missing card types).
 */
bool game_start(Game* game) {
    if (!game) {
        return false;
    }
    if (game->player_count < 2) {
        return false;  /* Need at least 2 players */
    }
    if (!game->scout_type || !game->viper_type) {
        return false;  /* Need starting card types */
    }
    if (game->phase != PHASE_NOT_STARTED) {
        return false;  /* Already started */
    }

    /* Initialize each player's starting deck */
    for (int i = 0; i < game->player_count; i++) {
        game_init_player_deck(game, game->players[i]);
    }

    /* Create trade row (if we have trade cards and no trade row exists) */
    if (!game->trade_row && game->card_types && game->card_type_count > 0) {
        game->trade_row = trade_row_create(game->card_types, game->card_type_count,
                                           game->explorer_type);
    }

    /* Start first turn */
    game->turn_number = 1;
    game->active_player = 0;
    game_start_turn(game);

    return true;
}
/* }}} */

/* {{{ game_start_turn
 * Begins a new turn for the active player.
 * Resets resources and enters draw order phase.
 */
void game_start_turn(Game* game) {
    if (!game || game->game_over) {
        return;
    }

    Player* player = game_get_active_player(game);
    if (!player) {
        return;
    }

    /* Reset turn resources */
    player_reset_turn(player);

    /* Deploy bases that were played last turn (they become active now) */
    game_deploy_new_bases(game, player);

    /* Process effects from deployed bases (spawning, etc.) */
    game_process_base_effects(game, player);

    /* Enter draw order selection phase */
    game->phase = PHASE_DRAW_ORDER;
}
/* }}} */

/* {{{ game_submit_draw_order
 * Player submits their preferred draw order.
 * Draws cards in that order, resolves auto-draws, then transitions to main phase.
 */
void game_submit_draw_order(Game* game, int* order, int count) {
    if (!game || game->phase != PHASE_DRAW_ORDER) {
        return;
    }

    Player* player = game_get_active_player(game);
    if (!player) {
        return;
    }

    int hand_size = player_get_hand_size(player);
    if (count != hand_size) {
        /* Order must match expected hand size */
        return;
    }

    /* Draw in specified order */
    deck_draw_ordered(player->deck, order, count);

    /* Resolve auto-draw effects before main phase */
    autodraw_resolve_chain(game, player);

    /* Transition to main phase */
    game->phase = PHASE_MAIN;
}
/* }}} */

/* {{{ game_skip_draw_order
 * Skips draw order selection, draws cards in default order.
 * Resolves auto-draws before transitioning to main phase.
 */
void game_skip_draw_order(Game* game) {
    if (!game || game->phase != PHASE_DRAW_ORDER) {
        return;
    }

    Player* player = game_get_active_player(game);
    if (!player) {
        return;
    }

    /* Draw in default order */
    player_draw_starting_hand(player);

    /* Resolve auto-draw effects before main phase */
    autodraw_resolve_chain(game, player);

    /* Transition to main phase */
    game->phase = PHASE_MAIN;
}
/* }}} */

/* {{{ game_process_action
 * Processes a player action during main phase.
 * Returns true if action was valid and executed.
 */
bool game_process_action(Game* game, Action* action) {
    if (!game || !action || game->phase != PHASE_MAIN) {
        return false;
    }

    Player* player = game_get_active_player(game);
    if (!player) {
        return false;
    }

    switch (action->type) {
        case ACTION_PLAY_CARD: {
            /* Find card in hand and play it */
            CardInstance* card = deck_find_in_hand(player->deck, action->card_instance_id);
            if (!card) {
                return false;
            }

            /* Move to played area (or bases) */
            if (!deck_play_from_hand(player->deck, card)) {
                return false;
            }

            /* Mark faction played for ally abilities */
            if (card->type) {
                player_mark_faction_played(player, card->type->faction);

                /* Apply instant effects (trade, combat, authority) */
                int trade = card_instance_total_trade(card);
                int combat = card_instance_total_combat(card);
                int auth = card_instance_total_authority(card);

                player_add_trade(player, trade);
                player_add_combat(player, combat);
                player_add_authority(player, auth);
            }
            return true;
        }

        case ACTION_BUY_CARD: {
            if (!game->trade_row) {
                return false;
            }
            CardInstance* bought = trade_row_buy(game->trade_row, action->slot, player);
            return bought != NULL;
        }

        case ACTION_BUY_EXPLORER: {
            if (!game->trade_row) {
                return false;
            }
            CardInstance* bought = trade_row_buy_explorer(game->trade_row, player);
            return bought != NULL;
        }

        case ACTION_ATTACK_PLAYER: {
            int opponent_idx = game_get_opponent_index(game, 0);
            Player* opponent = game->players[opponent_idx];
            if (!opponent) {
                return false;
            }

            /* Check for bases - all bases must be destroyed before player */
            if (deck_total_base_count(opponent->deck) > 0) {
                return false;  /* Must destroy all bases first */
            }

            int damage = action->amount;
            if (damage <= 0 || damage > player->combat) {
                return false;
            }

            player->combat -= damage;
            player_take_damage(opponent, damage);

            /* Check for game over */
            if (!player_is_alive(opponent)) {
                game->game_over = true;
                game->winner = game->active_player;
                game->phase = PHASE_GAME_OVER;
            }
            return true;
        }

        case ACTION_ATTACK_BASE: {
            /* TODO: Implement base targeting */
            return false;
        }

        case ACTION_END_TURN: {
            game_end_turn(game);
            return true;
        }

        default:
            return false;
    }
}
/* }}} */

/* {{{ game_end_turn
 * Ends the current player's turn and transitions to next player.
 */
void game_end_turn(Game* game) {
    if (!game || game->game_over) {
        return;
    }

    Player* player = game_get_active_player(game);
    if (player) {
        player_end_turn(player);
    }

    /* Switch to next player */
    game->active_player = (game->active_player + 1) % game->player_count;

    /* Increment turn number when cycling back to first player */
    if (game->active_player == 0) {
        game->turn_number++;
    }

    /* Start the next player's turn */
    game_start_turn(game);
}
/* }}} */

/* ========================================================================== */
/*                           Game State Queries                               */
/* ========================================================================== */

/* {{{ game_is_over
 * Returns true if the game has ended.
 */
bool game_is_over(Game* game) {
    if (!game) {
        return true;
    }
    return game->game_over;
}
/* }}} */

/* {{{ game_get_active_player
 * Returns the player whose turn it currently is.
 */
Player* game_get_active_player(Game* game) {
    if (!game || game->active_player < 0 || game->active_player >= game->player_count) {
        return NULL;
    }
    return game->players[game->active_player];
}
/* }}} */

/* {{{ game_get_opponent
 * Returns an opponent player. Offset 0 = next player, 1 = player after, etc.
 */
Player* game_get_opponent(Game* game, int offset) {
    int idx = game_get_opponent_index(game, offset);
    if (idx < 0) {
        return NULL;
    }
    return game->players[idx];
}
/* }}} */

/* {{{ game_get_opponent_index
 * Returns the index of an opponent player.
 */
int game_get_opponent_index(Game* game, int offset) {
    if (!game || game->player_count <= 1) {
        return -1;
    }
    return (game->active_player + 1 + offset) % game->player_count;
}
/* }}} */

/* {{{ game_get_phase
 * Returns the current game phase.
 */
GamePhase game_get_phase(Game* game) {
    if (!game) {
        return PHASE_GAME_OVER;
    }
    return game->phase;
}
/* }}} */

/* ========================================================================== */
/*                               Utility                                      */
/* ========================================================================== */

/* {{{ action_create
 * Creates a new action with the specified type.
 */
Action* action_create(ActionType type) {
    Action* action = calloc(1, sizeof(Action));
    if (!action) {
        return NULL;
    }
    action->type = type;
    action->slot = -1;
    action->card_instance_id = NULL;
    action->target_player = -1;
    action->amount = 0;
    return action;
}
/* }}} */

/* {{{ action_free
 * Frees an action and its associated strings.
 */
void action_free(Action* action) {
    if (!action) {
        return;
    }
    free(action->card_instance_id);
    free(action);
}
/* }}} */

/* {{{ game_phase_to_string
 * Returns human-readable phase name.
 */
const char* game_phase_to_string(GamePhase phase) {
    switch (phase) {
        case PHASE_NOT_STARTED: return "Not Started";
        case PHASE_DRAW_ORDER:  return "Draw Order";
        case PHASE_MAIN:        return "Main";
        case PHASE_END:         return "End";
        case PHASE_GAME_OVER:   return "Game Over";
        default:                return "Unknown";
    }
}
/* }}} */

/* {{{ action_type_to_string
 * Returns human-readable action type name.
 */
const char* action_type_to_string(ActionType type) {
    switch (type) {
        case ACTION_PLAY_CARD:       return "Play Card";
        case ACTION_BUY_CARD:        return "Buy Card";
        case ACTION_BUY_EXPLORER:    return "Buy Explorer";
        case ACTION_ATTACK_PLAYER:   return "Attack Player";
        case ACTION_ATTACK_BASE:     return "Attack Base";
        case ACTION_SCRAP_HAND:      return "Scrap from Hand";
        case ACTION_SCRAP_DISCARD:   return "Scrap from Discard";
        case ACTION_SCRAP_TRADE_ROW: return "Scrap from Trade Row";
        case ACTION_END_TURN:        return "End Turn";
        default:                     return "Unknown";
    }
}
/* }}} */

/* ========================================================================== */
/*                           Card Database                                    */
/* ========================================================================== */

/* {{{ game_find_card_type
 * Looks up a card type by its ID string.
 * Returns NULL if not found.
 */
CardType* game_find_card_type(Game* game, const char* id) {
    if (!game || !id) {
        return NULL;
    }

    for (int i = 0; i < game->card_type_count; i++) {
        if (game->card_types[i] && game->card_types[i]->id) {
            if (strcmp(game->card_types[i]->id, id) == 0) {
                return game->card_types[i];
            }
        }
    }

    return NULL;
}
/* }}} */

/* {{{ game_register_card_type
 * Registers a card type in the game's card database.
 * The game takes ownership of the card type.
 */
void game_register_card_type(Game* game, CardType* type) {
    if (!game || !type) {
        return;
    }

    /* Grow array if needed */
    int new_count = game->card_type_count + 1;
    CardType** new_array = realloc(game->card_types,
                                    new_count * sizeof(CardType*));
    if (!new_array) {
        return;  /* Allocation failure */
    }

    game->card_types = new_array;
    game->card_types[game->card_type_count] = type;
    game->card_type_count = new_count;
}
/* }}} */

/* ========================================================================== */
/*                           Base Effects                                     */
/* ========================================================================== */

/* {{{ game_deploy_new_bases
 * Marks newly-played bases as deployed after their first turn.
 * Should be called at the start of the owner's turn.
 */
void game_deploy_new_bases(Game* game, Player* player) {
    if (!game || !player || !player->deck) {
        return;
    }

    /* Deploy frontier bases */
    for (int i = 0; i < player->deck->frontier_base_count; i++) {
        CardInstance* base = player->deck->frontier_bases[i];
        if (base && !base->deployed) {
            base->deployed = true;
        }
    }

    /* Deploy interior bases */
    for (int i = 0; i < player->deck->interior_base_count; i++) {
        CardInstance* base = player->deck->interior_bases[i];
        if (base && !base->deployed) {
            base->deployed = true;
        }
    }
}
/* }}} */

/* {{{ game_process_base_effects
 * Processes turn-start effects for all deployed bases.
 * Triggers spawning for bases with spawns_id.
 */
void game_process_base_effects(Game* game, Player* player) {
    if (!game || !player || !player->deck) {
        return;
    }

    /* Process frontier bases */
    for (int i = 0; i < player->deck->frontier_base_count; i++) {
        CardInstance* base = player->deck->frontier_bases[i];
        if (!base || !base->type || !base->deployed) {
            continue;  /* Skip undeployed bases */
        }

        /* Spawn unit if base has spawns_id */
        if (base->type->spawns_id) {
            CardType* unit_type = game_find_card_type(game, base->type->spawns_id);
            if (unit_type) {
                CardInstance* unit = card_instance_create(unit_type);
                if (unit) {
                    deck_add_to_discard(player->deck, unit);
                }
            }
        }

        /* TODO: Trigger other base effects via effects_execute() */
    }

    /* Process interior bases */
    for (int i = 0; i < player->deck->interior_base_count; i++) {
        CardInstance* base = player->deck->interior_bases[i];
        if (!base || !base->type || !base->deployed) {
            continue;  /* Skip undeployed bases */
        }

        /* Spawn unit if base has spawns_id */
        if (base->type->spawns_id) {
            CardType* unit_type = game_find_card_type(game, base->type->spawns_id);
            if (unit_type) {
                CardInstance* unit = card_instance_create(unit_type);
                if (unit) {
                    deck_add_to_discard(player->deck, unit);
                }
            }
        }

        /* TODO: Trigger other base effects via effects_execute() */
    }
}
/* }}} */

/* ========================================================================== */
/*                          Pending Actions                                   */
/* ========================================================================== */

/* {{{ pending_action_type_to_string
 * Returns human-readable pending action type name.
 */
const char* pending_action_type_to_string(PendingActionType type) {
    switch (type) {
        case PENDING_NONE:              return "None";
        case PENDING_DISCARD:           return "Discard";
        case PENDING_SCRAP_TRADE_ROW:   return "Scrap Trade Row";
        case PENDING_SCRAP_HAND:        return "Scrap Hand";
        case PENDING_SCRAP_DISCARD:     return "Scrap Discard";
        case PENDING_SCRAP_HAND_DISCARD: return "Scrap Hand/Discard";
        case PENDING_TOP_DECK:          return "Top Deck";
        case PENDING_COPY_SHIP:         return "Copy Ship";
        case PENDING_DESTROY_BASE:      return "Destroy Base";
        case PENDING_UPGRADE:           return "Upgrade";
        default:                        return "Unknown";
    }
}
/* }}} */

/* {{{ game_has_pending_action
 * Returns true if there are pending actions waiting for resolution.
 */
bool game_has_pending_action(Game* game) {
    if (!game) {
        return false;
    }
    return game->pending_count > 0;
}
/* }}} */

/* {{{ game_get_pending_action
 * Returns the current pending action (first in queue), or NULL if none.
 */
PendingAction* game_get_pending_action(Game* game) {
    if (!game || game->pending_count == 0) {
        return NULL;
    }
    return &game->pending_actions[0];
}
/* }}} */

/* {{{ game_push_pending_action
 * Adds a pending action to the queue.
 */
void game_push_pending_action(Game* game, PendingAction* action) {
    if (!game || !action || game->pending_count >= MAX_PENDING_ACTIONS) {
        return;
    }

    game->pending_actions[game->pending_count] = *action;
    game->pending_count++;
}
/* }}} */

/* {{{ game_pop_pending_action
 * Removes the first pending action from the queue.
 */
void game_pop_pending_action(Game* game) {
    if (!game || game->pending_count == 0) {
        return;
    }

    /* Shift remaining actions down */
    for (int i = 0; i < game->pending_count - 1; i++) {
        game->pending_actions[i] = game->pending_actions[i + 1];
    }
    game->pending_count--;
}
/* }}} */

/* {{{ game_clear_pending_actions
 * Removes all pending actions.
 */
void game_clear_pending_actions(Game* game) {
    if (!game) {
        return;
    }
    game->pending_count = 0;
}
/* }}} */

/* {{{ game_request_discard
 * Creates a pending action for player to discard cards.
 * Used by opponent discard effects.
 */
void game_request_discard(Game* game, int player_id, int count) {
    if (!game || player_id < 1 || count <= 0) {
        return;
    }

    PendingAction action = {
        .type = PENDING_DISCARD,
        .player_id = player_id,
        .count = count,
        .min_count = count,  /* Must discard exactly this many */
        .resolved_count = 0,
        .optional = false,
        .source_card = NULL,
        .source_effect = NULL,
        .upgrade_type = 0,
        .upgrade_value = 0,
    };

    game_push_pending_action(game, &action);
}
/* }}} */

/* {{{ game_request_scrap_trade_row
 * Creates a pending action to scrap from trade row.
 */
void game_request_scrap_trade_row(Game* game, int player_id, int count) {
    if (!game || player_id < 1 || count <= 0) {
        return;
    }

    PendingAction action = {
        .type = PENDING_SCRAP_TRADE_ROW,
        .player_id = player_id,
        .count = count,
        .min_count = 0,  /* Optional - can skip */
        .resolved_count = 0,
        .optional = true,
        .source_card = NULL,
        .source_effect = NULL,
        .upgrade_type = 0,
        .upgrade_value = 0,
    };

    game_push_pending_action(game, &action);
}
/* }}} */

/* {{{ game_request_scrap_hand
 * Creates a pending action to scrap from hand.
 */
void game_request_scrap_hand(Game* game, int player_id, int count) {
    if (!game || player_id < 1 || count <= 0) {
        return;
    }

    PendingAction action = {
        .type = PENDING_SCRAP_HAND,
        .player_id = player_id,
        .count = count,
        .min_count = 0,  /* Optional - can skip */
        .resolved_count = 0,
        .optional = true,
        .source_card = NULL,
        .source_effect = NULL,
        .upgrade_type = 0,
        .upgrade_value = 0,
    };

    game_push_pending_action(game, &action);
}
/* }}} */

/* {{{ game_request_scrap_discard
 * Creates a pending action to scrap from discard pile.
 */
void game_request_scrap_discard(Game* game, int player_id, int count) {
    if (!game || player_id < 1 || count <= 0) {
        return;
    }

    PendingAction action = {
        .type = PENDING_SCRAP_DISCARD,
        .player_id = player_id,
        .count = count,
        .min_count = 0,  /* Optional - can skip */
        .resolved_count = 0,
        .optional = true,
        .source_card = NULL,
        .source_effect = NULL,
        .upgrade_type = 0,
        .upgrade_value = 0,
    };

    game_push_pending_action(game, &action);
}
/* }}} */

/* {{{ game_request_scrap_hand_discard
 * Creates a pending action to scrap from hand or discard.
 */
void game_request_scrap_hand_discard(Game* game, int player_id, int count) {
    if (!game || player_id < 1 || count <= 0) {
        return;
    }

    PendingAction action = {
        .type = PENDING_SCRAP_HAND_DISCARD,
        .player_id = player_id,
        .count = count,
        .min_count = 0,  /* Optional - can skip */
        .resolved_count = 0,
        .optional = true,
        .source_card = NULL,
        .source_effect = NULL,
        .upgrade_type = 0,
        .upgrade_value = 0,
    };

    game_push_pending_action(game, &action);
}
/* }}} */

/* {{{ game_request_top_deck
 * Creates a pending action to put a card from discard on top of deck.
 */
void game_request_top_deck(Game* game, int player_id, int count) {
    if (!game || player_id < 1 || count <= 0) {
        return;
    }

    PendingAction action = {
        .type = PENDING_TOP_DECK,
        .player_id = player_id,
        .count = count,
        .min_count = 0,  /* Optional - can skip */
        .resolved_count = 0,
        .optional = true,
        .source_card = NULL,
        .source_effect = NULL,
        .upgrade_type = 0,
        .upgrade_value = 0,
    };

    game_push_pending_action(game, &action);
}
/* }}} */

/* {{{ game_resolve_discard
 * Resolves a discard action by discarding the specified card from hand.
 * Returns true if successfully discarded.
 */
bool game_resolve_discard(Game* game, const char* card_instance_id) {
    if (!game || !card_instance_id) {
        return false;
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending || pending->type != PENDING_DISCARD) {
        return false;
    }

    /* Find the player */
    Player* player = NULL;
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i] && game->players[i]->id == pending->player_id) {
            player = game->players[i];
            break;
        }
    }
    if (!player || !player->deck) {
        return false;
    }

    /* Find card in hand */
    CardInstance* card = deck_find_in_hand(player->deck, card_instance_id);
    if (!card) {
        return false;
    }

    /* Discard the card */
    if (!deck_discard_from_hand(player->deck, card)) {
        return false;
    }

    /* Update resolved count */
    pending->resolved_count++;

    /* If all required discards done, pop the action */
    if (pending->resolved_count >= pending->count) {
        game_pop_pending_action(game);
    }

    return true;
}
/* }}} */

/* {{{ game_resolve_scrap_trade_row
 * Resolves a scrap trade row action.
 * Returns true if successfully scrapped.
 */
bool game_resolve_scrap_trade_row(Game* game, int slot) {
    if (!game || !game->trade_row) {
        return false;
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending || pending->type != PENDING_SCRAP_TRADE_ROW) {
        return false;
    }

    /* Validate slot */
    if (slot < 0 || slot >= TRADE_ROW_SLOTS) {
        return false;
    }

    /* Scrap the card (remove from game) */
    CardInstance* scrapped = trade_row_scrap(game->trade_row, slot);
    if (!scrapped) {
        return false;
    }

    /* Free the scrapped card */
    card_instance_free(scrapped);

    /* Update resolved count */
    pending->resolved_count++;

    /* If all scraps done, pop the action */
    if (pending->resolved_count >= pending->count) {
        game_pop_pending_action(game);
    }

    return true;
}
/* }}} */

/* {{{ game_resolve_scrap_hand
 * Resolves a scrap hand action.
 * Returns true if successfully scrapped.
 */
bool game_resolve_scrap_hand(Game* game, const char* card_instance_id) {
    if (!game || !card_instance_id) {
        return false;
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending || (pending->type != PENDING_SCRAP_HAND &&
                     pending->type != PENDING_SCRAP_HAND_DISCARD)) {
        return false;
    }

    /* Find the player */
    Player* player = NULL;
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i] && game->players[i]->id == pending->player_id) {
            player = game->players[i];
            break;
        }
    }
    if (!player || !player->deck) {
        return false;
    }

    /* Find card in hand */
    CardInstance* card = deck_find_in_hand(player->deck, card_instance_id);
    if (!card) {
        return false;
    }

    /* Scrap the card */
    CardInstance* scrapped = deck_scrap_from_hand(player->deck, card);
    if (!scrapped) {
        return false;
    }

    /* Scrapping decrements d10 */
    player_d10_decrement(player);

    /* Free the scrapped card */
    card_instance_free(scrapped);

    /* Update resolved count */
    pending->resolved_count++;

    /* If all scraps done, pop the action */
    if (pending->resolved_count >= pending->count) {
        game_pop_pending_action(game);
    }

    return true;
}
/* }}} */

/* {{{ game_resolve_scrap_discard
 * Resolves a scrap discard action.
 * Returns true if successfully scrapped.
 */
bool game_resolve_scrap_discard(Game* game, const char* card_instance_id) {
    if (!game || !card_instance_id) {
        return false;
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending || (pending->type != PENDING_SCRAP_DISCARD &&
                     pending->type != PENDING_SCRAP_HAND_DISCARD)) {
        return false;
    }

    /* Find the player */
    Player* player = NULL;
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i] && game->players[i]->id == pending->player_id) {
            player = game->players[i];
            break;
        }
    }
    if (!player || !player->deck) {
        return false;
    }

    /* Find card in discard */
    CardInstance* card = deck_find_in_discard(player->deck, card_instance_id);
    if (!card) {
        return false;
    }

    /* Scrap the card */
    CardInstance* scrapped = deck_scrap_from_discard(player->deck, card);
    if (!scrapped) {
        return false;
    }

    /* Scrapping decrements d10 */
    player_d10_decrement(player);

    /* Free the scrapped card */
    card_instance_free(scrapped);

    /* Update resolved count */
    pending->resolved_count++;

    /* If all scraps done, pop the action */
    if (pending->resolved_count >= pending->count) {
        game_pop_pending_action(game);
    }

    return true;
}
/* }}} */

/* {{{ game_resolve_top_deck
 * Resolves a top deck action - puts card from discard on top of deck.
 * Returns true if successfully moved.
 */
bool game_resolve_top_deck(Game* game, const char* card_instance_id) {
    if (!game || !card_instance_id) {
        return false;
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending || pending->type != PENDING_TOP_DECK) {
        return false;
    }

    /* Find the player */
    Player* player = NULL;
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i] && game->players[i]->id == pending->player_id) {
            player = game->players[i];
            break;
        }
    }
    if (!player || !player->deck) {
        return false;
    }

    /* Find card in discard */
    CardInstance* card = deck_find_in_discard(player->deck, card_instance_id);
    if (!card) {
        return false;
    }

    /* Remove from discard and put on top of deck */
    CardInstance* removed = deck_scrap_from_discard(player->deck, card);
    if (!removed) {
        return false;
    }

    /* Put on top of draw pile */
    if (!deck_put_on_top(player->deck, removed)) {
        /* If failed, put back in discard */
        deck_add_to_discard(player->deck, removed);
        return false;
    }

    /* Update resolved count */
    pending->resolved_count++;

    /* If all done, pop the action */
    if (pending->resolved_count >= pending->count) {
        game_pop_pending_action(game);
    }

    return true;
}
/* }}} */

/* {{{ game_skip_pending_action
 * Skips the current pending action if it's optional.
 * Returns true if successfully skipped.
 */
bool game_skip_pending_action(Game* game) {
    if (!game) {
        return false;
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending) {
        return false;
    }

    /* Check if action can be skipped */
    if (!pending->optional && pending->resolved_count < pending->min_count) {
        return false;  /* Cannot skip - must complete minimum */
    }

    /* Remove the pending action */
    game_pop_pending_action(game);
    return true;
}
/* }}} */

/* {{{ game_request_copy_ship
 * Creates a pending action to copy another ship's effects.
 */
void game_request_copy_ship(Game* game, int player_id) {
    if (!game || player_id < 1) {
        return;
    }

    PendingAction action = {
        .type = PENDING_COPY_SHIP,
        .player_id = player_id,
        .count = 1,
        .min_count = 0,  /* Optional - can skip */
        .resolved_count = 0,
        .optional = true,
        .source_card = NULL,
        .source_effect = NULL,
        .upgrade_type = 0,
        .upgrade_value = 0,
    };

    game_push_pending_action(game, &action);
}
/* }}} */

/* {{{ game_request_destroy_base
 * Creates a pending action to destroy an opponent's base.
 */
void game_request_destroy_base(Game* game, int player_id) {
    if (!game || player_id < 1) {
        return;
    }

    PendingAction action = {
        .type = PENDING_DESTROY_BASE,
        .player_id = player_id,
        .count = 1,
        .min_count = 0,  /* Optional - can skip if no bases */
        .resolved_count = 0,
        .optional = true,
        .source_card = NULL,
        .source_effect = NULL,
        .upgrade_type = 0,
        .upgrade_value = 0,
    };

    game_push_pending_action(game, &action);
}
/* }}} */

/* {{{ game_resolve_copy_ship
 * Resolves a copy ship action by executing the target ship's effects.
 * Target can be from player's played area or trade row.
 * Returns true if successfully copied.
 */
bool game_resolve_copy_ship(Game* game, const char* card_instance_id) {
    if (!game || !card_instance_id) {
        return false;
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending || pending->type != PENDING_COPY_SHIP) {
        return false;
    }

    /* Find the player */
    Player* player = NULL;
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i] && game->players[i]->id == pending->player_id) {
            player = game->players[i];
            break;
        }
    }
    if (!player || !player->deck) {
        return false;
    }

    /* Find target card - check player's played area first */
    CardInstance* target = NULL;
    for (int i = 0; i < player->deck->played_count; i++) {
        if (player->deck->played[i] &&
            strcmp(player->deck->played[i]->instance_id, card_instance_id) == 0) {
            target = player->deck->played[i];
            break;
        }
    }

    /* If not in played, check trade row */
    if (!target && game->trade_row) {
        for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
            if (game->trade_row->slots[i] &&
                strcmp(game->trade_row->slots[i]->instance_id, card_instance_id) == 0) {
                target = game->trade_row->slots[i];
                break;
            }
        }
    }

    if (!target || !target->type) {
        return false;
    }

    /* Only ships can be copied */
    if (target->type->kind != CARD_KIND_SHIP) {
        return false;
    }

    /* Include effects header for effects_execute_card */
    /* Note: effects_execute_card defined in 07-effects.h */
    extern void effects_execute_card(Game*, Player*, CardInstance*);

    /* Execute target's effects as if played */
    effects_execute_card(game, player, target);

    /* Pop the pending action */
    game_pop_pending_action(game);
    return true;
}
/* }}} */

/* {{{ game_resolve_destroy_base
 * Resolves a destroy base action by removing opponent's base.
 * Returns true if successfully destroyed.
 */
bool game_resolve_destroy_base(Game* game, const char* card_instance_id) {
    if (!game || !card_instance_id) {
        return false;
    }

    PendingAction* pending = game_get_pending_action(game);
    if (!pending || pending->type != PENDING_DESTROY_BASE) {
        return false;
    }

    /* Find the player (the one who triggered the effect) */
    Player* player = NULL;
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i] && game->players[i]->id == pending->player_id) {
            player = game->players[i];
            break;
        }
    }
    if (!player) {
        return false;
    }

    /* Get opponent - check all other players for the target base */
    CardInstance* target = NULL;
    Player* target_owner = NULL;

    for (int p = 0; p < game->player_count; p++) {
        if (game->players[p] == player) {
            continue;  /* Skip the active player */
        }

        Player* opponent = game->players[p];
        if (!opponent || !opponent->deck) {
            continue;
        }

        /* Check frontier bases */
        for (int i = 0; i < opponent->deck->frontier_base_count; i++) {
            if (opponent->deck->frontier_bases[i] &&
                strcmp(opponent->deck->frontier_bases[i]->instance_id, card_instance_id) == 0) {
                target = opponent->deck->frontier_bases[i];
                target_owner = opponent;
                break;
            }
        }
        if (target) break;

        /* Check interior bases */
        for (int i = 0; i < opponent->deck->interior_base_count; i++) {
            if (opponent->deck->interior_bases[i] &&
                strcmp(opponent->deck->interior_bases[i]->instance_id, card_instance_id) == 0) {
                target = opponent->deck->interior_bases[i];
                target_owner = opponent;
                break;
            }
        }
        if (target) break;
    }

    if (!target || !target_owner) {
        return false;
    }

    /* Remove base from owner's deck */
    CardInstance* removed = deck_remove_base(target_owner->deck, target);
    if (!removed) {
        return false;
    }

    /* Free the destroyed base (it's removed from game, not scrapped) */
    card_instance_free(removed);

    /* Pop the pending action */
    game_pop_pending_action(game);
    return true;
}
/* }}} */

/* ========================================================================== */
/*                        Purchase with Effect Context                        */
/* ========================================================================== */

/* Forward declare effects context functions */
typedef struct {
    bool next_ship_free;
    int free_ship_max_cost;
    bool next_ship_to_top;
    int pending_draws;
} EffectContext;
extern EffectContext* effects_get_context(Player* player);

/* {{{ game_buy_card
 * Purchases a card from the trade row, respecting effect context flags.
 * Handles next_ship_free (reduced cost) and next_ship_to_top (deck placement).
 * Returns the purchased card instance, or NULL on failure.
 */
CardInstance* game_buy_card(Game* game, int slot) {
    if (!game || !game->trade_row || slot < 0 || slot >= TRADE_ROW_SLOTS) {
        return NULL;
    }

    Player* player = game_get_active_player(game);
    if (!player) {
        return NULL;
    }

    CardInstance* target = trade_row_get_slot(game->trade_row, slot);
    if (!target || !target->type) {
        return NULL;
    }

    int cost = target->type->cost;
    EffectContext* ctx = effects_get_context(player);

    /* Check if next ship is free */
    bool is_free = false;
    if (ctx && ctx->next_ship_free) {
        /* Check cost limit (0 = any cost) */
        if (ctx->free_ship_max_cost == 0 || cost <= ctx->free_ship_max_cost) {
            is_free = true;
        }
    }

    /* Check if player can afford */
    if (!is_free && player->trade < cost) {
        return NULL;  /* Can't afford */
    }

    /* Deduct trade (unless free) */
    if (!is_free) {
        player->trade -= cost;
    }

    /* Increment d10 (buy momentum) */
    player_d10_increment(player);

    /* Get the card from trade row */
    CardInstance* card = game->trade_row->slots[slot];
    game->trade_row->slots[slot] = NULL;

    /* Refill the slot */
    trade_row_fill_slots(game->trade_row);

    /* Check if card goes to top of deck or discard */
    bool to_top = ctx && ctx->next_ship_to_top;

    if (to_top) {
        deck_put_on_top(player->deck, card);
    } else {
        deck_add_to_discard(player->deck, card);
    }

    /* Reset effect flags after use */
    if (ctx) {
        if (is_free) {
            ctx->next_ship_free = false;
            ctx->free_ship_max_cost = 0;
        }
        if (to_top) {
            ctx->next_ship_to_top = false;
        }
    }

    return card;
}
/* }}} */

/* {{{ game_buy_explorer
 * Purchases an explorer, respecting effect context flags.
 * Returns the purchased explorer instance, or NULL on failure.
 */
CardInstance* game_buy_explorer(Game* game) {
    if (!game || !game->trade_row || !game->trade_row->explorer_type) {
        return NULL;
    }

    Player* player = game_get_active_player(game);
    if (!player) {
        return NULL;
    }

    int cost = EXPLORER_COST;
    EffectContext* ctx = effects_get_context(player);

    /* Check if next ship is free */
    bool is_free = false;
    if (ctx && ctx->next_ship_free) {
        /* Check cost limit (0 = any cost) */
        if (ctx->free_ship_max_cost == 0 || cost <= ctx->free_ship_max_cost) {
            is_free = true;
        }
    }

    /* Check if player can afford */
    if (!is_free && player->trade < cost) {
        return NULL;  /* Can't afford */
    }

    /* Deduct trade (unless free) */
    if (!is_free) {
        player->trade -= cost;
    }

    /* Increment d10 (buy momentum) */
    player_d10_increment(player);

    /* Create explorer instance */
    CardInstance* card = card_instance_create(game->trade_row->explorer_type);
    if (!card) {
        return NULL;
    }

    /* Check if card goes to top of deck or discard */
    bool to_top = ctx && ctx->next_ship_to_top;

    if (to_top) {
        deck_put_on_top(player->deck, card);
    } else {
        deck_add_to_discard(player->deck, card);
    }

    /* Reset effect flags after use */
    if (ctx) {
        if (is_free) {
            ctx->next_ship_free = false;
            ctx->free_ship_max_cost = 0;
        }
        if (to_top) {
            ctx->next_ship_to_top = false;
        }
    }

    return card;
}
/* }}} */
