/* test-hidden-info.c - Hidden Information Handling Tests
 *
 * Tests that game state serialization properly hides opponent hand contents
 * while exposing all public information.
 *
 * Run with: make test-hidden-info
 */

#define _POSIX_C_SOURCE 200809L  /* For strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../src/core/09-serialize.h"
#include "../libs/cJSON.h"

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

/* ========================================================================== */
/*                              Test Setup                                     */
/* ========================================================================== */

/* {{{ Helper to create minimal card types */
static CardType* create_test_card_type(const char* id, const char* name) {
    CardType* type = card_type_create(id, name, 2, FACTION_NEUTRAL, CARD_KIND_SHIP);
    return type;
}
/* }}} */

/* {{{ Helper to create a test game with cards in hands */
static Game* create_test_game(void) {
    /* Create card types */
    CardType* scout = create_test_card_type("scout", "Scout");
    CardType* viper = create_test_card_type("viper", "Viper");
    CardType* explorer = create_test_card_type("explorer", "Explorer");

    if (!scout || !viper || !explorer) {
        if (scout) card_type_free(scout);
        if (viper) card_type_free(viper);
        if (explorer) card_type_free(explorer);
        return NULL;
    }

    /* Create game */
    Game* game = game_create(2);
    if (!game) {
        card_type_free(scout);
        card_type_free(viper);
        card_type_free(explorer);
        return NULL;
    }

    /* Set starting types */
    game_set_starting_types(game, scout, viper, explorer);

    /* Create card type array (game takes ownership) */
    CardType** types = malloc(sizeof(CardType*) * 3);
    if (!types) {
        game_free(game);
        return NULL;
    }
    types[0] = scout;
    types[1] = viper;
    types[2] = explorer;
    game_set_card_types(game, types, 3);

    /* Add players */
    game_add_player(game, "Alice");
    game_add_player(game, "Bob");

    /* Start game */
    if (!game_start(game)) {
        game_free(game);
        return NULL;
    }

    /* Draw starting hands (game_start enters PHASE_DRAW_ORDER, need to skip to draw) */
    game_skip_draw_order(game);

    return game;
}
/* }}} */

/* ========================================================================== */
/*                              Player View Tests                              */
/* ========================================================================== */

/* {{{ test_player_private_has_hand */
static void test_player_private_has_hand(void) {
    TEST("Private player view includes hand contents");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    ASSERT(player != NULL, "No player 0");
    ASSERT(player->deck != NULL, "Player has no deck");
    ASSERT(player->deck->hand_count > 0, "Player has empty hand");

    cJSON* json = serialize_player_private(player);
    ASSERT(json != NULL, "Failed to serialize");

    /* Should have "hand" array with card contents */
    cJSON* hand = cJSON_GetObjectItem(json, "hand");
    ASSERT(hand != NULL, "Missing 'hand' field");
    ASSERT(cJSON_IsArray(hand), "'hand' should be array");
    ASSERT(cJSON_GetArraySize(hand) == player->deck->hand_count, "Hand size mismatch");

    /* First card should have instance_id */
    cJSON* first_card = cJSON_GetArrayItem(hand, 0);
    ASSERT(first_card != NULL, "Hand array empty");
    cJSON* instance_id = cJSON_GetObjectItem(first_card, "instance_id");
    ASSERT(instance_id != NULL, "Card missing instance_id");
    ASSERT(cJSON_IsString(instance_id), "instance_id should be string");

    cJSON_Delete(json);
    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_player_public_hides_hand */
static void test_player_public_hides_hand(void) {
    TEST("Public player view hides hand contents");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    ASSERT(player != NULL, "No player 0");

    cJSON* json = serialize_player_public(player);
    ASSERT(json != NULL, "Failed to serialize");

    /* Should NOT have "hand" array */
    cJSON* hand = cJSON_GetObjectItem(json, "hand");
    ASSERT(hand == NULL, "'hand' field should NOT be present");

    /* Should have "hand_count" */
    cJSON* hand_count = cJSON_GetObjectItem(json, "hand_count");
    ASSERT(hand_count != NULL, "Missing 'hand_count' field");
    ASSERT(cJSON_IsNumber(hand_count), "'hand_count' should be number");
    ASSERT((int)hand_count->valuedouble == player->deck->hand_count, "Wrong hand count");

    cJSON_Delete(json);
    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_player_public_has_d10_d4 */
static void test_player_public_has_d10_d4(void) {
    TEST("Public player view includes d10/d4");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    cJSON* json = serialize_player_public(player);
    ASSERT(json != NULL, "Failed to serialize");

    cJSON* d10 = cJSON_GetObjectItem(json, "d10");
    cJSON* d4 = cJSON_GetObjectItem(json, "d4");

    ASSERT(d10 != NULL, "Missing 'd10' field");
    ASSERT(d4 != NULL, "Missing 'd4' field");
    ASSERT(cJSON_IsNumber(d10), "'d10' should be number");
    ASSERT(cJSON_IsNumber(d4), "'d4' should be number");

    cJSON_Delete(json);
    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_player_public_has_discard */
static void test_player_public_has_discard(void) {
    TEST("Public player view includes discard pile");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    cJSON* json = serialize_player_public(player);
    ASSERT(json != NULL, "Failed to serialize");

    /* Should have "discard" array */
    cJSON* discard = cJSON_GetObjectItem(json, "discard");
    ASSERT(discard != NULL, "Missing 'discard' field");
    ASSERT(cJSON_IsArray(discard), "'discard' should be array");

    cJSON_Delete(json);
    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Game View Tests                                */
/* ========================================================================== */

/* {{{ test_game_for_player_hides_opponent_hand */
static void test_game_for_player_hides_opponent_hand(void) {
    TEST("Game serialization hides opponent hand");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    /* Serialize for player 0 */
    cJSON* json = serialize_game_for_player(game, 0);
    ASSERT(json != NULL, "Failed to serialize");

    /* "you" should have hand contents */
    cJSON* you = cJSON_GetObjectItem(json, "you");
    ASSERT(you != NULL, "Missing 'you' field");
    cJSON* your_hand = cJSON_GetObjectItem(you, "hand");
    ASSERT(your_hand != NULL, "'you' should have hand");
    ASSERT(cJSON_IsArray(your_hand), "your hand should be array");

    /* "opponents" should NOT have hand contents */
    cJSON* opponents = cJSON_GetObjectItem(json, "opponents");
    ASSERT(opponents != NULL, "Missing 'opponents' field");
    ASSERT(cJSON_IsArray(opponents), "'opponents' should be array");
    ASSERT(cJSON_GetArraySize(opponents) == 1, "Should have 1 opponent");

    cJSON* opponent = cJSON_GetArrayItem(opponents, 0);
    ASSERT(opponent != NULL, "Opponent array empty");

    cJSON* opp_hand = cJSON_GetObjectItem(opponent, "hand");
    ASSERT(opp_hand == NULL, "Opponent should NOT have 'hand' field");

    cJSON* opp_hand_count = cJSON_GetObjectItem(opponent, "hand_count");
    ASSERT(opp_hand_count != NULL, "Opponent should have 'hand_count'");

    cJSON_Delete(json);
    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_game_for_spectator_shows_all_hands */
static void test_game_for_spectator_shows_all_hands(void) {
    TEST("Spectator view shows all hands");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    cJSON* json = serialize_game_for_spectator(game);
    ASSERT(json != NULL, "Failed to serialize");

    /* Should have is_spectator flag */
    cJSON* is_spectator = cJSON_GetObjectItem(json, "is_spectator");
    ASSERT(is_spectator != NULL, "Missing 'is_spectator' flag");
    ASSERT(cJSON_IsTrue(is_spectator), "'is_spectator' should be true");

    /* All players should have hand contents */
    cJSON* players = cJSON_GetObjectItem(json, "players");
    ASSERT(players != NULL, "Missing 'players' field");
    ASSERT(cJSON_IsArray(players), "'players' should be array");
    ASSERT(cJSON_GetArraySize(players) == 2, "Should have 2 players");

    for (int i = 0; i < 2; i++) {
        cJSON* player = cJSON_GetArrayItem(players, i);
        ASSERT(player != NULL, "Player array has NULL");

        cJSON* hand = cJSON_GetObjectItem(player, "hand");
        ASSERT(hand != NULL, "Player missing 'hand' in spectator view");
        ASSERT(cJSON_IsArray(hand), "hand should be array");
    }

    cJSON_Delete(json);
    game_free(game);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Information Leakage Tests                      */
/* ========================================================================== */

/* {{{ test_no_hand_leak_in_opponent_json */
static void test_no_hand_leak_in_opponent_json(void) {
    TEST("No hand info leaks in opponent JSON string");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    /* Get player 1's hand card IDs for checking */
    Player* opponent = game->players[1];
    ASSERT(opponent != NULL && opponent->deck != NULL, "No opponent");

    /* Draw cards for opponent (normally happens on their turn) */
    player_draw_cards(opponent, 5);
    ASSERT(opponent->deck->hand_count > 0, "Opponent has no hand");

    char* opponent_card_id = NULL;
    if (opponent->deck->hand[0] && opponent->deck->hand[0]->instance_id) {
        opponent_card_id = strdup(opponent->deck->hand[0]->instance_id);
    }
    ASSERT(opponent_card_id != NULL, "No opponent card ID");

    /* Serialize game for player 0 */
    char* json_str = game_state_to_string(game, 0);
    ASSERT(json_str != NULL, "Failed to serialize");

    /* The opponent's card instance_id should NOT appear in the JSON */
    char* found = strstr(json_str, opponent_card_id);
    bool leaked = (found != NULL);

    /* But first check if it's OUR card (we might have same card type) */
    /* Actually, instance IDs are unique, so if found, it's a leak */

    free(json_str);
    free(opponent_card_id);
    game_free(game);

    ASSERT(!leaked, "Opponent card instance_id leaked in JSON");
    PASS();
}
/* }}} */

/* {{{ test_view_perspective_opponent */
static void test_view_perspective_opponent(void) {
    TEST("ViewPerspective VIEW_OPPONENT hides hand");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    cJSON* json = serialize_player_for_view(player, VIEW_OPPONENT);
    ASSERT(json != NULL, "Failed to serialize");

    cJSON* hand = cJSON_GetObjectItem(json, "hand");
    ASSERT(hand == NULL, "VIEW_OPPONENT should hide hand");

    cJSON_Delete(json);
    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_view_perspective_self */
static void test_view_perspective_self(void) {
    TEST("ViewPerspective VIEW_SELF shows hand");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    cJSON* json = serialize_player_for_view(player, VIEW_SELF);
    ASSERT(json != NULL, "Failed to serialize");

    cJSON* hand = cJSON_GetObjectItem(json, "hand");
    ASSERT(hand != NULL, "VIEW_SELF should show hand");
    ASSERT(cJSON_IsArray(hand), "hand should be array");

    cJSON_Delete(json);
    game_free(game);
    PASS();
}
/* }}} */

/* {{{ test_view_perspective_spectator */
static void test_view_perspective_spectator(void) {
    TEST("ViewPerspective VIEW_SPECTATOR shows hand");

    Game* game = create_test_game();
    ASSERT(game != NULL, "Failed to create test game");

    Player* player = game->players[0];
    cJSON* json = serialize_player_for_view(player, VIEW_SPECTATOR);
    ASSERT(json != NULL, "Failed to serialize");

    cJSON* hand = cJSON_GetObjectItem(json, "hand");
    ASSERT(hand != NULL, "VIEW_SPECTATOR should show hand");
    ASSERT(cJSON_IsArray(hand), "hand should be array");

    cJSON_Delete(json);
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
    printf("  Hidden Information Tests\n");
    printf("==========================================\n\n");

    /* Player view tests */
    printf("Player Serialization:\n");
    test_player_private_has_hand();
    test_player_public_hides_hand();
    test_player_public_has_d10_d4();
    test_player_public_has_discard();
    printf("\n");

    /* Game view tests */
    printf("Game Serialization:\n");
    test_game_for_player_hides_opponent_hand();
    test_game_for_spectator_shows_all_hands();
    printf("\n");

    /* Information leakage tests */
    printf("Information Leakage:\n");
    test_no_hand_leak_in_opponent_json();
    test_view_perspective_opponent();
    test_view_perspective_self();
    test_view_perspective_spectator();
    printf("\n");

    /* Summary */
    printf("==========================================\n");
    printf("  Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("==========================================\n\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
