/* test-core.c - Tests for core game logic modules
 *
 * Tests card creation, deck management, and player state for issues
 * 1-001, 1-002, and 1-003. Run with: make test
 */

#include "../src/core/01-card.h"
#include "../src/core/02-deck.h"
#include "../src/core/03-player.h"
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
/*                               Main                                         */
/* ========================================================================== */

/* {{{ main */
int main(void) {
    printf("Symbeline Realms - Core Module Tests\n");
    printf("=====================================\n");

    test_card_module();
    test_deck_module();
    test_player_module();

    printf("\n=====================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
/* }}} */
