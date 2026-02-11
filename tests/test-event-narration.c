/*
 * test-event-narration.c - Tests for Event Narration Module
 *
 * Validates event narration prompt generation, intensity calculation,
 * and proper handling of all game event types.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/llm/05-event-narration.h"

// {{{ Test Statistics
static int tests_run = 0;
static int tests_passed = 0;
// }}}

// {{{ Mock Data Structures
// Minimal mock structures for testing
static Player mock_player1;
static Player mock_player2;
static CardType mock_card_type;
static CardInstance mock_card;
static CardType mock_base_type;
static CardInstance mock_base;
static WorldState mock_world_state;
// }}}

// {{{ setup_mocks
static void setup_mocks(void) {
    // Setup player 1
    mock_player1.name = "Commander Vex";
    mock_player1.authority = 35;
    mock_player1.trade = 5;
    mock_player1.combat = 3;

    // Setup player 2
    mock_player2.name = "Admiral Thorne";
    mock_player2.authority = 28;
    mock_player2.trade = 2;
    mock_player2.combat = 7;

    // Setup card type
    mock_card_type.name = "Battle Cruiser";
    mock_card_type.faction = FACTION_KINGDOM;
    mock_card_type.cost = 5;
    mock_card_type.defense = 0;

    // Setup card instance
    mock_card.type = &mock_card_type;
    mock_card.damage_taken = 0;

    // Setup base type
    mock_base_type.name = "Orbital Station";
    mock_base_type.faction = FACTION_ARTIFICER;
    mock_base_type.cost = 4;
    mock_base_type.defense = 5;

    // Setup base instance
    mock_base.type = &mock_base_type;
    mock_base.damage_taken = 2;

    // Setup world state
    mock_world_state.tension = 0.5f;
    mock_world_state.event_count = 3;
}
// }}}

// {{{ test_event_create
static void test_event_create(void) {
    printf("  Testing event creation...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_CARD_PLAYED);
    assert(event != NULL);
    assert(event->type == GAME_EVENT_CARD_PLAYED);
    assert(event->actor == NULL);
    assert(event->target == NULL);
    assert(event->card == NULL);
    assert(event->damage == 0);
    assert(event->intensity == INTENSITY_MEDIUM);

    event_narration_free(event);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_event_create_all_types
static void test_event_create_all_types(void) {
    printf("  Testing creation of all event types...\n");
    tests_run++;

    for (int i = 0; i < GAME_EVENT_TYPE_COUNT; i++) {
        NarrationEvent* event = event_narration_create((GameEventType)i);
        assert(event != NULL);
        assert(event->type == (GameEventType)i);
        event_narration_free(event);
    }

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_event_type_to_string
static void test_event_type_to_string(void) {
    printf("  Testing event type to string conversion...\n");
    tests_run++;

    assert(strcmp(event_type_to_string(GAME_EVENT_CARD_PLAYED), "card played") == 0);
    assert(strcmp(event_type_to_string(GAME_EVENT_CARD_PURCHASED), "card purchased") == 0);
    assert(strcmp(event_type_to_string(GAME_EVENT_ATTACK_PLAYER), "attack on player") == 0);
    assert(strcmp(event_type_to_string(GAME_EVENT_BASE_DESTROYED), "base destroyed") == 0);
    assert(strcmp(event_type_to_string(GAME_EVENT_GAME_OVER), "game over") == 0);

    // Invalid type should return "unknown event"
    assert(strcmp(event_type_to_string((GameEventType)999), "unknown event") == 0);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_intensity_word
static void test_intensity_word(void) {
    printf("  Testing intensity word retrieval...\n");
    tests_run++;

    assert(strcmp(event_narration_get_intensity_word(INTENSITY_LOW), "quietly") == 0);
    assert(strcmp(event_narration_get_intensity_word(INTENSITY_MEDIUM), "dramatically") == 0);
    assert(strcmp(event_narration_get_intensity_word(INTENSITY_HIGH), "furiously") == 0);
    assert(strcmp(event_narration_get_intensity_word(INTENSITY_EPIC), "magnificently") == 0);

    // Invalid intensity should return default
    assert(strcmp(event_narration_get_intensity_word((NarrationIntensity)999), "dramatically") == 0);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_intensity_calculation_game_over
static void test_intensity_calculation_game_over(void) {
    printf("  Testing intensity calculation for game over...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_GAME_OVER);
    NarrationIntensity intensity = event_narration_calculate_intensity(NULL, event);

    // Game over should always be EPIC
    assert(intensity == INTENSITY_EPIC);

    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_intensity_calculation_base_destroyed
static void test_intensity_calculation_base_destroyed(void) {
    printf("  Testing intensity calculation for base destroyed...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_BASE_DESTROYED);
    NarrationIntensity intensity = event_narration_calculate_intensity(NULL, event);

    // Base destroyed should be HIGH
    assert(intensity == INTENSITY_HIGH);

    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_intensity_calculation_high_tension
static void test_intensity_calculation_high_tension(void) {
    printf("  Testing intensity calculation with high tension...\n");
    tests_run++;

    WorldState high_tension_state;
    high_tension_state.tension = 0.85f;

    NarrationEvent* event = event_narration_create(GAME_EVENT_CARD_PLAYED);
    NarrationIntensity intensity = event_narration_calculate_intensity(&high_tension_state, event);

    // High tension should elevate to HIGH
    assert(intensity == INTENSITY_HIGH);

    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_intensity_calculation_large_damage
static void test_intensity_calculation_large_damage(void) {
    printf("  Testing intensity calculation with large damage...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_ATTACK_PLAYER);
    event->damage = 15;
    NarrationIntensity intensity = event_narration_calculate_intensity(NULL, event);

    // Large damage (>=10) should be HIGH
    assert(intensity == INTENSITY_HIGH);

    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_intensity_calculation_small_damage
static void test_intensity_calculation_small_damage(void) {
    printf("  Testing intensity calculation with small damage...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_ATTACK_PLAYER);
    event->damage = 2;
    NarrationIntensity intensity = event_narration_calculate_intensity(NULL, event);

    // Small damage (<=2) should be LOW
    assert(intensity == INTENSITY_LOW);

    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_intensity_calculation_turn_transitions
static void test_intensity_calculation_turn_transitions(void) {
    printf("  Testing intensity calculation for turn transitions...\n");
    tests_run++;

    NarrationEvent* start_event = event_narration_create(GAME_EVENT_TURN_START);
    NarrationEvent* end_event = event_narration_create(GAME_EVENT_TURN_END);

    // Turn transitions should be LOW
    assert(event_narration_calculate_intensity(NULL, start_event) == INTENSITY_LOW);
    assert(event_narration_calculate_intensity(NULL, end_event) == INTENSITY_LOW);

    event_narration_free(start_event);
    event_narration_free(end_event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_card_played
static void test_build_card_played(void) {
    printf("  Testing card played narration build...\n");
    tests_run++;

    char* prompt = event_narration_build_card_played(&mock_player1, &mock_card);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);

    // Should contain player name and card name
    assert(strstr(prompt, "Commander Vex") != NULL);
    assert(strstr(prompt, "Battle Cruiser") != NULL);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_card_played_null
static void test_build_card_played_null(void) {
    printf("  Testing card played narration with NULL...\n");
    tests_run++;

    char* prompt = event_narration_build_card_played(NULL, NULL);
    assert(prompt != NULL);
    assert(strcmp(prompt, "A card is played.") == 0);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_purchase
static void test_build_purchase(void) {
    printf("  Testing purchase narration build...\n");
    tests_run++;

    char* prompt = event_narration_build_purchase(&mock_player1, &mock_card, 5);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);

    // Should contain player name, card name, and cost
    assert(strstr(prompt, "Commander Vex") != NULL);
    assert(strstr(prompt, "Battle Cruiser") != NULL);
    assert(strstr(prompt, "5 trade") != NULL);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_attack
static void test_build_attack(void) {
    printf("  Testing attack narration build...\n");
    tests_run++;

    char* prompt = event_narration_build_attack(&mock_player1, &mock_player2, 7, 21);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);

    // Should contain both player names
    assert(strstr(prompt, "Commander Vex") != NULL || strstr(prompt, "Vex") != NULL);
    assert(strstr(prompt, "Admiral Thorne") != NULL || strstr(prompt, "Thorne") != NULL);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_base_attack
static void test_build_base_attack(void) {
    printf("  Testing base attack narration build...\n");
    tests_run++;

    char* prompt = event_narration_build_base_attack(&mock_player1, &mock_player2,
                                                      &mock_base, 3);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);

    // Should contain base name
    assert(strstr(prompt, "Orbital Station") != NULL);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_base_destroyed
static void test_build_base_destroyed(void) {
    printf("  Testing base destroyed narration build...\n");
    tests_run++;

    char* prompt = event_narration_build_base_destroyed(&mock_player2, &mock_base);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);

    // Should contain owner name and base name
    assert(strstr(prompt, "Admiral Thorne") != NULL);
    assert(strstr(prompt, "Orbital Station") != NULL);
    assert(strstr(prompt, "destroyed") != NULL);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_turn_start
static void test_build_turn_start(void) {
    printf("  Testing turn start narration build...\n");
    tests_run++;

    char* prompt = event_narration_build_turn_start(&mock_player1, 5);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);

    // Should contain player name and turn number
    assert(strstr(prompt, "Commander Vex") != NULL);
    assert(strstr(prompt, "Turn 5") != NULL);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_turn_end
static void test_build_turn_end(void) {
    printf("  Testing turn end narration build...\n");
    tests_run++;

    char* prompt = event_narration_build_turn_end(&mock_player1, 8, 5, 3);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);

    // Should contain player name
    assert(strstr(prompt, "Commander Vex") != NULL || strstr(prompt, "Vex") != NULL);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_game_over
static void test_build_game_over(void) {
    printf("  Testing game over narration build...\n");
    tests_run++;

    char* prompt = event_narration_build_game_over(&mock_player1, &mock_player2, 35);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);

    // Should contain VICTORY and both players
    assert(strstr(prompt, "VICTORY") != NULL);
    assert(strstr(prompt, "Commander Vex") != NULL);
    assert(strstr(prompt, "Admiral Thorne") != NULL);
    assert(strstr(prompt, "35") != NULL);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_via_event_struct
static void test_build_via_event_struct(void) {
    printf("  Testing build via event struct dispatch...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_CARD_PLAYED);
    event->actor = &mock_player1;
    event->card = &mock_card;

    char* prompt = event_narration_build(event);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);
    assert(strstr(prompt, "Commander Vex") != NULL);

    free(prompt);
    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_attack_via_event_struct
static void test_build_attack_via_event_struct(void) {
    printf("  Testing attack build via event struct...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_ATTACK_PLAYER);
    event->actor = &mock_player1;
    event->target = &mock_player2;
    event->damage = 7;

    char* prompt = event_narration_build(event);
    assert(prompt != NULL);
    assert(strlen(prompt) > 0);

    free(prompt);
    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_build_null_event
static void test_build_null_event(void) {
    printf("  Testing build with NULL event...\n");
    tests_run++;

    char* prompt = event_narration_build(NULL);
    assert(prompt != NULL);
    assert(strcmp(prompt, "Something happens.") == 0);

    free(prompt);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_all_event_types_via_dispatch
static void test_all_event_types_via_dispatch(void) {
    printf("  Testing all event types via dispatch...\n");
    tests_run++;

    // Test each event type produces a non-empty result
    GameEventType types[] = {
        GAME_EVENT_CARD_PLAYED,
        GAME_EVENT_CARD_PURCHASED,
        GAME_EVENT_ATTACK_PLAYER,
        GAME_EVENT_ATTACK_BASE,
        GAME_EVENT_BASE_DESTROYED,
        GAME_EVENT_TURN_START,
        GAME_EVENT_TURN_END,
        GAME_EVENT_GAME_OVER
    };

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        NarrationEvent* event = event_narration_create(types[i]);
        event->actor = &mock_player1;
        event->target = &mock_player2;
        event->card = &mock_card;
        event->base = &mock_base;
        event->damage = 5;
        event->cost = 3;
        event->turn = 4;

        char* prompt = event_narration_build(event);
        assert(prompt != NULL);
        assert(strlen(prompt) > 0);

        free(prompt);
        event_narration_free(event);
    }

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ main
int main(void) {
    printf("=== Event Narration Tests ===\n\n");

    setup_mocks();

    printf("Event Creation Tests:\n");
    test_event_create();
    test_event_create_all_types();

    printf("\nString Conversion Tests:\n");
    test_event_type_to_string();
    test_intensity_word();

    printf("\nIntensity Calculation Tests:\n");
    test_intensity_calculation_game_over();
    test_intensity_calculation_base_destroyed();
    test_intensity_calculation_high_tension();
    test_intensity_calculation_large_damage();
    test_intensity_calculation_small_damage();
    test_intensity_calculation_turn_transitions();

    printf("\nNarration Build Tests:\n");
    test_build_card_played();
    test_build_card_played_null();
    test_build_purchase();
    test_build_attack();
    test_build_base_attack();
    test_build_base_destroyed();
    test_build_turn_start();
    test_build_turn_end();
    test_build_game_over();

    printf("\nEvent Struct Dispatch Tests:\n");
    test_build_via_event_struct();
    test_build_attack_via_event_struct();
    test_build_null_event();
    test_all_event_types_via_dispatch();

    printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);

    return tests_passed == tests_run ? 0 : 1;
}
// }}}
