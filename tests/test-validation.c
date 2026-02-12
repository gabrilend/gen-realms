/* test-validation.c - Input Validation Tests
 *
 * Tests server-side validation of client actions. Verifies that invalid
 * actions are rejected with appropriate error messages.
 *
 * Run with: make test-validation
 */

#define _POSIX_C_SOURCE 200809L  /* For strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../src/net/08-validation.h"
#include "../src/core/05-game.h"

/* ========================================================================== */
/*                              Test Utilities                                 */
/* ========================================================================== */

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("  [TEST] %s... ", name); \
        tests_run++; \
    } while (0)

#define PASS() \
    do { \
        printf("\033[32mPASS\033[0m\n"); \
        tests_passed++; \
    } while (0)

#define FAIL(msg) \
    do { \
        printf("\033[31mFAIL\033[0m: %s\n", msg); \
    } while (0)

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            FAIL(msg); \
            return; \
        } \
    } while (0)

#define ASSERT_VALID(result) \
    ASSERT((result).valid, (result).error_message[0] ? (result).error_message : "Expected valid")

#define ASSERT_INVALID(result, expected_error) \
    do { \
        ASSERT(!(result).valid, "Expected invalid"); \
        ASSERT((result).error == (expected_error), "Wrong error code"); \
    } while (0)

/* ========================================================================== */
/*                              Test Setup                                     */
/* ========================================================================== */

/* {{{ Helper to create minimal card types */
static CardType* create_test_card_type(const char* id, const char* name, int cost) {
    CardType* type = card_type_create(id, name, cost, FACTION_NEUTRAL, CARD_KIND_SHIP);
    return type;
}
/* }}} */

/* {{{ Helper to create a test game with cards in hands */
static Game* create_test_game(void) {
    /* Create card types */
    CardType* scout = create_test_card_type("scout", "Scout", 0);
    CardType* viper = create_test_card_type("viper", "Viper", 0);
    CardType* explorer = create_test_card_type("explorer", "Explorer", 2);
    CardType* cutter = create_test_card_type("cutter", "Cutter", 3);

    if (!scout || !viper || !explorer || !cutter) {
        if (scout) card_type_free(scout);
        if (viper) card_type_free(viper);
        if (explorer) card_type_free(explorer);
        if (cutter) card_type_free(cutter);
        return NULL;
    }

    /* Create game */
    Game* game = game_create(2);
    if (!game) {
        card_type_free(scout);
        card_type_free(viper);
        card_type_free(explorer);
        card_type_free(cutter);
        return NULL;
    }

    /* Set starting types */
    game_set_starting_types(game, scout, viper, explorer);

    /* Create card type array (game takes ownership) */
    CardType** types = malloc(sizeof(CardType*) * 4);
    if (!types) {
        game_free(game);
        return NULL;
    }
    types[0] = scout;
    types[1] = viper;
    types[2] = explorer;
    types[3] = cutter;
    game_set_card_types(game, types, 4);

    /* Add players */
    game_add_player(game, "Alice");
    game_add_player(game, "Bob");

    /* Start game */
    if (!game_start(game)) {
        game_free(game);
        return NULL;
    }

    /* Draw starting hands (game_start enters PHASE_DRAW_ORDER) */
    game_skip_draw_order(game);

    return game;
}
/* }}} */

/* ========================================================================== */
/*                         Turn Ownership Tests                                */
/* ========================================================================== */

/* {{{ test_validate_is_player_turn_correct */
static void test_validate_is_player_turn_correct(void) {
    TEST("validate_is_player_turn accepts active player");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    /* Player 0 is active */
    ValidationResult result = validate_is_player_turn(game, 0);
    ASSERT_VALID(result);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_is_player_turn_wrong */
static void test_validate_is_player_turn_wrong(void) {
    TEST("validate_is_player_turn rejects non-active player");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    /* Player 1 is not active */
    ValidationResult result = validate_is_player_turn(game, 1);
    ASSERT_INVALID(result, PROTOCOL_ERROR_NOT_YOUR_TURN);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_is_player_turn_invalid_id */
static void test_validate_is_player_turn_invalid_id(void) {
    TEST("validate_is_player_turn rejects invalid player ID");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    ValidationResult result = validate_is_player_turn(game, 99);
    ASSERT_INVALID(result, PROTOCOL_ERROR_NOT_IN_GAME);

    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                           Phase Tests                                       */
/* ========================================================================== */

/* {{{ test_validate_phase_correct */
static void test_validate_phase_correct(void) {
    TEST("validate_phase accepts correct phase");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");
    ASSERT(game->phase == PHASE_MAIN, "Expected main phase");

    ValidationResult result = validate_phase(game, PHASE_MAIN);
    ASSERT_VALID(result);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_phase_wrong */
static void test_validate_phase_wrong(void) {
    TEST("validate_phase rejects wrong phase");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    ValidationResult result = validate_phase(game, PHASE_DRAW_ORDER);
    ASSERT_INVALID(result, PROTOCOL_ERROR_INVALID_PHASE);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_game_in_progress */
static void test_validate_game_in_progress(void) {
    TEST("validate_game_in_progress accepts active game");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    ValidationResult result = validate_game_in_progress(game);
    ASSERT_VALID(result);

    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                         Play Card Tests                                     */
/* ========================================================================== */

/* {{{ test_validate_play_card_valid */
static void test_validate_play_card_valid(void) {
    TEST("validate_play_card accepts valid play");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    ASSERT(player != NULL && player->deck != NULL, "No player");
    ASSERT(player->deck->hand_count > 0, "Empty hand");

    const char* card_id = player->deck->hand[0]->instance_id;
    ASSERT(card_id != NULL, "No card ID");

    ValidationResult result = validate_play_card(game, 0, card_id);
    ASSERT_VALID(result);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_play_card_wrong_turn */
static void test_validate_play_card_wrong_turn(void) {
    TEST("validate_play_card rejects wrong turn");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[1];
    ASSERT(player != NULL && player->deck != NULL, "No player");

    /* Draw cards for player 1 so they have something in hand */
    player_draw_cards(player, 5);
    ASSERT(player->deck->hand_count > 0, "Empty hand");

    const char* card_id = player->deck->hand[0]->instance_id;

    /* Player 1 trying to play on player 0's turn */
    ValidationResult result = validate_play_card(game, 1, card_id);
    ASSERT_INVALID(result, PROTOCOL_ERROR_NOT_YOUR_TURN);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_play_card_not_in_hand */
static void test_validate_play_card_not_in_hand(void) {
    TEST("validate_play_card rejects card not in hand");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    ValidationResult result = validate_play_card(game, 0, "nonexistent_card");
    ASSERT_INVALID(result, PROTOCOL_ERROR_CARD_NOT_IN_HAND);

    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                         Buy Card Tests                                      */
/* ========================================================================== */

/* {{{ test_validate_buy_card_invalid_slot */
static void test_validate_buy_card_invalid_slot(void) {
    TEST("validate_buy_card rejects invalid slot");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    ValidationResult result = validate_buy_card(game, 0, 99);
    ASSERT_INVALID(result, PROTOCOL_ERROR_INVALID_SLOT);

    result = validate_buy_card(game, 0, -1);
    ASSERT_INVALID(result, PROTOCOL_ERROR_INVALID_SLOT);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_buy_card_insufficient_trade */
static void test_validate_buy_card_insufficient_trade(void) {
    TEST("validate_buy_card rejects insufficient trade");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    /* Player starts with 0 trade, trade row cards cost > 0 */
    Player* player = game->players[0];
    ASSERT(player != NULL, "No player");
    ASSERT(player->trade == 0, "Expected 0 trade");

    /* Assuming slot 0 has a card with cost > 0 */
    if (game->trade_row && trade_row_get_slot(game->trade_row, 0)) {
        ValidationResult result = validate_buy_card(game, 0, 0);
        ASSERT_INVALID(result, PROTOCOL_ERROR_INSUFFICIENT_TRADE);
    }

    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                         Attack Tests                                        */
/* ========================================================================== */

/* {{{ test_validate_attack_player_no_combat */
static void test_validate_attack_player_no_combat(void) {
    TEST("validate_attack_player rejects no combat");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    ASSERT(player != NULL, "No player");
    ASSERT(player->combat == 0, "Expected 0 combat");

    ValidationResult result = validate_attack_player(game, 0, 1, 5);
    ASSERT_INVALID(result, PROTOCOL_ERROR_INSUFFICIENT_COMBAT);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_attack_player_self */
static void test_validate_attack_player_self(void) {
    TEST("validate_attack_player rejects self-attack");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    /* Give player some combat */
    game->players[0]->combat = 10;

    ValidationResult result = validate_attack_player(game, 0, 0, 5);
    ASSERT_INVALID(result, PROTOCOL_ERROR_INVALID_TARGET);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_attack_player_invalid_target */
static void test_validate_attack_player_invalid_target(void) {
    TEST("validate_attack_player rejects invalid target");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    /* Give player some combat */
    game->players[0]->combat = 10;

    ValidationResult result = validate_attack_player(game, 0, 99, 5);
    ASSERT_INVALID(result, PROTOCOL_ERROR_INVALID_TARGET);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_attack_player_valid */
static void test_validate_attack_player_valid(void) {
    TEST("validate_attack_player accepts valid attack");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    /* Give player some combat */
    game->players[0]->combat = 10;

    ValidationResult result = validate_attack_player(game, 0, 1, 5);
    ASSERT_VALID(result);

    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                         End Turn Tests                                      */
/* ========================================================================== */

/* {{{ test_validate_end_turn_valid */
static void test_validate_end_turn_valid(void) {
    TEST("validate_end_turn accepts valid end turn");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    ValidationResult result = validate_end_turn(game, 0);
    ASSERT_VALID(result);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_end_turn_wrong_player */
static void test_validate_end_turn_wrong_player(void) {
    TEST("validate_end_turn rejects wrong player");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    ValidationResult result = validate_end_turn(game, 1);
    ASSERT_INVALID(result, PROTOCOL_ERROR_NOT_YOUR_TURN);

    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                       Action Dispatch Tests                                 */
/* ========================================================================== */

/* {{{ test_validate_action_play_card */
static void test_validate_action_play_card(void) {
    TEST("validate_action dispatches play_card");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    ASSERT(player != NULL && player->deck != NULL, "No player");
    ASSERT(player->deck->hand_count > 0, "Empty hand");

    Action action;
    action.type = ACTION_PLAY_CARD;
    action.card_instance_id = strdup(player->deck->hand[0]->instance_id);

    ValidationResult result = validate_action(game, 0, &action);
    ASSERT_VALID(result);

    free(action.card_instance_id);
    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_action_end_turn */
static void test_validate_action_end_turn(void) {
    TEST("validate_action dispatches end_turn");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Action action;
    action.type = ACTION_END_TURN;

    ValidationResult result = validate_action(game, 0, &action);
    ASSERT_VALID(result);

    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_validate_action_null */
static void test_validate_action_null(void) {
    TEST("validate_action rejects null action");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    ValidationResult result = validate_action(game, 0, NULL);
    ASSERT_INVALID(result, PROTOCOL_ERROR_INVALID_ACTION);

    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                       Draw Order Tests                                      */
/* ========================================================================== */

/* {{{ test_validate_draw_order_wrong_phase */
static void test_validate_draw_order_wrong_phase(void) {
    TEST("validate_draw_order rejects wrong phase");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");
    ASSERT(game->phase == PHASE_MAIN, "Expected main phase");

    int order[] = {0, 1, 2, 3, 4};
    ValidationResult result = validate_draw_order(game, 0, order, 5);
    ASSERT_INVALID(result, PROTOCOL_ERROR_INVALID_PHASE);

    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Main                                           */
/* ========================================================================== */

int main(void) {
    printf("\n");
    printf("==========================================\n");
    printf("  Input Validation Tests\n");
    printf("==========================================\n\n");

    /* Turn ownership tests */
    printf("Turn Ownership:\n");
    test_validate_is_player_turn_correct();
    test_validate_is_player_turn_wrong();
    test_validate_is_player_turn_invalid_id();
    printf("\n");

    /* Phase tests */
    printf("Phase Validation:\n");
    test_validate_phase_correct();
    test_validate_phase_wrong();
    test_validate_game_in_progress();
    printf("\n");

    /* Play card tests */
    printf("Play Card:\n");
    test_validate_play_card_valid();
    test_validate_play_card_wrong_turn();
    test_validate_play_card_not_in_hand();
    printf("\n");

    /* Buy card tests */
    printf("Buy Card:\n");
    test_validate_buy_card_invalid_slot();
    test_validate_buy_card_insufficient_trade();
    printf("\n");

    /* Attack tests */
    printf("Attack:\n");
    test_validate_attack_player_no_combat();
    test_validate_attack_player_self();
    test_validate_attack_player_invalid_target();
    test_validate_attack_player_valid();
    printf("\n");

    /* End turn tests */
    printf("End Turn:\n");
    test_validate_end_turn_valid();
    test_validate_end_turn_wrong_player();
    printf("\n");

    /* Action dispatch tests */
    printf("Action Dispatch:\n");
    test_validate_action_play_card();
    test_validate_action_end_turn();
    test_validate_action_null();
    printf("\n");

    /* Draw order tests */
    printf("Draw Order:\n");
    test_validate_draw_order_wrong_phase();
    printf("\n");

    /* Summary */
    printf("==========================================\n");
    printf("  Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("==========================================\n\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
