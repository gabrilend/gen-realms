/* 05-game.c - Game state and turn loop implementation
 *
 * Manages the complete game lifecycle from creation through game end.
 * Handles turn phases, player actions, and win condition detection.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "05-game.h"
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

    /* Create trade row (if we have trade cards) */
    if (game->card_types && game->card_type_count > 0) {
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

    /* Enter draw order selection phase */
    game->phase = PHASE_DRAW_ORDER;
}
/* }}} */

/* {{{ game_submit_draw_order
 * Player submits their preferred draw order.
 * Draws cards in that order, then transitions to main phase.
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

    /* Transition to main phase */
    game->phase = PHASE_MAIN;
}
/* }}} */

/* {{{ game_skip_draw_order
 * Skips draw order selection, draws cards in default order.
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
