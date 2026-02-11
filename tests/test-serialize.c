/* test-serialize.c - Tests for gamestate serialization
 *
 * Tests JSON serialization and deserialization for issue 1-012.
 * Run with: make test-serialize
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "../src/core/01-card.h"
#include "../src/core/02-deck.h"
#include "../src/core/03-player.h"
#include "../src/core/04-trade-row.h"
#include "../src/core/05-game.h"
#include "../src/core/09-serialize.h"
#include "../libs/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========================================================================== */
/*                              Test Helpers                                  */
/* ========================================================================== */

static int tests_passed = 0;
static int tests_failed = 0;

/* {{{ TEST macro */
#define TEST(name, condition) do { \
    if (condition) { \
        printf("  [PASS] %s\n", name); \
        tests_passed++; \
    } else { \
        printf("  [FAIL] %s\n", name); \
        tests_failed++; \
    } \
} while(0)
/* }}} */

/* {{{ create_test_game
 * Creates a simple test game with 2 players for serialization testing.
 */
static Game* create_test_game(void) {
    Game* game = game_create(2);
    game_add_player(game, "Alice");
    game_add_player(game, "Bob");

    /* Create starting card types */
    CardType* scout = card_type_create("scout", "Scout", 0,
                                        FACTION_NEUTRAL, CARD_KIND_SHIP);
    scout->effects = effect_array_create(1);
    scout->effects[0].type = EFFECT_TRADE;
    scout->effects[0].value = 1;
    scout->effect_count = 1;

    CardType* viper = card_type_create("viper", "Viper", 0,
                                        FACTION_NEUTRAL, CARD_KIND_SHIP);
    viper->effects = effect_array_create(1);
    viper->effects[0].type = EFFECT_COMBAT;
    viper->effects[0].value = 1;
    viper->effect_count = 1;

    CardType* explorer = card_type_create("explorer", "Explorer", 2,
                                           FACTION_NEUTRAL, CARD_KIND_SHIP);
    explorer->effects = effect_array_create(1);
    explorer->effects[0].type = EFFECT_TRADE;
    explorer->effects[0].value = 2;
    explorer->effect_count = 1;

    game_set_starting_types(game, scout, viper, explorer);

    /* Create trade row with some cards */
    CardType* trade_cards[10];
    for (int i = 0; i < 10; i++) {
        trade_cards[i] = scout;  /* Reuse scout type for simplicity */
    }
    game->trade_row = trade_row_create(trade_cards, 10, explorer);

    game_start(game);
    game_skip_draw_order(game);

    return game;
}
/* }}} */

/* {{{ cleanup_test_game
 * Frees test game and its starting types.
 */
static void cleanup_test_game(Game* game) {
    /* Free the starting types that game_set_starting_types doesn't take ownership of */
    card_type_free(game->scout_type);
    card_type_free(game->viper_type);
    card_type_free(game->explorer_type);
    game_free(game);
}
/* }}} */

/* ========================================================================== */
/*                          Effect Serialization Tests                        */
/* ========================================================================== */

/* {{{ test_effect_serialization */
static void test_effect_serialization(void) {
    printf("\n=== Effect Serialization Tests ===\n");

    /* Test basic effect */
    Effect effect = { EFFECT_TRADE, 5, NULL };
    cJSON* json = serialize_effect(&effect);
    TEST("Effect serialized", json != NULL);

    cJSON* type = cJSON_GetObjectItem(json, "type");
    TEST("Effect type field", type && cJSON_IsString(type));
    TEST("Effect type value", type && strcmp(type->valuestring, "Trade") == 0);

    cJSON* value = cJSON_GetObjectItem(json, "value");
    TEST("Effect value field", value && cJSON_IsNumber(value));
    TEST("Effect value correct", value && value->valuedouble == 5);

    cJSON_Delete(json);

    /* Test effect with target */
    Effect spawn = { EFFECT_SPAWN, 1, "soldier" };
    json = serialize_effect(&spawn);
    TEST("Spawn effect serialized", json != NULL);

    cJSON* target = cJSON_GetObjectItem(json, "target_card_id");
    TEST("Target field present", target && cJSON_IsString(target));
    TEST("Target value correct", target && strcmp(target->valuestring, "soldier") == 0);

    cJSON_Delete(json);

    /* Test NULL effect */
    json = serialize_effect(NULL);
    TEST("NULL effect returns NULL", json == NULL);
}
/* }}} */

/* ========================================================================== */
/*                          Card Serialization Tests                          */
/* ========================================================================== */

/* {{{ test_card_type_serialization */
static void test_card_type_serialization(void) {
    printf("\n=== Card Type Serialization Tests ===\n");

    /* Create a card type with effects */
    CardType* type = card_type_create("dire_bear", "Dire Bear", 4,
                                       FACTION_WILDS, CARD_KIND_SHIP);
    card_type_set_flavor(type, "A fearsome beast.");

    type->effects = effect_array_create(2);
    type->effects[0].type = EFFECT_COMBAT;
    type->effects[0].value = 4;
    type->effects[1].type = EFFECT_DRAW;
    type->effects[1].value = 1;
    type->effect_count = 2;

    type->ally_effects = effect_array_create(1);
    type->ally_effects[0].type = EFFECT_COMBAT;
    type->ally_effects[0].value = 2;
    type->ally_effect_count = 1;

    cJSON* json = serialize_card_type(type);
    TEST("Card type serialized", json != NULL);

    /* Check fields */
    cJSON* id = cJSON_GetObjectItem(json, "id");
    TEST("Card ID field", id && strcmp(id->valuestring, "dire_bear") == 0);

    cJSON* name = cJSON_GetObjectItem(json, "name");
    TEST("Card name field", name && strcmp(name->valuestring, "Dire Bear") == 0);

    cJSON* cost = cJSON_GetObjectItem(json, "cost");
    TEST("Card cost field", cost && cost->valuedouble == 4);

    cJSON* faction = cJSON_GetObjectItem(json, "faction");
    TEST("Card faction field", faction && strcmp(faction->valuestring, "The Wilds") == 0);

    cJSON* kind = cJSON_GetObjectItem(json, "kind");
    TEST("Card kind field", kind && strcmp(kind->valuestring, "Ship") == 0);

    cJSON* flavor = cJSON_GetObjectItem(json, "flavor");
    TEST("Card flavor field", flavor && strcmp(flavor->valuestring, "A fearsome beast.") == 0);

    cJSON* effects = cJSON_GetObjectItem(json, "effects");
    TEST("Effects array exists", effects && cJSON_IsArray(effects));
    TEST("Effects count correct", cJSON_GetArraySize(effects) == 2);

    cJSON* ally_effects = cJSON_GetObjectItem(json, "ally_effects");
    TEST("Ally effects array exists", ally_effects && cJSON_IsArray(ally_effects));

    cJSON_Delete(json);
    card_type_free(type);

    /* Test base type */
    CardType* base = card_type_create("fortress", "Fortress", 5,
                                       FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(base, 6, true);

    json = serialize_card_type(base);
    TEST("Base type serialized", json != NULL);

    cJSON* defense = cJSON_GetObjectItem(json, "defense");
    TEST("Defense field present", defense && defense->valuedouble == 6);

    cJSON* outpost = cJSON_GetObjectItem(json, "is_outpost");
    TEST("Outpost field present", outpost && cJSON_IsTrue(outpost));

    cJSON_Delete(json);
    card_type_free(base);
}
/* }}} */

/* {{{ test_card_instance_serialization */
static void test_card_instance_serialization(void) {
    printf("\n=== Card Instance Serialization Tests ===\n");

    CardType* type = card_type_create("scout", "Scout", 0,
                                       FACTION_NEUTRAL, CARD_KIND_SHIP);
    CardInstance* inst = card_instance_create(type);
    inst->attack_bonus = 2;
    inst->trade_bonus = 1;

    cJSON* json = serialize_card_instance(inst);
    TEST("Instance serialized", json != NULL);

    cJSON* instance_id = cJSON_GetObjectItem(json, "instance_id");
    TEST("Instance ID present", instance_id && cJSON_IsString(instance_id));

    cJSON* card_id = cJSON_GetObjectItem(json, "card_id");
    TEST("Card ID present", card_id && strcmp(card_id->valuestring, "scout") == 0);

    cJSON* attack_bonus = cJSON_GetObjectItem(json, "attack_bonus");
    TEST("Attack bonus present", attack_bonus && attack_bonus->valuedouble == 2);

    cJSON* trade_bonus = cJSON_GetObjectItem(json, "trade_bonus");
    TEST("Trade bonus present", trade_bonus && trade_bonus->valuedouble == 1);

    cJSON* image_seed = cJSON_GetObjectItem(json, "image_seed");
    TEST("Image seed present", image_seed && cJSON_IsNumber(image_seed));

    cJSON_Delete(json);
    card_instance_free(inst);
    card_type_free(type);

    /* Test base instance with placement */
    CardType* base_type = card_type_create("fort", "Fort", 3,
                                            FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(base_type, 5, false);

    CardInstance* base = card_instance_create(base_type);
    base->placement = ZONE_FRONTIER;
    base->deployed = true;
    base->damage_taken = 2;

    json = serialize_card_instance(base);
    TEST("Base instance serialized", json != NULL);

    cJSON* placement = cJSON_GetObjectItem(json, "placement");
    TEST("Placement field present", placement && strcmp(placement->valuestring, "Frontier") == 0);

    cJSON* deployed = cJSON_GetObjectItem(json, "deployed");
    TEST("Deployed field present", deployed && cJSON_IsTrue(deployed));

    cJSON* damage = cJSON_GetObjectItem(json, "damage_taken");
    TEST("Damage field present", damage && damage->valuedouble == 2);

    cJSON_Delete(json);
    card_instance_free(base);
    card_type_free(base_type);
}
/* }}} */

/* ========================================================================== */
/*                          Player Serialization Tests                        */
/* ========================================================================== */

/* {{{ test_player_public_serialization */
static void test_player_public_serialization(void) {
    printf("\n=== Player Public Serialization Tests ===\n");

    Player* player = player_create("TestPlayer", 1);
    player->authority = 42;

    /* Add some cards to test counts */
    CardType* scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    for (int i = 0; i < 3; i++) {
        CardInstance* card = card_instance_create(scout);
        deck_add_to_hand(player->deck, card);
    }
    for (int i = 0; i < 5; i++) {
        CardInstance* card = card_instance_create(scout);
        deck_add_to_draw_pile(player->deck, card);
    }
    for (int i = 0; i < 2; i++) {
        CardInstance* card = card_instance_create(scout);
        deck_add_to_discard(player->deck, card);
    }

    cJSON* json = serialize_player_public(player);
    TEST("Public player serialized", json != NULL);

    /* Check identity */
    cJSON* id = cJSON_GetObjectItem(json, "id");
    TEST("Player ID present", id && id->valuedouble == 1);

    cJSON* name = cJSON_GetObjectItem(json, "name");
    TEST("Player name present", name && strcmp(name->valuestring, "TestPlayer") == 0);

    /* Check authority */
    cJSON* auth = cJSON_GetObjectItem(json, "authority");
    TEST("Authority present", auth && auth->valuedouble == 42);

    /* Check counts (public info) */
    cJSON* hand_count = cJSON_GetObjectItem(json, "hand_count");
    TEST("Hand count present", hand_count && hand_count->valuedouble == 3);

    cJSON* deck_count = cJSON_GetObjectItem(json, "deck_count");
    TEST("Deck count present", deck_count && deck_count->valuedouble == 5);

    cJSON* discard_count = cJSON_GetObjectItem(json, "discard_count");
    TEST("Discard count present", discard_count && discard_count->valuedouble == 2);

    /* Hand contents should NOT be present in public view */
    cJSON* hand = cJSON_GetObjectItem(json, "hand");
    TEST("Hand contents NOT in public view", hand == NULL);

    cJSON_Delete(json);
    player_free(player);
    card_type_free(scout);
}
/* }}} */

/* {{{ test_player_private_serialization */
static void test_player_private_serialization(void) {
    printf("\n=== Player Private Serialization Tests ===\n");

    Player* player = player_create("TestPlayer", 1);
    player->authority = 42;
    player->trade = 5;
    player->combat = 3;
    player->d10 = 7;
    player->d4 = 1;

    /* Add cards */
    CardType* scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    for (int i = 0; i < 3; i++) {
        CardInstance* card = card_instance_create(scout);
        deck_add_to_hand(player->deck, card);
    }

    /* Mark faction played */
    player_mark_faction_played(player, FACTION_WILDS);

    cJSON* json = serialize_player_private(player);
    TEST("Private player serialized", json != NULL);

    /* Check resources (private info) */
    cJSON* trade = cJSON_GetObjectItem(json, "trade");
    TEST("Trade present", trade && trade->valuedouble == 5);

    cJSON* combat = cJSON_GetObjectItem(json, "combat");
    TEST("Combat present", combat && combat->valuedouble == 3);

    cJSON* d10 = cJSON_GetObjectItem(json, "d10");
    TEST("D10 present", d10 && d10->valuedouble == 7);

    cJSON* d4 = cJSON_GetObjectItem(json, "d4");
    TEST("D4 present", d4 && d4->valuedouble == 1);

    /* Hand contents SHOULD be present in private view */
    cJSON* hand = cJSON_GetObjectItem(json, "hand");
    TEST("Hand contents in private view", hand && cJSON_IsArray(hand));
    TEST("Hand has correct count", cJSON_GetArraySize(hand) == 3);

    /* Factions played */
    cJSON* factions = cJSON_GetObjectItem(json, "factions_played");
    TEST("Factions array present", factions && cJSON_IsArray(factions));
    TEST("Has faction played", cJSON_GetArraySize(factions) == 1);

    cJSON_Delete(json);
    player_free(player);
    card_type_free(scout);
}
/* }}} */

/* ========================================================================== */
/*                         Trade Row Serialization Tests                      */
/* ========================================================================== */

/* {{{ test_trade_row_serialization */
static void test_trade_row_serialization(void) {
    printf("\n=== Trade Row Serialization Tests ===\n");

    /* Create card types for trade deck */
    CardType* cards[10];
    for (int i = 0; i < 10; i++) {
        char id[16], name[16];
        snprintf(id, sizeof(id), "card_%d", i);
        snprintf(name, sizeof(name), "Card %d", i);
        cards[i] = card_type_create(id, name, i + 1, FACTION_NEUTRAL, CARD_KIND_SHIP);
    }

    CardType* explorer = card_type_create("explorer", "Explorer", 2,
                                          FACTION_NEUTRAL, CARD_KIND_SHIP);

    TradeRow* row = trade_row_create(cards, 10, explorer);

    cJSON* json = serialize_trade_row(row);
    TEST("Trade row serialized", json != NULL);

    /* Check slots */
    cJSON* slots = cJSON_GetObjectItem(json, "slots");
    TEST("Slots array present", slots && cJSON_IsArray(slots));
    TEST("Slots count is 5", cJSON_GetArraySize(slots) == 5);

    /* Check explorer */
    cJSON* exp_json = cJSON_GetObjectItem(json, "explorer");
    TEST("Explorer object present", exp_json && cJSON_IsObject(exp_json));

    cJSON* exp_id = cJSON_GetObjectItem(exp_json, "card_id");
    TEST("Explorer ID present", exp_id && strcmp(exp_id->valuestring, "explorer") == 0);

    /* Check deck remaining */
    cJSON* deck_remaining = cJSON_GetObjectItem(json, "deck_remaining");
    TEST("Deck remaining present", deck_remaining && cJSON_IsNumber(deck_remaining));
    TEST("Deck remaining correct", deck_remaining->valuedouble == 5); /* 10 - 5 in slots */

    cJSON_Delete(json);
    trade_row_free(row);
    for (int i = 0; i < 10; i++) {
        card_type_free(cards[i]);
    }
    card_type_free(explorer);
}
/* }}} */

/* ========================================================================== */
/*                         Game State Serialization Tests                     */
/* ========================================================================== */

/* {{{ test_game_for_player_serialization */
static void test_game_for_player_serialization(void) {
    printf("\n=== Game State (Player View) Serialization Tests ===\n");

    Game* game = create_test_game();
    game->players[0]->trade = 5;
    game->players[0]->combat = 3;

    /* Serialize for player 0 */
    cJSON* json = serialize_game_for_player(game, 0);
    TEST("Game serialized for player", json != NULL);

    /* Check turn info */
    cJSON* turn = cJSON_GetObjectItem(json, "turn");
    TEST("Turn number present", turn && cJSON_IsNumber(turn));

    cJSON* phase = cJSON_GetObjectItem(json, "phase");
    TEST("Phase present", phase && cJSON_IsString(phase));

    cJSON* active = cJSON_GetObjectItem(json, "active_player");
    TEST("Active player present", active && cJSON_IsNumber(active));

    cJSON* is_your_turn = cJSON_GetObjectItem(json, "is_your_turn");
    TEST("Is your turn present", is_your_turn && cJSON_IsBool(is_your_turn));
    TEST("Is your turn correct", cJSON_IsTrue(is_your_turn)); /* Player 0 is active */

    /* Check "you" (private) */
    cJSON* you = cJSON_GetObjectItem(json, "you");
    TEST("You object present", you && cJSON_IsObject(you));

    cJSON* your_hand = cJSON_GetObjectItem(you, "hand");
    TEST("Your hand present (private)", your_hand && cJSON_IsArray(your_hand));

    cJSON* your_trade = cJSON_GetObjectItem(you, "trade");
    TEST("Your trade present (private)", your_trade && your_trade->valuedouble == 5);

    /* Check "opponents" (public) */
    cJSON* opponents = cJSON_GetObjectItem(json, "opponents");
    TEST("Opponents array present", opponents && cJSON_IsArray(opponents));
    TEST("One opponent", cJSON_GetArraySize(opponents) == 1);

    cJSON* opp = cJSON_GetArrayItem(opponents, 0);
    cJSON* opp_hand = cJSON_GetObjectItem(opp, "hand");
    TEST("Opponent hand NOT present (public)", opp_hand == NULL);

    cJSON* opp_hand_count = cJSON_GetObjectItem(opp, "hand_count");
    TEST("Opponent hand count present", opp_hand_count && cJSON_IsNumber(opp_hand_count));

    /* Check trade row */
    cJSON* trade_row = cJSON_GetObjectItem(json, "trade_row");
    TEST("Trade row present", trade_row && cJSON_IsObject(trade_row));

    cJSON_Delete(json);

    /* Verify different player view */
    json = serialize_game_for_player(game, 1);
    TEST("Game serialized for player 1", json != NULL);

    is_your_turn = cJSON_GetObjectItem(json, "is_your_turn");
    TEST("Not player 1's turn", cJSON_IsFalse(is_your_turn));

    you = cJSON_GetObjectItem(json, "you");
    cJSON* your_id = cJSON_GetObjectItem(you, "id");
    /* Player at index 1 should have ID matching game->players[1]->id */
    TEST("You is correct player", your_id &&
         (int)your_id->valuedouble == game->players[1]->id);

    cJSON_Delete(json);
    cleanup_test_game(game);
}
/* }}} */

/* {{{ test_game_full_serialization */
static void test_game_full_serialization(void) {
    printf("\n=== Game State (Full) Serialization Tests ===\n");

    Game* game = create_test_game();

    cJSON* json = serialize_game_full(game);
    TEST("Full game serialized", json != NULL);

    /* Check all players have full info */
    cJSON* players = cJSON_GetObjectItem(json, "players");
    TEST("Players array present", players && cJSON_IsArray(players));
    TEST("Has 2 players", cJSON_GetArraySize(players) == 2);

    /* Both players should have hand contents (full view) */
    for (int i = 0; i < 2; i++) {
        cJSON* player = cJSON_GetArrayItem(players, i);
        cJSON* hand = cJSON_GetObjectItem(player, "hand");
        char test_name[64];
        snprintf(test_name, sizeof(test_name), "Player %d has hand (full view)", i);
        TEST(test_name, hand && cJSON_IsArray(hand));
    }

    cJSON_Delete(json);
    cleanup_test_game(game);
}
/* }}} */

/* ========================================================================== */
/*                        Action Deserialization Tests                        */
/* ========================================================================== */

/* {{{ test_action_deserialization */
static void test_action_deserialization(void) {
    printf("\n=== Action Deserialization Tests ===\n");

    /* Test play card action */
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "play_card");
    cJSON_AddStringToObject(json, "card_id", "abc123");

    Action* action = deserialize_action(json);
    TEST("Play card action parsed", action != NULL);
    TEST("Action type correct", action && action->type == ACTION_PLAY_CARD);
    TEST("Card ID correct", action && action->card_instance_id &&
         strcmp(action->card_instance_id, "abc123") == 0);

    action_free(action);
    cJSON_Delete(json);

    /* Test buy card action */
    json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "buy_card");
    cJSON_AddNumberToObject(json, "slot", 2);

    action = deserialize_action(json);
    TEST("Buy card action parsed", action != NULL);
    TEST("Action type correct", action && action->type == ACTION_BUY_CARD);
    TEST("Slot correct", action && action->slot == 2);

    action_free(action);
    cJSON_Delete(json);

    /* Test attack action */
    json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "attack_player");
    cJSON_AddNumberToObject(json, "target", 1);
    cJSON_AddNumberToObject(json, "amount", 5);

    action = deserialize_action(json);
    TEST("Attack action parsed", action != NULL);
    TEST("Action type correct", action && action->type == ACTION_ATTACK_PLAYER);
    TEST("Target correct", action && action->target_player == 1);
    TEST("Amount correct", action && action->amount == 5);

    action_free(action);
    cJSON_Delete(json);

    /* Test end turn action */
    json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", "end_turn");

    action = deserialize_action(json);
    TEST("End turn action parsed", action != NULL);
    TEST("Action type correct", action && action->type == ACTION_END_TURN);

    action_free(action);
    cJSON_Delete(json);

    /* Test all action types */
    const char* action_types[] = {
        "play_card", "buy_card", "buy_explorer", "attack_player",
        "attack_base", "scrap_hand", "scrap_discard", "scrap_trade_row", "end_turn"
    };
    ActionType expected[] = {
        ACTION_PLAY_CARD, ACTION_BUY_CARD, ACTION_BUY_EXPLORER, ACTION_ATTACK_PLAYER,
        ACTION_ATTACK_BASE, ACTION_SCRAP_HAND, ACTION_SCRAP_DISCARD, ACTION_SCRAP_TRADE_ROW, ACTION_END_TURN
    };

    for (int i = 0; i < 9; i++) {
        json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "type", action_types[i]);
        action = deserialize_action(json);

        char test_name[64];
        snprintf(test_name, sizeof(test_name), "Action type '%s' parsed", action_types[i]);
        TEST(test_name, action && action->type == expected[i]);

        action_free(action);
        cJSON_Delete(json);
    }

    /* Test NULL handling */
    action = deserialize_action(NULL);
    TEST("NULL input returns NULL", action == NULL);
}
/* }}} */

/* ========================================================================== */
/*                          Utility Function Tests                            */
/* ========================================================================== */

/* {{{ test_utility_functions */
static void test_utility_functions(void) {
    printf("\n=== Utility Function Tests ===\n");

    Game* game = create_test_game();

    /* Test game_state_to_string */
    char* str = game_state_to_string(game, 0);
    TEST("game_state_to_string returns string", str != NULL);
    TEST("String contains turn", str && strstr(str, "\"turn\"") != NULL);
    TEST("String contains phase", str && strstr(str, "\"phase\"") != NULL);
    free(str);

    /* Test game_state_to_string_pretty */
    str = game_state_to_string_pretty(game, 0);
    TEST("game_state_to_string_pretty returns string", str != NULL);
    TEST("Pretty string has newlines", str && strchr(str, '\n') != NULL);
    free(str);

    /* Test invalid player ID */
    cJSON* json = serialize_game_for_player(game, -1);
    TEST("Invalid player ID returns NULL", json == NULL);

    json = serialize_game_for_player(game, 10);
    TEST("Out of range player ID returns NULL", json == NULL);

    cleanup_test_game(game);

    /* Test NULL game */
    json = serialize_game_for_player(NULL, 0);
    TEST("NULL game returns NULL", json == NULL);

    str = game_state_to_string(NULL, 0);
    TEST("NULL game to string returns NULL", str == NULL);
}
/* }}} */

/* ========================================================================== */
/*                           Round-Trip Tests                                 */
/* ========================================================================== */

/* {{{ test_round_trip */
static void test_round_trip(void) {
    printf("\n=== Round-Trip Tests ===\n");

    Game* game = create_test_game();

    /* Serialize to string and verify it's valid JSON */
    char* str = game_state_to_string(game, 0);
    TEST("Serialization succeeds", str != NULL);

    cJSON* parsed = cJSON_Parse(str);
    TEST("Output is valid JSON", parsed != NULL);

    /* Verify key fields survive round-trip */
    cJSON* turn = cJSON_GetObjectItem(parsed, "turn");
    TEST("Turn survives round-trip", turn && turn->valuedouble == game->turn_number);

    cJSON* you = cJSON_GetObjectItem(parsed, "you");
    cJSON* auth = cJSON_GetObjectItem(you, "authority");
    TEST("Authority survives round-trip", auth &&
         auth->valuedouble == game->players[0]->authority);

    cJSON_Delete(parsed);
    free(str);
    cleanup_test_game(game);
}
/* }}} */

/* ========================================================================== */
/*                               Main                                         */
/* ========================================================================== */

/* {{{ main */
int main(void) {
    printf("Symbeline Realms - Serialization Tests (1-012)\n");
    printf("===============================================\n");

    test_effect_serialization();
    test_card_type_serialization();
    test_card_instance_serialization();
    test_player_public_serialization();
    test_player_private_serialization();
    test_trade_row_serialization();
    test_game_for_player_serialization();
    test_game_full_serialization();
    test_action_deserialization();
    test_utility_functions();
    test_round_trip();

    printf("\n===============================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
/* }}} */
