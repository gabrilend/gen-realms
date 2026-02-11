/*
 * test-world-state.c - Tests for World State System
 *
 * Validates world state creation, updates, and context building.
 * Tests require mock game structures for full verification.
 * Run with: gcc -o test-world-state test-world-state.c ../src/llm/03-world-state.c
 *           ../src/llm/02-prompts.c ../src/core/05-game.c ../src/core/03-player.c
 *           ../src/core/02-deck.c ../src/core/01-card.c ../src/core/04-trade-row.c
 *           ../src/core/06-combat.c ../src/core/07-effects.c -lm && ./test-world-state
 *
 * Note: For unit testing without full dependencies, mock functions are used.
 */

#include "../src/llm/03-world-state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("Running %s...", #name); name(); printf(" PASS\n"); } while(0)

/* Mock game phase conversion for standalone testing */
const char* game_phase_to_string(GamePhase phase) {
    switch (phase) {
        case PHASE_NOT_STARTED: return "Not Started";
        case PHASE_DRAW_ORDER: return "Draw Order";
        case PHASE_MAIN: return "Main";
        case PHASE_END: return "End";
        case PHASE_GAME_OVER: return "Game Over";
        default: return "Unknown";
    }
}

/* Helper to create a minimal mock game for testing */
static Game* create_mock_game(void) {
    Game* game = malloc(sizeof(Game));
    if (game == NULL) return NULL;

    game->player_count = 2;
    game->turn_number = 5;
    game->phase = PHASE_MAIN;
    game->game_over = false;
    game->active_player = 0;

    // Create mock players
    game->players[0] = malloc(sizeof(Player));
    game->players[0]->name = strdup("Lady Morgaine");
    game->players[0]->authority = 42;
    game->players[0]->id = 1;

    game->players[1] = malloc(sizeof(Player));
    game->players[1]->name = strdup("Lord Theron");
    game->players[1]->authority = 38;
    game->players[1]->id = 2;

    for (int i = 2; i < MAX_PLAYERS; i++) {
        game->players[i] = NULL;
    }

    return game;
}

static void free_mock_game(Game* game) {
    if (game == NULL) return;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i] != NULL) {
            free(game->players[i]->name);
            free(game->players[i]);
        }
    }
    free(game);
}

// {{{ test_create
TEST(test_create) {
    WorldState* state = world_state_create();
    assert(state != NULL);
    assert(state->battlefield_description != NULL);
    assert(state->turn_number == 0);
    assert(state->event_count == 0);
    assert(state->tension == 0.0f);

    world_state_free(state);
}
// }}}

// {{{ test_free_null
TEST(test_free_null) {
    // Should not crash
    world_state_free(NULL);
}
// }}}

// {{{ test_init_from_game
TEST(test_init_from_game) {
    WorldState* state = world_state_create();
    Game* game = create_mock_game();

    world_state_init_from_game(state, game);

    assert(state->turn_number == 5);
    assert(state->player_forces[0] != NULL);
    assert(state->player_forces[1] != NULL);

    world_state_free(state);
    free_mock_game(game);
}
// }}}

// {{{ test_record_event
TEST(test_record_event) {
    WorldState* state = world_state_create();

    world_state_record_event(state, "attack", "Dire wolves strike!", 1, 3);
    assert(state->event_count == 1);

    world_state_record_event(state, "buy", "A merchant vessel acquired", 2, 4);
    assert(state->event_count == 2);

    world_state_free(state);
}
// }}}

// {{{ test_get_recent_events
TEST(test_get_recent_events) {
    WorldState* state = world_state_create();

    world_state_record_event(state, "attack", "First strike!", 1, 1);
    world_state_record_event(state, "buy", "Card acquired", 2, 2);
    world_state_record_event(state, "play", "Beast summoned", 1, 3);

    char* events = world_state_get_recent_events(state, 3);
    assert(events != NULL);
    assert(strstr(events, "First strike") != NULL);
    assert(strstr(events, "Card acquired") != NULL);
    assert(strstr(events, "Beast summoned") != NULL);

    free(events);
    world_state_free(state);
}
// }}}

// {{{ test_event_circular_buffer
TEST(test_event_circular_buffer) {
    WorldState* state = world_state_create();

    // Fill beyond capacity
    for (int i = 0; i < WORLD_STATE_MAX_EVENTS + 5; i++) {
        char desc[32];
        snprintf(desc, sizeof(desc), "Event %d", i);
        world_state_record_event(state, "test", desc, 1, i);
    }

    // Should have max events
    assert(state->event_count == WORLD_STATE_MAX_EVENTS);

    // Recent events should be the latest ones
    char* events = world_state_get_recent_events(state, 3);
    assert(events != NULL);
    // Latest events should be present
    assert(strstr(events, "Event 14") != NULL ||
           strstr(events, "Event 13") != NULL);

    free(events);
    world_state_free(state);
}
// }}}

// {{{ test_calculate_tension_low
TEST(test_calculate_tension_low) {
    WorldState* state = world_state_create();
    Game* game = create_mock_game();

    // Both players at high authority, early game
    game->players[0]->authority = 50;
    game->players[1]->authority = 50;  // Equal authority
    game->turn_number = 1;

    float tension = world_state_calculate_tension(state, game);
    // Early game with equal high authority = moderate tension
    // Closeness adds ~0.4, low danger, minimal length
    assert(tension >= 0.0f && tension <= 0.5f);

    world_state_free(state);
    free_mock_game(game);
}
// }}}

// {{{ test_calculate_tension_high
TEST(test_calculate_tension_high) {
    WorldState* state = world_state_create();
    Game* game = create_mock_game();

    // Both players at low authority, close values, late game
    game->players[0]->authority = 10;
    game->players[1]->authority = 8;
    game->turn_number = 15;

    float tension = world_state_calculate_tension(state, game);
    assert(tension >= 0.6f);

    world_state_free(state);
    free_mock_game(game);
}
// }}}

// {{{ test_update
TEST(test_update) {
    WorldState* state = world_state_create();
    Game* game = create_mock_game();

    world_state_init_from_game(state, game);

    // Simulate low authority situation
    game->players[0]->authority = 15;
    game->players[1]->authority = 12;
    game->turn_number = 10;

    world_state_update(state, game);

    assert(state->turn_number == 10);
    assert(state->tension > 0.5f);
    // Battlefield should have updated description
    assert(state->battlefield_description != NULL);

    world_state_free(state);
    free_mock_game(game);
}
// }}}

// {{{ test_to_prompt_vars
TEST(test_to_prompt_vars) {
    WorldState* state = world_state_create();
    Game* game = create_mock_game();
    PromptVars* vars = prompt_vars_create();

    world_state_to_prompt_vars(state, game, vars);

    const char* turn = prompt_vars_get(vars, "turn");
    assert(turn != NULL);
    assert(strcmp(turn, "5") == 0);

    const char* p1_name = prompt_vars_get(vars, "player1_name");
    assert(p1_name != NULL);
    assert(strcmp(p1_name, "Lady Morgaine") == 0);

    const char* p1_auth = prompt_vars_get(vars, "player1_authority");
    assert(p1_auth != NULL);
    assert(strcmp(p1_auth, "42") == 0);

    prompt_vars_free(vars);
    world_state_free(state);
    free_mock_game(game);
}
// }}}

// {{{ test_build_context
TEST(test_build_context) {
    WorldState* state = world_state_create();
    Game* game = create_mock_game();

    world_state_init_from_game(state, game);
    world_state_record_event(state, "play", "A mighty beast appears", 1, 5);

    char* context = world_state_build_context(state, game);
    assert(context != NULL);

    // Should contain game state info
    assert(strstr(context, "Turn 5") != NULL);
    assert(strstr(context, "Morgaine") != NULL);
    assert(strstr(context, "Theron") != NULL);

    // Should contain battlefield description
    assert(strstr(context, "Symbeline") != NULL);

    free(context);
    world_state_free(state);
    free_mock_game(game);
}
// }}}

// {{{ test_get_faction_name
TEST(test_get_faction_name) {
    const char* name = world_state_get_faction_name(FACTION_MERCHANT);
    assert(name != NULL);
    assert(strstr(name, "Merchant") != NULL);

    name = world_state_get_faction_name(FACTION_WILDS);
    assert(name != NULL);
    assert(strstr(name, "Wilds") != NULL);

    // Invalid faction
    name = world_state_get_faction_name(FACTION_COUNT);
    assert(name != NULL); // Should return fallback
}
// }}}

// {{{ main
int main(void) {
    printf("=== World State Tests ===\n");

    RUN_TEST(test_create);
    RUN_TEST(test_free_null);
    RUN_TEST(test_init_from_game);
    RUN_TEST(test_record_event);
    RUN_TEST(test_get_recent_events);
    RUN_TEST(test_event_circular_buffer);
    RUN_TEST(test_calculate_tension_low);
    RUN_TEST(test_calculate_tension_high);
    RUN_TEST(test_update);
    RUN_TEST(test_to_prompt_vars);
    RUN_TEST(test_build_context);
    RUN_TEST(test_get_faction_name);

    printf("\nAll tests passed!\n");
    return 0;
}
// }}}
