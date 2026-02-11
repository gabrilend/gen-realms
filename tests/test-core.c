/* test-core.c - Tests for core game logic modules
 *
 * Tests card creation, deck management, player state, trade row,
 * game loop, and combat for issues 1-001 through 1-006.
 * Run with: make test
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "../src/core/01-card.h"
#include "../src/core/02-deck.h"
#include "../src/core/03-player.h"
#include "../src/core/04-trade-row.h"
#include "../src/core/05-game.h"
#include "../src/core/06-combat.h"
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

/* {{{ create_test_card_type
 * Creates a simple test card type for testing.
 */
static CardType* create_test_card_type(const char* id, int cost, Faction faction) {
    CardType* type = card_type_create(id, id, cost, faction, CARD_KIND_SHIP);
    if (!type) return NULL;

    /* Add a simple trade effect */
    type->effects = effect_array_create(1);
    if (type->effects) {
        type->effects[0].type = EFFECT_TRADE;
        type->effects[0].value = 2;
        type->effects[0].target_card_id = NULL;
        type->effect_count = 1;
    }

    return type;
}
/* }}} */

/* ========================================================================== */
/*                           Card Tests (1-001)                               */
/* ========================================================================== */

/* {{{ test_card_module */
static void test_card_module(void) {
    printf("\n=== Card Module Tests (1-001) ===\n");

    /* Test CardType creation */
    CardType* type = card_type_create("dire_bear", "Dire Bear", 4,
                                       FACTION_WILDS, CARD_KIND_SHIP);
    TEST("CardType creation", type != NULL);
    TEST("CardType id set", type && strcmp(type->id, "dire_bear") == 0);
    TEST("CardType name set", type && strcmp(type->name, "Dire Bear") == 0);
    TEST("CardType cost set", type && type->cost == 4);
    TEST("CardType faction set", type && type->faction == FACTION_WILDS);
    TEST("CardType kind set", type && type->kind == CARD_KIND_SHIP);

    /* Test flavor text */
    card_type_set_flavor(type, "A fearsome beast from the northern woods.");
    TEST("Flavor text set", type->flavor != NULL);

    /* Test base stats */
    CardType* base = card_type_create("fortress", "Fortress", 5,
                                       FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(base, 6, true);
    TEST("Base defense set", base && base->defense == 6);
    TEST("Base outpost set", base && base->is_outpost == true);

    /* Test CardInstance creation */
    CardInstance* inst = card_instance_create(type);
    TEST("CardInstance creation", inst != NULL);
    TEST("CardInstance has type", inst && inst->type == type);
    TEST("CardInstance has instance_id", inst && inst->instance_id != NULL);
    TEST("CardInstance no initial upgrades", inst && inst->attack_bonus == 0);
    TEST("CardInstance needs_regen true", inst && inst->needs_regen == true);

    /* Test upgrades */
    card_instance_apply_upgrade(inst, EFFECT_UPGRADE_ATTACK, 2);
    TEST("Attack upgrade applied", inst->attack_bonus == 2);
    card_instance_apply_upgrade(inst, EFFECT_UPGRADE_TRADE, 1);
    TEST("Trade upgrade applied", inst->trade_bonus == 1);

    /* Test unique instance IDs */
    CardInstance* inst2 = card_instance_create(type);
    TEST("Unique instance IDs", strcmp(inst->instance_id, inst2->instance_id) != 0);

    /* Test enum to string */
    TEST("Faction to string", strcmp(faction_to_string(FACTION_WILDS), "The Wilds") == 0);
    TEST("Card kind to string", strcmp(card_kind_to_string(CARD_KIND_SHIP), "Ship") == 0);
    TEST("Effect type to string", strcmp(effect_type_to_string(EFFECT_TRADE), "Trade") == 0);

    /* Cleanup */
    card_instance_free(inst);
    card_instance_free(inst2);
    card_type_free(type);
    card_type_free(base);
}
/* }}} */

/* ========================================================================== */
/*                           Deck Tests (1-002)                               */
/* ========================================================================== */

/* {{{ test_deck_module */
static void test_deck_module(void) {
    printf("\n=== Deck Module Tests (1-002) ===\n");

    /* Create test cards */
    CardType* scout = create_test_card_type("scout", 0, FACTION_NEUTRAL);
    CardType* viper = create_test_card_type("viper", 0, FACTION_NEUTRAL);

    /* Test deck creation */
    Deck* deck = deck_create();
    TEST("Deck creation", deck != NULL);
    TEST("Deck empty initially", deck_total_card_count(deck) == 0);

    /* Add cards to draw pile */
    for (int i = 0; i < 5; i++) {
        CardInstance* card = card_instance_create(scout);
        deck_add_to_draw_pile(deck, card);
    }
    for (int i = 0; i < 3; i++) {
        CardInstance* card = card_instance_create(viper);
        deck_add_to_draw_pile(deck, card);
    }
    TEST("Cards added to draw pile", deck->draw_pile_count == 8);
    TEST("Total card count", deck_total_card_count(deck) == 8);

    /* Test shuffle */
    deck_shuffle(deck);
    TEST("Shuffle doesn't lose cards", deck->draw_pile_count == 8);

    /* Test draw */
    CardInstance* drawn = deck_draw_top(deck);
    TEST("Draw returns card", drawn != NULL);
    TEST("Draw moves to hand", deck->hand_count == 1);
    TEST("Draw reduces draw pile", deck->draw_pile_count == 7);

    /* Test draw at index */
    int original_count = deck->draw_pile_count;
    CardInstance* drawn2 = deck_draw_at(deck, 2);
    TEST("Draw at index works", drawn2 != NULL);
    TEST("Draw at reduces count", deck->draw_pile_count == original_count - 1);

    /* Test play from hand */
    bool played = deck_play_from_hand(deck, drawn);
    TEST("Play from hand succeeds", played);
    TEST("Played area has card", deck->played_count == 1);
    TEST("Hand reduced", deck->hand_count == 1);

    /* Test end turn */
    deck_end_turn(deck);
    TEST("End turn clears hand", deck->hand_count == 0);
    TEST("End turn clears played", deck->played_count == 0);
    TEST("End turn adds to discard", deck->discard_count == 2);

    /* Test reshuffle */
    /* Draw remaining cards to trigger reshuffle */
    while (deck->draw_pile_count > 0) {
        deck_draw_top(deck);
    }
    TEST("Draw pile empty", deck->draw_pile_count == 0);

    /* Drawing now should reshuffle discard */
    int discard_before = deck->discard_count;
    if (discard_before > 0) {
        CardInstance* reshuffled = deck_draw_top(deck);
        TEST("Reshuffle on empty draw", reshuffled != NULL);
    }

    /* Test scrap */
    if (deck->hand_count > 0) {
        CardInstance* to_scrap = deck->hand[0];
        CardInstance* scrapped = deck_scrap_from_hand(deck, to_scrap);
        TEST("Scrap returns card", scrapped == to_scrap);
        card_instance_free(scrapped);  /* Caller must free scrapped cards */
    }

    /* Test base handling */
    CardType* base_type = card_type_create("fort", "Fort", 3,
                                            FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(base_type, 5, false);
    CardInstance* base = card_instance_create(base_type);
    deck_add_to_hand(deck, base);
    deck_play_from_hand(deck, base);
    TEST("Base goes to base zone", deck->base_count == 1);
    TEST("Base not in played zone", deck->played_count == 0);

    /* Cleanup */
    deck_free(deck);
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(base_type);
}
/* }}} */

/* ========================================================================== */
/*                          Player Tests (1-003)                              */
/* ========================================================================== */

/* {{{ test_player_module */
static void test_player_module(void) {
    printf("\n=== Player Module Tests (1-003) ===\n");

    /* Test player creation */
    Player* player = player_create("TestPlayer", 1);
    TEST("Player creation", player != NULL);
    TEST("Player name set", strcmp(player->name, "TestPlayer") == 0);
    TEST("Player id set", player->id == 1);
    TEST("Starting authority", player->authority == PLAYER_STARTING_AUTHORITY);
    TEST("Starting d10", player->d10 == PLAYER_STARTING_D10);
    TEST("Starting d4", player->d4 == PLAYER_STARTING_D4);
    TEST("Player has deck", player->deck != NULL);

    /* Test resource management */
    player_add_trade(player, 5);
    TEST("Add trade", player->trade == 5);
    player_add_combat(player, 3);
    TEST("Add combat", player->combat == 3);
    player_add_authority(player, 10);
    TEST("Add authority", player->authority == PLAYER_STARTING_AUTHORITY + 10);

    /* Test spending */
    bool spent = player_spend_trade(player, 3);
    TEST("Spend trade succeeds", spent && player->trade == 2);
    spent = player_spend_trade(player, 10);
    TEST("Spend trade fails if insufficient", !spent && player->trade == 2);

    /* Test damage */
    player_take_damage(player, 20);
    TEST("Take damage", player->authority == PLAYER_STARTING_AUTHORITY + 10 - 20);
    TEST("Player alive", player_is_alive(player));

    /* Test death */
    player_take_damage(player, player->authority);
    TEST("Player dead at 0", !player_is_alive(player));
    player->authority = 50;  /* Restore for more tests */

    /* Test d10/d4 system */
    player->d10 = 8;
    player_d10_increment(player);
    TEST("D10 increment 8->9", player->d10 == 9);
    player_d10_increment(player);
    TEST("D10 overflow 9->0", player->d10 == 0);
    TEST("D4 increments on overflow", player->d4 == 1);

    player->d10 = 1;
    player_d10_decrement(player);
    TEST("D10 decrement 1->0", player->d10 == 0);
    player_d10_decrement(player);
    TEST("D10 underflow 0->9", player->d10 == 9);
    TEST("D4 decrements on underflow", player->d4 == 0);

    /* Test hand size calculation */
    player->d4 = 0;
    TEST("Base hand size", player_get_hand_size(player) == PLAYER_BASE_HAND_SIZE);
    player->d4 = 2;
    TEST("Hand size with bonus", player_get_hand_size(player) == PLAYER_BASE_HAND_SIZE + 2);
    player->d4 = -10;  /* Very negative */
    TEST("Hand size minimum", player_get_hand_size(player) >= PLAYER_MIN_HAND_SIZE);
    player->d4 = 0;

    /* Test faction tracking */
    player_mark_faction_played(player, FACTION_WILDS);
    TEST("Faction marked played", player_has_faction_ally(player, FACTION_WILDS));
    TEST("Other faction not played", !player_has_faction_ally(player, FACTION_MERCHANT));

    /* Test turn reset */
    player->trade = 10;
    player->combat = 5;
    player_reset_turn(player);
    TEST("Turn reset clears trade", player->trade == 0);
    TEST("Turn reset clears combat", player->combat == 0);
    TEST("Turn reset clears factions", !player_has_faction_ally(player, FACTION_WILDS));
    TEST("Turn reset keeps authority", player->authority == 50);
    TEST("Turn reset keeps d10", player->d10 == 9);

    /* Test drawing cards */
    CardType* scout = create_test_card_type("scout", 0, FACTION_NEUTRAL);
    for (int i = 0; i < 10; i++) {
        CardInstance* card = card_instance_create(scout);
        deck_add_to_draw_pile(player->deck, card);
    }
    deck_shuffle(player->deck);

    player->d4 = 0;
    player_draw_starting_hand(player);
    TEST("Draw starting hand", player->deck->hand_count == PLAYER_BASE_HAND_SIZE);

    /* Test end turn */
    player_end_turn(player);
    TEST("End turn clears hand", player->deck->hand_count == 0);
    TEST("End turn moves to discard", player->deck->discard_count == PLAYER_BASE_HAND_SIZE);

    /* Cleanup */
    player_free(player);
    card_type_free(scout);
}
/* }}} */

/* ========================================================================== */
/*                        Trade Row Tests (1-004)                             */
/* ========================================================================== */

/* {{{ test_trade_row_module */
static void test_trade_row_module(void) {
    printf("\n=== Trade Row Module Tests (1-004) ===\n");

    /* Create test card types for trade deck */
    CardType* cards[5];
    for (int i = 0; i < 5; i++) {
        char id[16];
        snprintf(id, sizeof(id), "card_%d", i);
        cards[i] = card_type_create(id, id, i + 1, FACTION_NEUTRAL, CARD_KIND_SHIP);
    }

    /* Create explorer type */
    CardType* explorer = card_type_create("explorer", "Explorer", 2,
                                          FACTION_NEUTRAL, CARD_KIND_SHIP);
    explorer->effects = effect_array_create(1);
    explorer->effects[0].type = EFFECT_TRADE;
    explorer->effects[0].value = 2;
    explorer->effect_count = 1;

    /* Test trade row creation */
    TradeRow* row = trade_row_create(cards, 5, explorer);
    TEST("Trade row creation", row != NULL);
    TEST("Trade row slots filled", trade_row_empty_slot_count(row) == 0);
    TEST("Trade deck depleted by 5", trade_row_deck_remaining(row) == 0);

    /* Test slot access */
    CardInstance* slot0 = trade_row_get_slot(row, 0);
    TEST("Slot 0 has card", slot0 != NULL);
    int cost = trade_row_get_cost(row, 0);
    TEST("Slot cost valid", cost >= 0);

    /* Test purchase validation */
    Player* player = player_create("Buyer", 1);
    player->trade = 0;
    TEST("Cannot buy with 0 trade", !trade_row_can_buy(row, 0, player));

    player->trade = 10;
    TEST("Can buy with enough trade", trade_row_can_buy(row, 0, player));

    /* Test purchasing (won't refill since deck empty) */
    int trade_before = player->trade;
    CardInstance* bought = trade_row_buy(row, 0, player);
    TEST("Buy returns card", bought != NULL);
    TEST("Trade deducted", player->trade < trade_before);
    TEST("Card in player discard", player->deck->discard_count == 1);
    TEST("D10 incremented", player->d10 == PLAYER_STARTING_D10 + 1);
    TEST("Slot now empty", trade_row_get_slot(row, 0) == NULL);

    /* Test explorer purchase */
    player->trade = 10;
    TEST("Can buy explorer", trade_row_can_buy_explorer(row, player));
    CardInstance* exp = trade_row_buy_explorer(row, player);
    TEST("Explorer bought", exp != NULL);
    TEST("Explorer in discard", player->deck->discard_count == 2);

    /* Test scrap from trade row */
    CardInstance* slot1 = trade_row_get_slot(row, 1);
    if (slot1) {
        CardInstance* scrapped = trade_row_scrap(row, 1);
        TEST("Scrap returns card", scrapped == slot1);
        card_instance_free(scrapped);
    }

    /* Cleanup */
    trade_row_free(row);
    player_free(player);
    for (int i = 0; i < 5; i++) {
        card_type_free(cards[i]);
    }
    card_type_free(explorer);
}
/* }}} */

/* ========================================================================== */
/*                           Game Tests (1-005)                               */
/* ========================================================================== */

/* {{{ test_game_module */
static void test_game_module(void) {
    printf("\n=== Game Module Tests (1-005) ===\n");

    /* Test game creation */
    Game* game = game_create(2);
    TEST("Game creation", game != NULL);
    TEST("Phase not started", game->phase == PHASE_NOT_STARTED);

    /* Add players */
    game_add_player(game, "Player 1");
    game_add_player(game, "Player 2");
    TEST("Players added", game->player_count == 2);

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

    game_set_starting_types(game, scout, viper, explorer);

    /* Test game start */
    bool started = game_start(game);
    TEST("Game starts", started);
    TEST("Phase is draw order", game->phase == PHASE_DRAW_ORDER);
    TEST("Turn 1", game->turn_number == 1);
    TEST("Player 0 active", game->active_player == 0);

    /* Check starting decks created */
    Player* p1 = game->players[0];
    TEST("P1 deck has cards", deck_total_card_count(p1->deck) == STARTING_SCOUTS + STARTING_VIPERS);

    /* Test skip draw order */
    game_skip_draw_order(game);
    TEST("Phase is main", game->phase == PHASE_MAIN);
    TEST("P1 hand drawn", p1->deck->hand_count == PLAYER_BASE_HAND_SIZE);

    /* Test play card action */
    if (p1->deck->hand_count > 0) {
        CardInstance* card = p1->deck->hand[0];
        Action* action = action_create(ACTION_PLAY_CARD);
        action->card_instance_id = strdup(card->instance_id);
        bool result = game_process_action(game, action);
        TEST("Play card action", result);
        TEST("Card in played zone", p1->deck->played_count > 0 || p1->deck->base_count > 0);
        action_free(action);
    }

    /* Test end turn */
    Action* end_action = action_create(ACTION_END_TURN);
    game_process_action(game, end_action);
    TEST("Active player switched", game->active_player == 1);
    TEST("Phase is draw order", game->phase == PHASE_DRAW_ORDER);
    action_free(end_action);

    /* Test game over detection */
    Player* p2 = game->players[1];
    p2->authority = 5;
    p1->combat = 10;
    game->active_player = 0;
    game->phase = PHASE_MAIN;

    Action* attack = action_create(ACTION_ATTACK_PLAYER);
    attack->amount = 5;
    game_process_action(game, attack);
    TEST("Game over on death", game->game_over);
    TEST("Winner is attacker", game->winner == 0);
    action_free(attack);

    /* Cleanup */
    game_free(game);
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(explorer);
}
/* }}} */

/* ========================================================================== */
/*                          Combat Tests (1-006)                              */
/* ========================================================================== */

/* {{{ test_combat_module */
static void test_combat_module(void) {
    printf("\n=== Combat Module Tests (1-006) ===\n");

    /* Create a game for combat testing */
    Game* game = game_create(2);
    game_add_player(game, "Attacker");
    game_add_player(game, "Defender");

    CardType* scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    CardType* viper = card_type_create("viper", "Viper", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    CardType* explorer = card_type_create("explorer", "Explorer", 2, FACTION_NEUTRAL, CARD_KIND_SHIP);

    game_set_starting_types(game, scout, viper, explorer);
    game_start(game);
    game_skip_draw_order(game);

    Player* attacker = game->players[0];
    Player* defender = game->players[1];

    /* Test combat available */
    attacker->combat = 5;
    TEST("Combat available", combat_get_available(game) == 5);

    /* Test attack player (no bases) */
    TEST("Can attack player", combat_can_attack_player(game, 1));
    int auth_before = defender->authority;
    bool attacked = combat_attack_player(game, 1, 3);
    TEST("Attack succeeds", attacked);
    TEST("Damage dealt", defender->authority == auth_before - 3);
    TEST("Combat spent", attacker->combat == 2);

    /* Reset for outpost test */
    attacker->combat = 10;
    game->game_over = false;

    /* Create an outpost */
    CardType* outpost_type = card_type_create("outpost", "Outpost", 3,
                                               FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(outpost_type, 4, true);
    CardInstance* outpost = card_instance_create(outpost_type);
    deck_add_base(defender->deck, outpost);

    /* Test outpost blocking */
    TEST("Has outpost", combat_has_outpost(game, 1));
    TEST("Cannot attack player with outpost", !combat_can_attack_player(game, 1));
    bool blocked = combat_attack_player(game, 1, 5);
    TEST("Attack blocked", !blocked);

    /* Test valid targets with outpost */
    CombatTarget targets[10];
    int target_count = combat_get_valid_targets(game, targets, 10);
    TEST("Only outpost valid", target_count == 1 && targets[0].type == TARGET_BASE);

    /* Test attack base */
    attacked = combat_attack_base(game, 1, outpost, 4);
    TEST("Attack base succeeds", attacked);
    TEST("Base destroyed", defender->deck->base_count == 0);
    TEST("Base in discard", defender->deck->discard_count == 1);

    /* Test can attack player now */
    TEST("Can attack after outpost destroyed", combat_can_attack_player(game, 1));

    /* Cleanup */
    game_free(game);
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(explorer);
    card_type_free(outpost_type);
}
/* }}} */

/* ========================================================================== */
/*                               Main                                         */
/* ========================================================================== */

/* {{{ main */
int main(void) {
    printf("Symbeline Realms - Core Module Tests\n");
    printf("=====================================\n");

    test_card_module();
    test_deck_module();
    test_player_module();
    test_trade_row_module();
    test_game_module();
    test_combat_module();

    printf("\n=====================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
/* }}} */
