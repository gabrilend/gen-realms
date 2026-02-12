/* test-core.c - Tests for core game logic modules
 *
 * Tests card creation, deck management, player state, trade row,
 * game loop, combat, and effects for issues 1-001 through 1-007.
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
#include "../src/core/07-effects.h"
#include "../src/core/08-auto-draw.h"
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
    TEST("Base goes to base zone", deck_total_base_count(deck) == 1);
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
        TEST("Card in played zone", p1->deck->played_count > 0 || deck_total_base_count(p1->deck) > 0);
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
    TEST("Base destroyed", deck_total_base_count(defender->deck) == 0);
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
/*                    Base Zone Tests (1-010)                                 */
/* ========================================================================== */

/* {{{ test_base_zones */
static void test_base_zones(void) {
    printf("\n=== Base Zone Tests (1-010) ===\n");

    /* Create game */
    Game* game = game_create(2);
    game_add_player(game, "Player 1");
    game_add_player(game, "Player 2");
    game_start(game);

    CardType* scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    CardType* viper = card_type_create("viper", "Viper", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    CardType* explorer = card_type_create("explorer", "Explorer", 2, FACTION_NEUTRAL, CARD_KIND_SHIP);

    CardType* cards[10];
    for (int i = 0; i < 5; i++) cards[i] = scout;
    for (int i = 5; i < 10; i++) cards[i] = viper;
    game->trade_row = trade_row_create(cards, 10, explorer);

    Player* defender = game->players[1];
    Player* attacker = game->players[0];

    /* Create bases for testing */
    CardType* frontier_type = card_type_create("fort", "Fort", 3, FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(frontier_type, 3, false);

    CardType* interior_type = card_type_create("castle", "Castle", 5, FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(interior_type, 5, false);

    /* Test default placement goes to frontier */
    CardInstance* base1 = card_instance_create(frontier_type);
    deck_add_base(defender->deck, base1);
    TEST("Base defaults to frontier", deck_frontier_count(defender->deck) == 1);
    TEST("Interior empty", deck_interior_count(defender->deck) == 0);
    TEST("Has frontier bases", deck_has_frontier_bases(defender->deck));
    TEST("Placement field set", base1->placement == ZONE_FRONTIER);

    /* Test explicit interior placement */
    CardInstance* base2 = card_instance_create(interior_type);
    base2->placement = ZONE_INTERIOR;
    deck_add_base(defender->deck, base2);
    TEST("Interior has base", deck_interior_count(defender->deck) == 1);
    TEST("Total base count", deck_total_base_count(defender->deck) == 2);

    /* Test combat priority - can't attack interior when frontier exists */
    attacker->combat = 20;

    bool can_attack_interior = combat_attack_base(game, 1, base2, 5);
    TEST("Can't attack interior with frontier", !can_attack_interior);

    /* Test can attack frontier */
    bool can_attack_frontier = combat_attack_base(game, 1, base1, 3);
    TEST("Can attack frontier", can_attack_frontier);
    TEST("Frontier destroyed", deck_frontier_count(defender->deck) == 0);
    TEST("Base1 in discard", defender->deck->discard_count >= 1);

    /* Now can attack interior */
    attacker->combat = 20;
    can_attack_interior = combat_attack_base(game, 1, base2, 5);
    TEST("Can attack interior now", can_attack_interior);
    TEST("Interior destroyed", deck_interior_count(defender->deck) == 0);

    /* Test player attackable after all bases destroyed */
    TEST("No bases remain", deck_total_base_count(defender->deck) == 0);
    TEST("Can attack player", combat_can_attack_player(game, 1));

    /* Test damage accumulation on bases */
    CardInstance* tough_base = card_instance_create(interior_type);  /* 5 defense */
    deck_add_base_to_frontier(defender->deck, tough_base);
    attacker->combat = 10;

    /* First attack - partial damage */
    combat_attack_base(game, 1, tough_base, 3);
    TEST("Damage accumulated", tough_base->damage_taken == 3);
    TEST("Base still alive", deck_frontier_count(defender->deck) == 1);
    TEST("Remaining defense", combat_get_base_defense(tough_base) == 2);

    /* Second attack - finish it off */
    combat_attack_base(game, 1, tough_base, 2);
    TEST("Base destroyed after total >= defense", deck_frontier_count(defender->deck) == 0);

    /* Test reset on destruction */
    CardInstance* discard_base = defender->deck->discard[defender->deck->discard_count - 1];
    TEST("Damage reset on destroy", discard_base->damage_taken == 0);
    TEST("Placement reset on destroy", discard_base->placement == ZONE_NONE);

    /* Cleanup */
    game_free(game);
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(explorer);
    card_type_free(frontier_type);
    card_type_free(interior_type);
}
/* }}} */

/* ========================================================================== */
/*                    Spawning Tests (1-011)                                  */
/* ========================================================================== */

/* {{{ test_spawning_mechanics */
static void test_spawning_mechanics(void) {
    printf("\n=== Spawning Tests (1-011) ===\n");

    /* Create game */
    Game* game = game_create(2);
    game_add_player(game, "Player 1");
    game_add_player(game, "Player 2");

    /* Create and register card types */
    CardType* scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    CardType* viper = card_type_create("viper", "Viper", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    CardType* explorer = card_type_create("explorer", "Explorer", 2, FACTION_NEUTRAL, CARD_KIND_SHIP);

    /* Create unit type that will be spawned */
    CardType* soldier = card_type_create("soldier", "Soldier", 0, FACTION_KINGDOM, CARD_KIND_UNIT);
    soldier->effects = effect_array_create(1);
    soldier->effects[0].type = EFFECT_COMBAT;
    soldier->effects[0].value = 1;
    soldier->effect_count = 1;

    /* Create spawning base */
    CardType* barracks = card_type_create("barracks", "Barracks", 4, FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(barracks, 5, false);
    card_type_set_spawns(barracks, "soldier");

    /* Register types in game (game takes ownership) */
    game_register_card_type(game, soldier);
    game_register_card_type(game, barracks);

    /* Test card type lookup */
    TEST("Find registered type", game_find_card_type(game, "soldier") == soldier);
    TEST("Find unknown returns NULL", game_find_card_type(game, "unknown") == NULL);

    /* Set starting types (required for game_start) */
    game_set_starting_types(game, scout, viper, explorer);

    CardType* cards[10];
    for (int i = 0; i < 5; i++) cards[i] = scout;
    for (int i = 5; i < 10; i++) cards[i] = viper;
    game->trade_row = trade_row_create(cards, 10, explorer);

    bool started = game_start(game);
    TEST("Game started", started);

    Player* p1 = game->players[0];
    int initial_discard = p1->deck->discard_count;

    /* Add a spawning base to player's frontier (not deployed yet) */
    CardInstance* base = card_instance_create(barracks);
    deck_add_base_to_frontier(p1->deck, base);
    TEST("Base not deployed initially", !base->deployed);

    /* End turn and come back to P1 */
    game_skip_draw_order(game);  /* P1 draws */
    Action* end1 = action_create(ACTION_END_TURN);
    game_process_action(game, end1);
    action_free(end1);
    game_skip_draw_order(game);  /* P2 draws */
    Action* end2 = action_create(ACTION_END_TURN);
    game_process_action(game, end2);
    action_free(end2);

    /* Now it's P1's turn again - base should be deployed */
    TEST("Base deployed after turn", base->deployed);

    /* Check that a soldier was spawned into discard */
    int new_discard = p1->deck->discard_count;
    TEST("Unit spawned to discard", new_discard > initial_discard);

    /* Find the spawned unit */
    CardInstance* spawned = NULL;
    for (int i = 0; i < p1->deck->discard_count; i++) {
        if (p1->deck->discard[i]->type == soldier) {
            spawned = p1->deck->discard[i];
            break;
        }
    }
    TEST("Spawned unit exists", spawned != NULL);
    TEST("Spawned unit is soldier type", spawned && spawned->type == soldier);
    TEST("Spawned unit has unique ID", spawned && spawned->instance_id != NULL);
    TEST("Spawned unit has image seed", spawned && spawned->image_seed != 0);

    /* Cleanup (game_free will free registered types) */
    game_free(game);
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(explorer);
}
/* }}} */

/* ========================================================================== */
/*                         Effect Tests (1-007)                               */
/* ========================================================================== */

/* Static counter for callback testing */
static int s_callback_fire_count = 0;
static EffectType s_last_effect_type = EFFECT_TRADE;

/* {{{ test_effect_callback
 * Test callback function that tracks invocations.
 */
static void test_effect_callback(Game* game, Player* player,
                                  CardInstance* source, Effect* effect,
                                  void* context) {
    (void)game; (void)player; (void)source; (void)context;
    s_callback_fire_count++;
    if (effect) {
        s_last_effect_type = effect->type;
    }
}
/* }}} */

/* {{{ test_effects_module */
static void test_effects_module(void) {
    printf("\n=== Effects Module Tests (1-007) ===\n");

    /* Initialize effect system */
    effects_init();
    TEST("Effects init succeeds", true);

    /* Create a game for testing */
    Game* game = game_create(2);
    game_add_player(game, "Player 1");
    game_add_player(game, "Player 2");

    CardType* scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    CardType* viper = card_type_create("viper", "Viper", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    CardType* explorer = card_type_create("explorer", "Explorer", 2, FACTION_NEUTRAL, CARD_KIND_SHIP);

    game_set_starting_types(game, scout, viper, explorer);
    game_start(game);
    game_skip_draw_order(game);

    Player* player = game->players[0];
    player_reset_turn(player);

    /* Test resource effects */
    Effect trade_effect = { EFFECT_TRADE, 5, NULL };
    effects_execute(game, player, &trade_effect, NULL);
    TEST("Trade effect adds trade", player->trade == 5);

    Effect combat_effect = { EFFECT_COMBAT, 3, NULL };
    effects_execute(game, player, &combat_effect, NULL);
    TEST("Combat effect adds combat", player->combat == 3);

    int auth_before = player->authority;
    Effect auth_effect = { EFFECT_AUTHORITY, 2, NULL };
    effects_execute(game, player, &auth_effect, NULL);
    TEST("Authority effect heals", player->authority == auth_before + 2);

    /* Test upgrade bonuses apply through effects */
    CardType* test_card = card_type_create("test", "Test Card", 1, FACTION_MERCHANT, CARD_KIND_SHIP);
    test_card->effects = effect_array_create(1);
    test_card->effects[0].type = EFFECT_TRADE;
    test_card->effects[0].value = 2;
    test_card->effect_count = 1;

    CardInstance* inst = card_instance_create(test_card);
    inst->trade_bonus = 3;  /* Upgrade bonus */

    player->trade = 0;
    Effect trade_with_upgrade = { EFFECT_TRADE, 2, NULL };
    effects_execute(game, player, &trade_with_upgrade, inst);
    TEST("Upgrade bonus applied to effect", player->trade == 5);  /* 2 base + 3 bonus */

    /* Test d10 effects */
    player->d10 = 5;
    Effect d10_up = { EFFECT_D10_UP, 2, NULL };
    effects_execute(game, player, &d10_up, NULL);
    TEST("D10 up effect", player->d10 == 7);

    Effect d10_down = { EFFECT_D10_DOWN, 1, NULL };
    effects_execute(game, player, &d10_down, NULL);
    TEST("D10 down effect", player->d10 == 6);

    /* Test callback registration */
    s_callback_fire_count = 0;
    effects_register_callback(test_effect_callback, NULL);

    Effect callback_test = { EFFECT_TRADE, 1, NULL };
    effects_execute(game, player, &callback_test, NULL);
    TEST("Callback fired", s_callback_fire_count == 1);
    TEST("Callback received effect", s_last_effect_type == EFFECT_TRADE);

    effects_execute(game, player, &callback_test, NULL);
    TEST("Callback fired again", s_callback_fire_count == 2);

    effects_unregister_callback(test_effect_callback);
    effects_execute(game, player, &callback_test, NULL);
    TEST("Callback unregistered", s_callback_fire_count == 2);

    /* Test effects_execute_all */
    player->trade = 0;
    player->combat = 0;
    Effect multi_effects[2] = {
        { EFFECT_TRADE, 3, NULL },
        { EFFECT_COMBAT, 2, NULL }
    };
    effects_execute_all(game, player, multi_effects, 2, NULL);
    TEST("Execute all - trade", player->trade == 3);
    TEST("Execute all - combat", player->combat == 2);

    /* Test draw effect */
    /* Add cards to draw pile first */
    for (int i = 0; i < 5; i++) {
        CardInstance* card = card_instance_create(scout);
        deck_add_to_draw_pile(player->deck, card);
    }
    deck_shuffle(player->deck);
    int hand_before = player->deck->hand_count;
    Effect draw_effect = { EFFECT_DRAW, 2, NULL };
    effects_execute(game, player, &draw_effect, NULL);
    TEST("Draw effect draws cards", player->deck->hand_count == hand_before + 2);

    /* Test effects_execute_card with ally abilities */
    CardType* ally_card = card_type_create("merchant", "Merchant", 2,
                                            FACTION_MERCHANT, CARD_KIND_SHIP);
    ally_card->effects = effect_array_create(1);
    ally_card->effects[0].type = EFFECT_TRADE;
    ally_card->effects[0].value = 2;
    ally_card->effect_count = 1;

    ally_card->ally_effects = effect_array_create(1);
    ally_card->ally_effects[0].type = EFFECT_TRADE;
    ally_card->ally_effects[0].value = 3;
    ally_card->ally_effect_count = 1;

    CardInstance* merchant1 = card_instance_create(ally_card);
    CardInstance* merchant2 = card_instance_create(ally_card);

    player_reset_turn(player);
    player->trade = 0;

    /* First merchant - no ally yet */
    effects_execute_card(game, player, merchant1);
    TEST("First card - primary effect only", player->trade == 2);

    /* Second merchant - ally trigger */
    effects_execute_card(game, player, merchant2);
    TEST("Second card - ally effect triggers", player->trade == 7);  /* 2 + (2+3) */

    /* Test context management */
    EffectContext* ctx = effects_get_context(player);
    TEST("Context returned", ctx != NULL);

    Effect free_ship = { EFFECT_ACQUIRE_FREE, 5, NULL };
    effects_execute(game, player, &free_ship, NULL);
    TEST("Next ship free set", ctx && ctx->next_ship_free == true);
    TEST("Free ship max cost set", ctx && ctx->free_ship_max_cost == 5);

    effects_reset_context(player);
    TEST("Context reset", ctx && ctx->next_ship_free == false);

    /* Cleanup */
    effects_clear_callbacks();
    card_instance_free(inst);
    card_instance_free(merchant1);
    card_instance_free(merchant2);
    card_type_free(test_card);
    card_type_free(ally_card);
    game_free(game);
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(explorer);
}
/* }}} */

/* ========================================================================== */
/*                      Auto-Draw Tests (1-008)                               */
/* ========================================================================== */

/* Static counter for auto-draw event testing */
static int s_autodraw_event_count = 0;
static AutoDrawEventType s_last_autodraw_event = AUTODRAW_EVENT_START;
static int s_autodraw_total_drawn = 0;

/* {{{ test_autodraw_listener
 * Test listener for auto-draw events.
 */
static void test_autodraw_listener(Game* game, Player* player,
                                    AutoDrawEvent* event, void* context) {
    (void)game; (void)player; (void)context;
    s_autodraw_event_count++;
    if (event) {
        s_last_autodraw_event = event->type;
        if (event->type == AUTODRAW_EVENT_COMPLETE) {
            s_autodraw_total_drawn = event->total_drawn;
        }
    }
}
/* }}} */

/* {{{ test_autodraw_module */
static void test_autodraw_module(void) {
    printf("\n=== Auto-Draw Module Tests (1-008) ===\n");

    /* Create card types */
    CardType* scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    scout->effects = effect_array_create(1);
    scout->effects[0].type = EFFECT_TRADE;
    scout->effects[0].value = 1;
    scout->effect_count = 1;

    CardType* viper = card_type_create("viper", "Viper", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    viper->effects = effect_array_create(1);
    viper->effects[0].type = EFFECT_COMBAT;
    viper->effects[0].value = 1;
    viper->effect_count = 1;

    /* Create a card with draw effect */
    CardType* courier = card_type_create("courier", "Guild Courier", 2, FACTION_MERCHANT, CARD_KIND_SHIP);
    courier->effects = effect_array_create(2);
    courier->effects[0].type = EFFECT_TRADE;
    courier->effects[0].value = 2;
    courier->effects[1].type = EFFECT_DRAW;
    courier->effects[1].value = 1;  /* Draw 1 card */
    courier->effect_count = 2;

    CardType* explorer = card_type_create("explorer", "Explorer", 2, FACTION_NEUTRAL, CARD_KIND_SHIP);

    /* Test eligibility detection (1-008a) */
    TEST("Scout has no draw effect", !autodraw_has_draw_effect(scout));
    TEST("Courier has draw effect", autodraw_has_draw_effect(courier));

    /* Create card instances */
    CardInstance* courier_inst = card_instance_create(courier);
    CardInstance* scout_inst = card_instance_create(scout);

    TEST("Courier eligible for auto-draw", autodraw_is_eligible(courier_inst));
    TEST("Scout not eligible", !autodraw_is_eligible(scout_inst));

    /* Get draw effect */
    Effect* draw_eff = autodraw_get_draw_effect(courier_inst);
    TEST("Draw effect found", draw_eff != NULL);
    TEST("Draw effect value", draw_eff && draw_eff->value == 1);

    /* Test spent flag management (1-008c) */
    TEST("Not spent initially", !autodraw_is_spent(courier_inst));
    autodraw_mark_spent(courier_inst);
    TEST("Marked as spent", autodraw_is_spent(courier_inst));
    TEST("Spent card not eligible", !autodraw_is_eligible(courier_inst));

    /* Reset for next test */
    courier_inst->draw_effect_spent = false;

    /* Test finding eligible cards in array */
    CardInstance* hand[3] = { scout_inst, courier_inst, NULL };
    hand[2] = card_instance_create(scout);
    AutoDrawCandidate candidates[3];
    int found = autodraw_find_eligible(hand, 3, candidates, 3);
    TEST("Found 1 eligible card", found == 1);
    TEST("Eligible card is courier", found > 0 && candidates[0].card == courier_inst);

    /* Test chain resolution (1-008b) */
    Game* game = game_create(2);
    game_add_player(game, "Player 1");
    game_add_player(game, "Player 2");
    game_set_starting_types(game, scout, viper, explorer);

    /* Create simple trade row */
    CardType* cards[5];
    for (int i = 0; i < 5; i++) cards[i] = scout;
    game->trade_row = trade_row_create(cards, 5, explorer);

    bool started = game_start(game);
    TEST("Game started for chain test", started);

    Player* player = game->players[0];

    /* Clear hand and add test cards */
    while (player->deck->hand_count > 0) {
        deck_add_to_discard(player->deck, player->deck->hand[0]);
        player->deck->hand[0] = player->deck->hand[player->deck->hand_count - 1];
        player->deck->hand_count--;
    }

    /* Add courier to hand */
    CardInstance* chain_courier = card_instance_create(courier);
    deck_add_to_hand(player->deck, chain_courier);

    /* Add cards to draw pile so chain can draw */
    for (int i = 0; i < 3; i++) {
        CardInstance* card = card_instance_create(scout);
        deck_add_to_draw_pile(player->deck, card);
    }

    int hand_before = player->deck->hand_count;
    int draw_before = player->deck->draw_pile_count;

    /* Test event emission (1-008d) */
    s_autodraw_event_count = 0;
    s_autodraw_total_drawn = 0;
    autodraw_register_listener(test_autodraw_listener, NULL);

    AutoDrawResult result = autodraw_resolve_chain(game, player);

    TEST("Chain resolved OK", result == AUTODRAW_OK);
    TEST("Courier marked spent", chain_courier->draw_effect_spent);
    TEST("Hand increased", player->deck->hand_count > hand_before);
    TEST("Draw pile decreased", player->deck->draw_pile_count < draw_before);

    /* Check events */
    TEST("Events emitted", s_autodraw_event_count > 0);
    TEST("Complete event received", s_last_autodraw_event == AUTODRAW_EVENT_COMPLETE);
    TEST("Total drawn tracked", s_autodraw_total_drawn > 0);

    /* Test second chain does nothing (card is spent) */
    s_autodraw_total_drawn = 0;
    result = autodraw_resolve_chain(game, player);
    TEST("Second chain - no eligible", result == AUTODRAW_NO_ELIGIBLE || s_autodraw_total_drawn == 0);

    /* Test shuffle resets spent flag - move card to discard first */
    deck_discard_from_hand(player->deck, chain_courier);
    TEST("Courier in discard", deck_discard_contains(player->deck, chain_courier));
    deck_reshuffle_discard(player->deck);
    TEST("Shuffle resets spent", !chain_courier->draw_effect_spent);

    /* Cleanup */
    autodraw_clear_listeners();
    card_instance_free(courier_inst);
    card_instance_free(scout_inst);
    card_instance_free(hand[2]);
    game_free(game);
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(courier);
    card_type_free(explorer);
}
/* }}} */

/* ========================================================================== */
/*                    Card Manipulation Effects Tests (1-007c)                */
/* ========================================================================== */

/* {{{ test_card_manipulation_effects */
static void test_card_manipulation_effects(void) {
    printf("\n=== Card Manipulation Effects Tests (1-007c) ===\n");

    /* Create card types */
    CardType* scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    scout->effects = effect_array_create(1);
    scout->effects[0].type = EFFECT_TRADE;
    scout->effects[0].value = 1;
    scout->effect_count = 1;

    CardType* viper = card_type_create("viper", "Viper", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    viper->effects = effect_array_create(1);
    viper->effects[0].type = EFFECT_COMBAT;
    viper->effects[0].value = 1;
    viper->effect_count = 1;

    CardType* explorer = card_type_create("explorer", "Explorer", 2, FACTION_NEUTRAL, CARD_KIND_SHIP);

    /* Create a card with discard effect */
    CardType* thief = card_type_create("thief", "Guild Thief", 3, FACTION_WILDS, CARD_KIND_SHIP);
    thief->effects = effect_array_create(2);
    thief->effects[0].type = EFFECT_COMBAT;
    thief->effects[0].value = 2;
    thief->effects[1].type = EFFECT_DISCARD;
    thief->effects[1].value = 1;  /* Opponent discards 1 */
    thief->effect_count = 2;

    /* Create a card with scrap trade row effect */
    CardType* saboteur = card_type_create("saboteur", "Saboteur", 4, FACTION_WILDS, CARD_KIND_SHIP);
    saboteur->effects = effect_array_create(1);
    saboteur->effects[0].type = EFFECT_SCRAP_TRADE_ROW;
    saboteur->effects[0].value = 1;
    saboteur->effect_count = 1;

    /* Create a card with scrap hand effect */
    CardType* recycler = card_type_create("recycler", "Recycler", 2, FACTION_ARTIFICER, CARD_KIND_SHIP);
    recycler->effects = effect_array_create(1);
    recycler->effects[0].type = EFFECT_SCRAP_HAND;
    recycler->effects[0].value = 1;
    recycler->effect_count = 1;

    /* Create a card with top deck effect */
    CardType* oracle = card_type_create("oracle", "Oracle", 3, FACTION_KINGDOM, CARD_KIND_SHIP);
    oracle->effects = effect_array_create(1);
    oracle->effects[0].type = EFFECT_TOP_DECK;
    oracle->effects[0].value = 1;
    oracle->effect_count = 1;

    /* Set up game */
    Game* game = game_create(2);
    game_add_player(game, "Player 1");
    game_add_player(game, "Player 2");
    game_set_starting_types(game, scout, viper, explorer);

    /* Create trade row with different cards */
    CardType* trade_cards[5] = { saboteur, thief, recycler, oracle, scout };
    game->trade_row = trade_row_create(trade_cards, 5, explorer);

    game_start(game);
    game_skip_draw_order(game);

    Player* player1 = game->players[0];
    Player* player2 = game->players[1];

    /* Test pending action queue operations */
    TEST("No pending initially", !game_has_pending_action(game));

    PendingAction test_action = {
        .type = PENDING_DISCARD,
        .player_id = 2,
        .count = 1,
        .min_count = 1,
        .resolved_count = 0,
        .optional = false
    };
    game_push_pending_action(game, &test_action);
    TEST("Pending action pushed", game_has_pending_action(game));

    PendingAction* pending = game_get_pending_action(game);
    TEST("Get pending action", pending != NULL);
    TEST("Pending type correct", pending && pending->type == PENDING_DISCARD);
    TEST("Pending player correct", pending && pending->player_id == 2);

    game_pop_pending_action(game);
    TEST("Pending action popped", !game_has_pending_action(game));

    /* Test discard effect creates pending action */
    effects_init();
    Effect discard_eff = { EFFECT_DISCARD, 1, NULL };
    effects_execute(game, player1, &discard_eff, NULL);

    TEST("Discard effect creates pending", game_has_pending_action(game));
    pending = game_get_pending_action(game);
    TEST("Discard pending type", pending && pending->type == PENDING_DISCARD);
    TEST("Discard for opponent", pending && pending->player_id == player2->id);
    TEST("Discard count 1", pending && pending->count == 1);
    TEST("Discard not optional", pending && pending->optional == false);

    /* Resolve discard action - add card to opponent's hand first */
    CardInstance* opponent_card = card_instance_create(scout);
    deck_add_to_hand(player2->deck, opponent_card);
    int hand_before = player2->deck->hand_count;
    int discard_before = player2->deck->discard_count;

    bool resolved = game_resolve_discard(game, opponent_card->instance_id);
    TEST("Discard resolved", resolved);
    TEST("Card removed from hand", player2->deck->hand_count == hand_before - 1);
    TEST("Card added to discard", player2->deck->discard_count == discard_before + 1);
    TEST("Pending removed after resolve", !game_has_pending_action(game));

    /* Test scrap trade row effect */
    Effect scrap_tr_eff = { EFFECT_SCRAP_TRADE_ROW, 1, NULL };
    effects_execute(game, player1, &scrap_tr_eff, NULL);

    TEST("Scrap TR creates pending", game_has_pending_action(game));
    pending = game_get_pending_action(game);
    TEST("Scrap TR pending type", pending && pending->type == PENDING_SCRAP_TRADE_ROW);
    TEST("Scrap TR is optional", pending && pending->optional == true);

    /* Resolve scrap trade row action */
    resolved = game_resolve_scrap_trade_row(game, 0);
    TEST("Scrap TR resolved", resolved);
    TEST("TR slot now empty", game->trade_row->slots[0] == NULL);
    TEST("Pending removed", !game_has_pending_action(game));

    /* Test scrap hand effect */
    Effect scrap_hand_eff = { EFFECT_SCRAP_HAND, 1, NULL };
    effects_execute(game, player1, &scrap_hand_eff, NULL);

    TEST("Scrap hand creates pending", game_has_pending_action(game));
    pending = game_get_pending_action(game);
    TEST("Scrap hand pending type", pending && pending->type == PENDING_SCRAP_HAND_DISCARD);
    TEST("Scrap hand is optional", pending && pending->optional == true);

    /* Add card to hand for scrapping */
    CardInstance* scrap_target = card_instance_create(scout);
    deck_add_to_hand(player1->deck, scrap_target);
    char* scrap_id = strdup(scrap_target->instance_id);
    int d10_before = player1->d10;

    resolved = game_resolve_scrap_hand(game, scrap_id);
    TEST("Scrap hand resolved", resolved);
    TEST("D10 decremented", player1->d10 == d10_before - 1 || (d10_before == 0 && player1->d10 == 9));
    TEST("Pending removed after scrap", !game_has_pending_action(game));

    /* Test scrap from discard */
    Effect scrap_hand_eff2 = { EFFECT_SCRAP_HAND, 1, NULL };
    effects_execute(game, player1, &scrap_hand_eff2, NULL);

    /* Add card to discard for scrapping */
    CardInstance* discard_scrap = card_instance_create(viper);
    deck_add_to_discard(player1->deck, discard_scrap);
    char* discard_scrap_id = strdup(discard_scrap->instance_id);
    int discard_count_before = player1->deck->discard_count;

    resolved = game_resolve_scrap_discard(game, discard_scrap_id);
    TEST("Scrap discard resolved", resolved);
    TEST("Discard count reduced", player1->deck->discard_count == discard_count_before - 1);
    TEST("Pending removed", !game_has_pending_action(game));

    /* Test top deck effect */
    Effect top_deck_eff = { EFFECT_TOP_DECK, 1, NULL };
    effects_execute(game, player1, &top_deck_eff, NULL);

    TEST("Top deck creates pending", game_has_pending_action(game));
    pending = game_get_pending_action(game);
    TEST("Top deck pending type", pending && pending->type == PENDING_TOP_DECK);

    /* Add card to discard for top deck */
    CardInstance* top_target = card_instance_create(scout);
    deck_add_to_discard(player1->deck, top_target);
    char* top_id = strdup(top_target->instance_id);
    int draw_before = player1->deck->draw_pile_count;

    resolved = game_resolve_top_deck(game, top_id);
    TEST("Top deck resolved", resolved);
    TEST("Draw pile increased", player1->deck->draw_pile_count == draw_before + 1);
    TEST("Card on top of deck", player1->deck->draw_pile[0] == top_target);
    TEST("Pending removed", !game_has_pending_action(game));

    /* Test skipping optional action */
    Effect skip_eff = { EFFECT_SCRAP_TRADE_ROW, 1, NULL };
    effects_execute(game, player1, &skip_eff, NULL);
    TEST("Optional pending created", game_has_pending_action(game));

    bool skipped = game_skip_pending_action(game);
    TEST("Optional skipped", skipped);
    TEST("Pending removed after skip", !game_has_pending_action(game));

    /* Test cannot skip required action */
    game_request_discard(game, player2->id, 1);
    skipped = game_skip_pending_action(game);
    TEST("Required not skipped", !skipped);
    game_clear_pending_actions(game);

    /* Test draw effect (verify already working) */
    CardInstance* draw_target = card_instance_create(scout);
    deck_add_to_draw_pile(player1->deck, draw_target);
    hand_before = player1->deck->hand_count;

    Effect draw_eff = { EFFECT_DRAW, 1, NULL };
    effects_execute(game, player1, &draw_eff, NULL);
    TEST("Draw effect works", player1->deck->hand_count == hand_before + 1);
    TEST("Draw no pending action", !game_has_pending_action(game));

    /* Cleanup */
    free(scrap_id);
    free(discard_scrap_id);
    free(top_id);
    game_free(game);
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(explorer);
    card_type_free(thief);
    card_type_free(saboteur);
    card_type_free(recycler);
    card_type_free(oracle);
}
/* }}} */

/* ========================================================================== */
/*                      Special Effects Tests (1-007d)                        */
/* ========================================================================== */

/* {{{ test_special_effects */
static void test_special_effects(void) {
    printf("\n=== Special Effects Tests (1-007d) ===\n");

    /* Create card types */
    CardType* scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    scout->effects = effect_array_create(1);
    scout->effects[0].type = EFFECT_TRADE;
    scout->effects[0].value = 1;
    scout->effect_count = 1;

    CardType* viper = card_type_create("viper", "Viper", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    viper->effects = effect_array_create(1);
    viper->effects[0].type = EFFECT_COMBAT;
    viper->effects[0].value = 1;
    viper->effect_count = 1;

    CardType* explorer = card_type_create("explorer", "Explorer", 2, FACTION_NEUTRAL, CARD_KIND_SHIP);
    explorer->effects = effect_array_create(2);
    explorer->effects[0].type = EFFECT_TRADE;
    explorer->effects[0].value = 2;
    explorer->effects[1].type = EFFECT_COMBAT;
    explorer->effects[1].value = 2;
    explorer->effect_count = 2;

    /* Create a card with copy ship effect */
    CardType* mimic = card_type_create("mimic", "Mimic Ship", 4, FACTION_ARTIFICER, CARD_KIND_SHIP);
    mimic->effects = effect_array_create(1);
    mimic->effects[0].type = EFFECT_COPY_SHIP;
    mimic->effects[0].value = 0;
    mimic->effect_count = 1;

    /* Create a card with destroy base effect */
    CardType* assassin = card_type_create("assassin", "Assassin", 5, FACTION_WILDS, CARD_KIND_SHIP);
    assassin->effects = effect_array_create(1);
    assassin->effects[0].type = EFFECT_DESTROY_BASE;
    assassin->effects[0].value = 0;
    assassin->effect_count = 1;

    /* Create a card with acquire free effect */
    CardType* patron = card_type_create("patron", "Patron", 4, FACTION_MERCHANT, CARD_KIND_SHIP);
    patron->effects = effect_array_create(1);
    patron->effects[0].type = EFFECT_ACQUIRE_FREE;
    patron->effects[0].value = 4;  /* Can acquire card up to cost 4 */
    patron->effect_count = 1;

    /* Create a card with acquire top effect */
    CardType* oracle = card_type_create("oracle", "Oracle", 3, FACTION_KINGDOM, CARD_KIND_SHIP);
    oracle->effects = effect_array_create(1);
    oracle->effects[0].type = EFFECT_ACQUIRE_TOP;
    oracle->effects[0].value = 0;
    oracle->effect_count = 1;

    /* Create a base for testing destroy */
    CardType* fortress = card_type_create("fortress", "Fortress", 5, FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(fortress, 6, false);

    /* Set up game */
    effects_init();

    Game* game = game_create(2);
    game_add_player(game, "Player 1");
    game_add_player(game, "Player 2");
    game_set_starting_types(game, scout, viper, explorer);

    /* Create trade row with more cards */
    CardType* trade_cards[15] = {
        mimic, assassin, patron, oracle, explorer,
        scout, scout, scout, scout, scout,
        viper, viper, viper, viper, viper
    };
    game->trade_row = trade_row_create(trade_cards, 15, explorer);

    game_start(game);
    game_skip_draw_order(game);

    Player* player1 = game->players[0];
    Player* player2 = game->players[1];

    /* Test copy ship effect */
    Effect copy_eff = { EFFECT_COPY_SHIP, 0, NULL };
    effects_execute(game, player1, &copy_eff, NULL);

    TEST("Copy ship creates pending", game_has_pending_action(game));
    PendingAction* pending = game_get_pending_action(game);
    TEST("Copy ship pending type", pending && pending->type == PENDING_COPY_SHIP);
    TEST("Copy ship is optional", pending && pending->optional == true);

    /* Add a ship to played area for copying */
    CardInstance* target_ship = card_instance_create(explorer);
    deck_add_to_played(player1->deck, target_ship);

    int trade_before = player1->trade;
    int combat_before = player1->combat;

    bool resolved = game_resolve_copy_ship(game, target_ship->instance_id);
    TEST("Copy ship resolved", resolved);
    TEST("Copy ship gained trade", player1->trade == trade_before + 2);
    TEST("Copy ship gained combat", player1->combat == combat_before + 2);
    TEST("Pending removed", !game_has_pending_action(game));

    /* Test destroy base effect */
    Effect destroy_eff = { EFFECT_DESTROY_BASE, 0, NULL };
    effects_execute(game, player1, &destroy_eff, NULL);

    TEST("Destroy base creates pending", game_has_pending_action(game));
    pending = game_get_pending_action(game);
    TEST("Destroy base pending type", pending && pending->type == PENDING_DESTROY_BASE);

    /* Add a base to opponent */
    CardInstance* opp_base = card_instance_create(fortress);
    opp_base->placement = ZONE_FRONTIER;
    deck_add_base(player2->deck, opp_base);

    int base_count_before = deck_total_base_count(player2->deck);
    resolved = game_resolve_destroy_base(game, opp_base->instance_id);
    TEST("Destroy base resolved", resolved);
    TEST("Base removed from opponent", deck_total_base_count(player2->deck) == base_count_before - 1);
    TEST("Pending removed", !game_has_pending_action(game));

    /* Test acquire free effect */
    extern EffectContext* effects_get_context(Player*);
    EffectContext* ctx = effects_get_context(player1);

    Effect free_eff = { EFFECT_ACQUIRE_FREE, 4, NULL };
    effects_execute(game, player1, &free_eff, NULL);

    TEST("Acquire free flag set", ctx && ctx->next_ship_free == true);
    TEST("Free max cost set", ctx && ctx->free_ship_max_cost == 4);

    /* Buy card with no trade using game_buy_card */
    player1->trade = 0;
    int discard_before = player1->deck->discard_count;
    CardInstance* bought = game_buy_card(game, 0);  /* Slot 0 */
    TEST("Buy with free succeeds", bought != NULL);
    TEST("Trade still 0", player1->trade == 0);
    TEST("Card in discard", player1->deck->discard_count == discard_before + 1);
    TEST("Free flag reset", ctx && ctx->next_ship_free == false);

    /* Test acquire top effect */
    Effect top_eff = { EFFECT_ACQUIRE_TOP, 0, NULL };
    effects_execute(game, player1, &top_eff, NULL);

    TEST("Acquire top flag set", ctx && ctx->next_ship_to_top == true);

    /* Buy card and check it goes to deck top */
    player1->trade = 10;  /* Enough to buy */
    int draw_before = player1->deck->draw_pile_count;
    discard_before = player1->deck->discard_count;
    bought = game_buy_card(game, 0);

    TEST("Buy with top succeeds", bought != NULL);
    TEST("Draw pile increased", player1->deck->draw_pile_count == draw_before + 1);
    TEST("Discard unchanged", player1->deck->discard_count == discard_before);
    TEST("Card on top of deck", player1->deck->draw_pile[0] == bought);
    TEST("Top flag reset", ctx && ctx->next_ship_to_top == false);

    /* Test skip copy ship */
    effects_execute(game, player1, &copy_eff, NULL);
    bool skipped = game_skip_pending_action(game);
    TEST("Copy ship skipped", skipped);
    TEST("Pending removed after skip", !game_has_pending_action(game));

    /* Cleanup */
    /* Note: target_ship is owned by player1->deck->played, freed by game_free */
    /* Note: opp_base was freed by game_resolve_destroy_base */
    game_free(game);
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(explorer);
    card_type_free(mimic);
    card_type_free(assassin);
    card_type_free(patron);
    card_type_free(oracle);
    card_type_free(fortress);
}
/* }}} */

/* {{{ test_upgrade_spawn_effects */
static void test_upgrade_spawn_effects(void) {
    printf("\n=== Upgrade and Spawn Effects Tests (1-007e) ===\n");

    /* Create card types for testing */
    CardType* us_scout = card_type_create("us_scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    us_scout->effects = effect_array_create(1);
    us_scout->effects[0].type = EFFECT_TRADE;
    us_scout->effects[0].value = 1;
    us_scout->effect_count = 1;

    CardType* us_viper = card_type_create("us_viper", "Viper", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    us_viper->effects = effect_array_create(1);
    us_viper->effects[0].type = EFFECT_COMBAT;
    us_viper->effects[0].value = 1;
    us_viper->effect_count = 1;

    CardType* us_explorer = card_type_create("us_explorer", "Explorer", 2, FACTION_NEUTRAL, CARD_KIND_SHIP);
    us_explorer->effects = effect_array_create(1);
    us_explorer->effects[0].type = EFFECT_TRADE;
    us_explorer->effects[0].value = 2;
    us_explorer->effect_count = 1;

    /* Unit type for spawn testing */
    CardType* us_soldier = card_type_create("us_soldier", "Soldier", 0, FACTION_KINGDOM, CARD_KIND_UNIT);
    us_soldier->effects = effect_array_create(1);
    us_soldier->effects[0].type = EFFECT_COMBAT;
    us_soldier->effects[0].value = 1;
    us_soldier->effect_count = 1;

    TEST("Card types created", us_scout && us_viper && us_explorer && us_soldier);

    /* Create game with starting types */
    Game* game = game_create(2);
    game_add_player(game, "Player 1");
    game_add_player(game, "Player 2");
    game_set_starting_types(game, us_scout, us_viper, us_explorer);
    game_register_card_type(game, us_soldier);  /* game takes ownership */

    /* Set up trade row */
    CardType* trade_cards[10];
    for (int i = 0; i < 10; i++) trade_cards[i] = us_scout;
    game->trade_row = trade_row_create(trade_cards, 10, us_explorer);

    bool started = game_start(game);
    TEST("Game started", started);
    game_skip_draw_order(game);
    TEST("Phase is main", game->phase == PHASE_MAIN);

    Player* player1 = game->players[0];
    TEST("Game setup complete", player1 != NULL);

    /* ===== Test 1: EFFECT_UPGRADE_ATTACK creates pending action ===== */
    Effect upgrade_atk = { .type = EFFECT_UPGRADE_ATTACK, .value = 2 };
    effects_execute(game, player1, &upgrade_atk, NULL);
    TEST("Upgrade attack creates pending", game_has_pending_action(game));

    PendingAction* pending = game_get_pending_action(game);
    TEST("Upgrade pending type", pending && pending->type == PENDING_UPGRADE);
    TEST("Upgrade is optional", pending && pending->optional);
    TEST("Upgrade type stored", pending && pending->upgrade_type == EFFECT_UPGRADE_ATTACK);
    TEST("Upgrade value stored", pending && pending->upgrade_value == 2);

    /* Get a card from hand to upgrade */
    CardInstance* hand_card = player1->deck->hand_count > 0 ? player1->deck->hand[0] : NULL;
    TEST("Have card in hand", hand_card != NULL);

    int orig_attack = hand_card ? hand_card->attack_bonus : 0;
    bool resolved = game_resolve_upgrade(game, hand_card ? hand_card->instance_id : "");
    TEST("Upgrade resolved", resolved);
    TEST("Attack bonus applied", hand_card && hand_card->attack_bonus == orig_attack + 2);
    TEST("Pending removed", !game_has_pending_action(game));

    /* ===== Test 2: EFFECT_UPGRADE_TRADE ===== */
    Effect upgrade_trade = { .type = EFFECT_UPGRADE_TRADE, .value = 1 };
    effects_execute(game, player1, &upgrade_trade, NULL);
    TEST("Upgrade trade creates pending", game_has_pending_action(game));

    pending = game_get_pending_action(game);
    TEST("Upgrade trade type stored", pending && pending->upgrade_type == EFFECT_UPGRADE_TRADE);

    CardInstance* card2 = player1->deck->hand_count > 1 ? player1->deck->hand[1] : player1->deck->hand[0];
    int orig_trade = card2 ? card2->trade_bonus : 0;
    resolved = game_resolve_upgrade(game, card2 ? card2->instance_id : "");
    TEST("Trade upgrade resolved", resolved);
    TEST("Trade bonus applied", card2 && card2->trade_bonus == orig_trade + 1);

    /* ===== Test 3: EFFECT_UPGRADE_AUTH ===== */
    Effect upgrade_auth = { .type = EFFECT_UPGRADE_AUTH, .value = 3 };
    effects_execute(game, player1, &upgrade_auth, NULL);
    TEST("Upgrade auth creates pending", game_has_pending_action(game));

    pending = game_get_pending_action(game);
    TEST("Upgrade auth type stored", pending && pending->upgrade_type == EFFECT_UPGRADE_AUTH);

    CardInstance* card3 = player1->deck->hand[0];
    int orig_auth = card3 ? card3->authority_bonus : 0;
    resolved = game_resolve_upgrade(game, card3 ? card3->instance_id : "");
    TEST("Auth upgrade resolved", resolved);
    TEST("Auth bonus applied", card3 && card3->authority_bonus == orig_auth + 3);

    /* ===== Test 4: Upgrade can be skipped ===== */
    Effect upgrade_skip = { .type = EFFECT_UPGRADE_ATTACK, .value = 1 };
    effects_execute(game, player1, &upgrade_skip, NULL);
    TEST("Upgrade pending exists", game_has_pending_action(game));
    bool skipped = game_skip_pending_action(game);
    TEST("Upgrade skipped", skipped);
    TEST("Pending removed after skip", !game_has_pending_action(game));

    /* ===== Test 5: EFFECT_SPAWN creates units ===== */
    int initial_discard = player1->deck->discard_count;
    Effect spawn_eff = {
        .type = EFFECT_SPAWN,
        .value = 1,
        .target_card_id = "us_soldier"
    };
    effects_execute(game, player1, &spawn_eff, NULL);
    TEST("Spawn adds to discard", player1->deck->discard_count == initial_discard + 1);

    /* Find the spawned soldier */
    CardInstance* spawned = NULL;
    for (int i = 0; i < player1->deck->discard_count; i++) {
        if (player1->deck->discard[i]->type == us_soldier) {
            spawned = player1->deck->discard[i];
            break;
        }
    }
    TEST("Spawned unit exists", spawned != NULL);
    TEST("Spawned is soldier type", spawned && spawned->type == us_soldier);
    TEST("Spawned has unique ID", spawned && spawned->instance_id != NULL);

    /* ===== Test 6: Spawn multiple units ===== */
    int before_multi = player1->deck->discard_count;
    Effect spawn_multi = {
        .type = EFFECT_SPAWN,
        .value = 3,
        .target_card_id = "us_soldier"
    };
    effects_execute(game, player1, &spawn_multi, NULL);
    TEST("Spawn 3 units", player1->deck->discard_count == before_multi + 3);

    /* ===== Test 7: Spawn with unknown type does nothing ===== */
    int before_unknown = player1->deck->discard_count;
    Effect spawn_unknown = {
        .type = EFFECT_SPAWN,
        .value = 1,
        .target_card_id = "unknown_type"
    };
    effects_execute(game, player1, &spawn_unknown, NULL);
    TEST("Unknown spawn type ignored", player1->deck->discard_count == before_unknown);

    /* ===== Test 8: Upgrade card in discard pile ===== */
    /* Move a card to discard for testing */
    CardInstance* discard_card = player1->deck->hand[0];
    const char* discard_id = discard_card->instance_id;
    deck_add_to_discard(player1->deck, discard_card);
    player1->deck->hand[0] = player1->deck->hand[--player1->deck->hand_count];

    Effect upgrade_discard = { .type = EFFECT_UPGRADE_ATTACK, .value = 5 };
    effects_execute(game, player1, &upgrade_discard, NULL);
    int orig_bonus = discard_card->attack_bonus;
    resolved = game_resolve_upgrade(game, discard_id);
    TEST("Upgrade discard card resolved", resolved);
    TEST("Discard card upgraded", discard_card->attack_bonus == orig_bonus + 5);

    /* Clean up - game_free handles us_soldier since it was registered */
    game_free(game);
    card_type_free(us_scout);
    card_type_free(us_viper);
    card_type_free(us_explorer);
    /* us_soldier freed by game_free since we registered it */
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
    test_base_zones();
    test_spawning_mechanics();
    test_effects_module();
    test_autodraw_module();
    test_card_manipulation_effects();
    test_special_effects();
    test_upgrade_spawn_effects();

    printf("\n=====================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
/* }}} */
