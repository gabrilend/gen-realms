/*
 * test-coherence.c - Unit Tests for Narrative Coherence Recovery
 *
 * Tests coherence checking, level determination, recovery triggering,
 * world state rebuild, and logging functionality.
 */

#include "../src/llm/09-coherence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counters */
static int tests_passed = 0;
static int tests_failed = 0;

/* {{{ Test helpers */
#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing %s...", #name); \
    test_##name(); \
    printf(" PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(expr) do { \
    if (!(expr)) { \
        printf(" FAILED at line %d: %s\n", __LINE__, #expr); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))
#define ASSERT_TRUE(a) ASSERT(a)
#define ASSERT_FALSE(a) ASSERT(!(a))
#define ASSERT_NULL(a) ASSERT((a) == NULL)
#define ASSERT_NOT_NULL(a) ASSERT((a) != NULL)
/* }}} */

/* {{{ Helper: create test game */
static Game* create_test_game(void) {
    Game* game = game_create(2);
    if (!game) return NULL;

    /* Add two players by name */
    game_add_player(game, "Lady Morgaine");
    game_add_player(game, "Lord Darkon");

    game->turn_number = 5;

    return game;
}
/* }}} */

/* {{{ Helper: create test world state */
static WorldState* create_test_world_state(Game* game) {
    WorldState* state = world_state_create();
    if (!state || !game) return state;

    world_state_init_from_game(state, game);
    return state;
}
/* }}} */

/* {{{ test_manager_create_free */
TEST(manager_create_free) {
    CoherenceManager* manager = coherence_manager_create(NULL);
    ASSERT_NOT_NULL(manager);
    ASSERT_EQ(manager->total_checks, 0);
    ASSERT_EQ(manager->total_recoveries, 0);
    ASSERT_NULL(manager->llm_config);

    coherence_manager_free(manager);
}
/* }}} */

/* {{{ test_manager_with_llm */
TEST(manager_with_llm) {
    LLMConfig* config = llm_config_create();
    CoherenceManager* manager = coherence_manager_create(config);

    ASSERT_NOT_NULL(manager);
    ASSERT_NOT_NULL(manager->llm_config);

    coherence_manager_free(manager);
    llm_config_free(config);
}
/* }}} */

/* {{{ test_check_names_consistent */
TEST(check_names_consistent) {
    Game* game = create_test_game();
    ASSERT_NOT_NULL(game);

    /* Narrative with correct names */
    const char* good = "Lady Morgaine plays a card. Lord Darkon watches.";
    ASSERT_TRUE(coherence_check_names(good, game));

    game_free(game);
}
/* }}} */

/* {{{ test_check_names_generic_ref */
TEST(check_names_generic_ref) {
    Game* game = create_test_game();
    ASSERT_NOT_NULL(game);

    /* Narrative with generic reference but no name */
    const char* bad = "Player 1 attacks. The enemy falls.";
    bool result = coherence_check_names(bad, game);
    /* Should fail because "player 1" used without actual name */
    ASSERT_FALSE(result);

    game_free(game);
}
/* }}} */

/* {{{ test_check_factions_valid */
TEST(check_factions_valid) {
    const char* good = "The Merchant Guilds send their traders.";
    ASSERT_TRUE(coherence_check_factions(good));

    const char* also_good = "Beasts of the Wilds charge forward.";
    ASSERT_TRUE(coherence_check_factions(also_good));
}
/* }}} */

/* {{{ test_check_factions_contradictory */
TEST(check_factions_contradictory) {
    /* Invalid faction combination */
    const char* bad = "The Merchant Wilds army attacks.";
    ASSERT_FALSE(coherence_check_factions(bad));
}
/* }}} */

/* {{{ test_check_timeline_valid */
TEST(check_timeline_valid) {
    Game* game = create_test_game();
    WorldState* state = create_test_world_state(game);
    ASSERT_NOT_NULL(state);

    /* Turn 5, narrative mentions turn 5 */
    const char* good = "On turn 5, the battle intensifies.";
    ASSERT_TRUE(coherence_check_timeline(good, state));

    /* Turn 5, narrative mentions turn 6 (close enough) */
    const char* close = "Turn 6 begins with renewed vigor.";
    ASSERT_TRUE(coherence_check_timeline(close, state));

    world_state_free(state);
    game_free(game);
}
/* }}} */

/* {{{ test_check_timeline_invalid */
TEST(check_timeline_invalid) {
    Game* game = create_test_game();
    WorldState* state = create_test_world_state(game);
    ASSERT_NOT_NULL(state);

    /* Turn 5, narrative mentions turn 20 (way off) */
    const char* bad = "After turn 20 of this epic battle...";
    ASSERT_FALSE(coherence_check_timeline(bad, state));

    world_state_free(state);
    game_free(game);
}
/* }}} */

/* {{{ test_check_authority_plausible */
TEST(check_authority_plausible) {
    Game* game = create_test_game();
    ASSERT_NOT_NULL(game);

    /* Players have default 50 authority */
    const char* good = "With 48 authority remaining, she presses on.";
    ASSERT_TRUE(coherence_check_authority(good, game));

    game_free(game);
}
/* }}} */

/* {{{ test_check_authority_implausible */
TEST(check_authority_implausible) {
    Game* game = create_test_game();
    ASSERT_NOT_NULL(game);

    /* 500 authority is way too high */
    const char* bad = "Her 500 authority makes her unstoppable.";
    ASSERT_FALSE(coherence_check_authority(bad, game));

    game_free(game);
}
/* }}} */

/* {{{ test_check_full_coherent */
TEST(check_full_coherent) {
    Game* game = create_test_game();
    WorldState* state = create_test_world_state(game);
    CoherenceManager* manager = coherence_manager_create(NULL);

    const char* narrative = "Lady Morgaine summons forces from the Wilds. "
                            "Turn 5 sees the battlefield shift.";

    CoherenceCheck* check = coherence_check(manager, narrative, game, state);
    ASSERT_NOT_NULL(check);
    ASSERT_EQ(check->level, COHERENCE_OK);
    ASSERT_GT(check->overall_score, 0.7f);

    coherence_check_free(check);
    coherence_manager_free(manager);
    world_state_free(state);
    game_free(game);
}
/* }}} */

/* {{{ test_check_full_incoherent */
TEST(check_full_incoherent) {
    Game* game = create_test_game();
    WorldState* state = create_test_world_state(game);
    CoherenceManager* manager = coherence_manager_create(NULL);

    /* Multiple issues: wrong turn, implausible authority, contradictory faction */
    const char* narrative = "Player 1 with 500 authority commands the "
                            "Merchant Wilds on turn 50.";

    CoherenceCheck* check = coherence_check(manager, narrative, game, state);
    ASSERT_NOT_NULL(check);
    ASSERT_NE(check->level, COHERENCE_OK);
    ASSERT_LT(check->overall_score, 0.7f);

    coherence_check_free(check);
    coherence_manager_free(manager);
    world_state_free(state);
    game_free(game);
}
/* }}} */

/* {{{ test_rebuild_world_state */
TEST(rebuild_world_state) {
    Game* game = create_test_game();
    WorldState* state = create_test_world_state(game);

    /* Manually mess up the state */
    state->turn_number = 999;
    state->tension = 0.0f;

    /* Rebuild should fix it */
    coherence_rebuild_world_state(state, game);

    ASSERT_EQ(state->turn_number, game->turn_number);

    world_state_free(state);
    game_free(game);
}
/* }}} */

/* {{{ test_recover_generates_narrative */
TEST(recover_generates_narrative) {
    Game* game = create_test_game();
    WorldState* state = create_test_world_state(game);
    CoherenceManager* manager = coherence_manager_create(NULL);

    char* narrative = coherence_recover(manager, game, state);
    ASSERT_NOT_NULL(narrative);

    /* Should mention players */
    ASSERT_TRUE(strstr(narrative, "Lady Morgaine") != NULL ||
                strstr(narrative, "Lord Darkon") != NULL ||
                strstr(narrative, "Player") != NULL);

    free(narrative);
    coherence_manager_free(manager);
    world_state_free(state);
    game_free(game);
}
/* }}} */

/* {{{ test_recover_increments_stats */
TEST(recover_increments_stats) {
    Game* game = create_test_game();
    WorldState* state = create_test_world_state(game);
    CoherenceManager* manager = coherence_manager_create(NULL);

    ASSERT_EQ(manager->total_recoveries, 0);

    char* narrative = coherence_recover(manager, game, state);
    free(narrative);

    ASSERT_EQ(manager->total_recoveries, 1);

    coherence_manager_free(manager);
    world_state_free(state);
    game_free(game);
}
/* }}} */

/* {{{ test_should_check_third_turn */
TEST(should_check_third_turn) {
    Game* game = create_test_game();
    CoherenceManager* manager = coherence_manager_create(NULL);

    /* Turn 3, 6, 9 should trigger check */
    game->turn_number = 3;
    ASSERT_TRUE(coherence_should_check(manager, game));

    game->turn_number = 6;
    ASSERT_TRUE(coherence_should_check(manager, game));

    /* Turn 4, 5 should not (normally) */
    game->turn_number = 4;
    ASSERT_FALSE(coherence_should_check(manager, game));

    coherence_manager_free(manager);
    game_free(game);
}
/* }}} */

/* {{{ test_should_check_low_authority */
TEST(should_check_low_authority) {
    Game* game = create_test_game();
    CoherenceManager* manager = coherence_manager_create(NULL);

    /* Normal authority, turn 4 - no check */
    game->turn_number = 4;
    ASSERT_FALSE(coherence_should_check(manager, game));

    /* Low authority should trigger check */
    game->players[0]->authority = 5;
    ASSERT_TRUE(coherence_should_check(manager, game));

    coherence_manager_free(manager);
    game_free(game);
}
/* }}} */

/* {{{ test_level_names */
TEST(level_names) {
    ASSERT_TRUE(strcmp(coherence_get_level_name(COHERENCE_OK), "OK") == 0);
    ASSERT_TRUE(strcmp(coherence_get_level_name(COHERENCE_MINOR_ISSUE),
                       "Minor Issue") == 0);
    ASSERT_TRUE(strcmp(coherence_get_level_name(COHERENCE_MAJOR_ISSUE),
                       "Major Issue") == 0);
    ASSERT_TRUE(strcmp(coherence_get_level_name(COHERENCE_RECOVERY_NEEDED),
                       "Recovery Needed") == 0);
}
/* }}} */

/* {{{ test_stats_tracking */
TEST(stats_tracking) {
    Game* game = create_test_game();
    WorldState* state = create_test_world_state(game);
    CoherenceManager* manager = coherence_manager_create(NULL);

    /* Perform some checks */
    const char* narrative = "Lady Morgaine attacks.";
    CoherenceCheck* check1 = coherence_check(manager, narrative, game, state);
    coherence_check_free(check1);

    CoherenceCheck* check2 = coherence_check(manager, narrative, game, state);
    coherence_check_free(check2);

    int checks, recoveries;
    float avg_score;
    coherence_get_stats(manager, &checks, &recoveries, &avg_score);

    ASSERT_EQ(checks, 2);
    ASSERT_EQ(recoveries, 0);

    coherence_manager_free(manager);
    world_state_free(state);
    game_free(game);
}
/* }}} */

/* {{{ test_log_entry */
TEST(log_entry) {
    CoherenceManager* manager = coherence_manager_create(NULL);

    CoherenceCheck check = {
        .names_consistent = true,
        .faction_consistent = false,
        .timeline_consistent = true,
        .authority_plausible = true,
        .event_referenced = true,
        .overall_score = 0.8f,
        .level = COHERENCE_MINOR_ISSUE,
        .issue_description = strdup("Faction error.")
    };

    coherence_log_entry(manager, &check, "Test narrative...", false);

    ASSERT_EQ(manager->log_count, 1);
    ASSERT_EQ(manager->log[0].level, COHERENCE_MINOR_ISSUE);

    free(check.issue_description);
    coherence_manager_free(manager);
}
/* }}} */

/* {{{ test_get_recent_issues_empty */
TEST(get_recent_issues_empty) {
    CoherenceManager* manager = coherence_manager_create(NULL);

    char* issues = coherence_get_recent_issues(manager, 5);
    ASSERT_NOT_NULL(issues);
    ASSERT_TRUE(strstr(issues, "No coherence issues") != NULL);

    free(issues);
    coherence_manager_free(manager);
}
/* }}} */

/* {{{ test_null_handling */
TEST(null_handling) {
    /* Various null inputs should not crash */
    ASSERT_TRUE(coherence_check_names(NULL, NULL));
    ASSERT_TRUE(coherence_check_factions(NULL));
    ASSERT_TRUE(coherence_check_timeline(NULL, NULL));
    ASSERT_TRUE(coherence_check_authority(NULL, NULL));

    CoherenceCheck* check = coherence_check(NULL, NULL, NULL, NULL);
    ASSERT_NOT_NULL(check);
    coherence_check_free(check);

    char* recovery = coherence_recover(NULL, NULL, NULL);
    ASSERT_NULL(recovery);

    ASSERT_FALSE(coherence_should_check(NULL, NULL));

    coherence_manager_free(NULL);
    coherence_check_free(NULL);
}
/* }}} */

/* {{{ test_consecutive_issues_tracking */
TEST(consecutive_issues_tracking) {
    Game* game = create_test_game();
    WorldState* state = create_test_world_state(game);
    CoherenceManager* manager = coherence_manager_create(NULL);

    /* Good narrative - resets consecutive */
    const char* good = "Lady Morgaine plays from the Wilds.";
    CoherenceCheck* check1 = coherence_check(manager, good, game, state);
    coherence_check_free(check1);
    ASSERT_EQ(manager->consecutive_issues, 0);

    /* Bad narrative - increments consecutive */
    const char* bad = "Player 1 with 999 authority on turn 50.";
    CoherenceCheck* check2 = coherence_check(manager, bad, game, state);
    coherence_check_free(check2);
    ASSERT_GT(manager->consecutive_issues, 0);

    coherence_manager_free(manager);
    world_state_free(state);
    game_free(game);
}
/* }}} */

/* {{{ main */
int main(void) {
    printf("=== Coherence Recovery Tests ===\n\n");

    /* Initialize LLM for tests that need config */
    llm_init();

    printf("Manager tests:\n");
    RUN_TEST(manager_create_free);
    RUN_TEST(manager_with_llm);

    printf("\nName checking tests:\n");
    RUN_TEST(check_names_consistent);
    RUN_TEST(check_names_generic_ref);

    printf("\nFaction checking tests:\n");
    RUN_TEST(check_factions_valid);
    RUN_TEST(check_factions_contradictory);

    printf("\nTimeline checking tests:\n");
    RUN_TEST(check_timeline_valid);
    RUN_TEST(check_timeline_invalid);

    printf("\nAuthority checking tests:\n");
    RUN_TEST(check_authority_plausible);
    RUN_TEST(check_authority_implausible);

    printf("\nFull coherence check tests:\n");
    RUN_TEST(check_full_coherent);
    RUN_TEST(check_full_incoherent);

    printf("\nRecovery tests:\n");
    RUN_TEST(rebuild_world_state);
    RUN_TEST(recover_generates_narrative);
    RUN_TEST(recover_increments_stats);

    printf("\nShould-check tests:\n");
    RUN_TEST(should_check_third_turn);
    RUN_TEST(should_check_low_authority);

    printf("\nUtility tests:\n");
    RUN_TEST(level_names);
    RUN_TEST(stats_tracking);
    RUN_TEST(log_entry);
    RUN_TEST(get_recent_issues_empty);

    printf("\nEdge case tests:\n");
    RUN_TEST(null_handling);
    RUN_TEST(consecutive_issues_tracking);

    llm_cleanup();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
/* }}} */
