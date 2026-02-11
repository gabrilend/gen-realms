/* test-sessions.c - Game Session Management Tests
 *
 * Tests the session manager for creation, player joining, lifecycle,
 * and spectator management.
 *
 * Run with: make test-sessions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../src/net/07-sessions.h"

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
/*                              Registry Tests                                 */
/* ========================================================================== */

/* {{{ test_registry_create */
static void test_registry_create(void) {
    TEST("Registry creation");

    SessionRegistry* registry = session_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");
    ASSERT(session_count(registry) == 0, "New registry should be empty");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_registry_destroy_null */
static void test_registry_destroy_null(void) {
    TEST("Registry destroy NULL");

    /* Should not crash */
    session_registry_destroy(NULL);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Session Creation Tests                         */
/* ========================================================================== */

/* {{{ test_session_create */
static void test_session_create(void) {
    TEST("Session creation");

    SessionRegistry* registry = session_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int session_id = session_create(registry, 100, "Alice", 2);
    ASSERT(session_id != SESSION_INVALID_ID, "Failed to create session");
    ASSERT(session_count(registry) == 1, "Should have 1 session");

    GameSession* session = session_get(registry, session_id);
    ASSERT(session != NULL, "Should find session");
    ASSERT(session->state == SESSION_WAITING, "Should be waiting");
    ASSERT(session->player_count == 1, "Should have 1 player (host)");
    ASSERT(session->required_players == 2, "Should require 2 players");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_session_create_invalid_players */
static void test_session_create_invalid_players(void) {
    TEST("Session creation with invalid player count");

    SessionRegistry* registry = session_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id1 = session_create(registry, 100, "Alice", 1);  /* Too few */
    int id2 = session_create(registry, 101, "Bob", 5);    /* Too many */

    ASSERT(id1 == SESSION_INVALID_ID, "Should reject 1 player");
    ASSERT(id2 == SESSION_INVALID_ID, "Should reject 5 players");
    ASSERT(session_count(registry) == 0, "No sessions should be created");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_session_destroy */
static void test_session_destroy(void) {
    TEST("Session destruction");

    SessionRegistry* registry = session_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int session_id = session_create(registry, 100, "Alice", 2);
    ASSERT(session_count(registry) == 1, "Should have 1 session");

    session_destroy(registry, session_id);
    ASSERT(session_count(registry) == 0, "Should have 0 sessions");
    ASSERT(session_get(registry, session_id) == NULL, "Session should not be found");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Player Join/Leave Tests                        */
/* ========================================================================== */

/* {{{ test_session_join */
static void test_session_join(void) {
    TEST("Player joining session");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);

    bool joined = session_join(registry, session_id, 101, "Bob");
    ASSERT(joined == true, "Bob should join");

    GameSession* session = session_get(registry, session_id);
    ASSERT(session->player_count == 2, "Should have 2 players");

    /* Session is now full */
    joined = session_join(registry, session_id, 102, "Charlie");
    ASSERT(joined == false, "Charlie should be rejected (full)");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_session_leave */
static void test_session_leave(void) {
    TEST("Player leaving session");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_join(registry, session_id, 101, "Bob");

    /* Bob leaves */
    bool left = session_leave(registry, session_id, 101);
    ASSERT(left == true, "Bob should leave");

    GameSession* session = session_get(registry, session_id);
    ASSERT(session->player_count == 1, "Should have 1 player");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_session_host_leave */
static void test_session_host_leave(void) {
    TEST("Host leaving destroys waiting session");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_join(registry, session_id, 101, "Bob");

    /* Host (Alice) leaves while WAITING - session should be destroyed */
    bool left = session_leave(registry, session_id, 100);
    ASSERT(left == true, "Alice should leave");
    ASSERT(session_count(registry) == 0, "Session should be destroyed");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Ready Status Tests                             */
/* ========================================================================== */

/* {{{ test_session_ready */
static void test_session_ready(void) {
    TEST("Player ready status");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_join(registry, session_id, 101, "Bob");

    /* Neither player ready initially */
    ASSERT(session_can_start(registry, session_id) == false, "Should not start yet");

    /* Alice readies up */
    bool result = session_set_ready(registry, session_id, 100, true);
    ASSERT(result == true, "Alice should set ready");
    ASSERT(session_can_start(registry, session_id) == false, "Still need Bob");

    /* Bob readies up */
    result = session_set_ready(registry, session_id, 101, true);
    ASSERT(result == true, "Bob should set ready");
    ASSERT(session_can_start(registry, session_id) == true, "Now can start");

    /* Bob unreadies */
    result = session_set_ready(registry, session_id, 101, false);
    ASSERT(result == true, "Bob should unready");
    ASSERT(session_can_start(registry, session_id) == false, "Cannot start now");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_session_can_start_not_enough_players */
static void test_session_can_start_not_enough_players(void) {
    TEST("Cannot start without enough players");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);

    /* Only Alice, even if ready */
    session_set_ready(registry, session_id, 100, true);
    ASSERT(session_can_start(registry, session_id) == false, "Need 2 players");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Session Start Tests                            */
/* ========================================================================== */

/* {{{ test_session_start_without_card_types
 * session_start creates a Game but game_start will fail without card types.
 * This tests that the session correctly handles game_start failure.
 */
static void test_session_start_without_card_types(void) {
    TEST("Session start (game_start fails without card types)");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_join(registry, session_id, 101, "Bob");
    session_set_ready(registry, session_id, 100, true);
    session_set_ready(registry, session_id, 101, true);

    /* Try to start - will fail because game_start needs card types */
    bool started = session_start(registry, session_id);
    ASSERT(started == false, "Should fail (no card types)");

    /* Session should still be WAITING */
    GameSession* session = session_get(registry, session_id);
    ASSERT(session != NULL, "Session should exist");
    ASSERT(session->state == SESSION_WAITING, "Should still be waiting");
    ASSERT(session->game == NULL, "Game should be cleaned up");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_session_start_not_ready */
static void test_session_start_not_ready(void) {
    TEST("Session start fails when not ready");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_join(registry, session_id, 101, "Bob");
    /* Don't mark anyone as ready */

    bool started = session_start(registry, session_id);
    ASSERT(started == false, "Should not start (not ready)");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Session Lookup Tests                           */
/* ========================================================================== */

/* {{{ test_session_find_by_conn */
static void test_session_find_by_conn(void) {
    TEST("Find session by connection");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_join(registry, session_id, 101, "Bob");

    GameSession* session1 = session_find_by_conn(registry, 100);
    GameSession* session2 = session_find_by_conn(registry, 101);
    GameSession* session3 = session_find_by_conn(registry, 999);

    ASSERT(session1 != NULL, "Should find Alice's session");
    ASSERT(session2 != NULL, "Should find Bob's session");
    ASSERT(session1 == session2, "Should be same session");
    ASSERT(session3 == NULL, "Should not find unknown conn");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_session_is_host */
static void test_session_is_host(void) {
    TEST("Check if connection is host");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_join(registry, session_id, 101, "Bob");

    GameSession* session = session_get(registry, session_id);
    ASSERT(session_is_host(session, 100) == true, "Alice is host");
    ASSERT(session_is_host(session, 101) == false, "Bob is not host");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_session_get_player_slot */
static void test_session_get_player_slot(void) {
    TEST("Get player slot by connection");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_join(registry, session_id, 101, "Bob");

    GameSession* session = session_get(registry, session_id);
    int alice_slot = session_get_player_slot(session, 100);
    int bob_slot = session_get_player_slot(session, 101);
    int unknown_slot = session_get_player_slot(session, 999);

    ASSERT(alice_slot == 0, "Alice in slot 0");
    ASSERT(bob_slot == 1, "Bob in slot 1");
    ASSERT(unknown_slot == -1, "Unknown conn returns -1");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Spectator Tests                                */
/* ========================================================================== */

/* {{{ test_spectator_add */
static void test_spectator_add(void) {
    TEST("Add spectator");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);

    bool added = session_add_spectator(registry, session_id, 200);
    ASSERT(added == true, "Spectator should be added");

    GameSession* session = session_get(registry, session_id);
    ASSERT(session->spectator_count == 1, "Should have 1 spectator");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_spectator_remove */
static void test_spectator_remove(void) {
    TEST("Remove spectator");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_add_spectator(registry, session_id, 200);

    bool removed = session_remove_spectator(registry, session_id, 200);
    ASSERT(removed == true, "Spectator should be removed");

    GameSession* session = session_get(registry, session_id);
    ASSERT(session->spectator_count == 0, "Should have 0 spectators");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_spectator_find_by_conn */
static void test_spectator_find_by_conn(void) {
    TEST("Find session by spectator connection");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_add_spectator(registry, session_id, 200);

    GameSession* session = session_find_by_conn(registry, 200);
    ASSERT(session != NULL, "Should find spectator's session");
    ASSERT(session->id == session_id, "Should be correct session");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_spectator_leave_via_session_leave */
static void test_spectator_leave_via_session_leave(void) {
    TEST("Spectator leaves via session_leave");

    SessionRegistry* registry = session_registry_create();
    int session_id = session_create(registry, 100, "Alice", 2);
    session_add_spectator(registry, session_id, 200);

    /* session_leave should handle spectators too */
    bool left = session_leave(registry, session_id, 200);
    ASSERT(left == true, "Spectator should leave");

    GameSession* session = session_get(registry, session_id);
    ASSERT(session->spectator_count == 0, "Should have 0 spectators");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Lobby Listing Tests                            */
/* ========================================================================== */

/* {{{ test_list_joinable */
static void test_list_joinable(void) {
    TEST("List joinable sessions");

    SessionRegistry* registry = session_registry_create();

    /* Create sessions */
    int s1 = session_create(registry, 100, "Alice", 2);  /* 1/2 players */
    int s2 = session_create(registry, 101, "Bob", 3);    /* 1/3 players */
    session_join(registry, s2, 102, "Charlie");          /* 2/3 players */

    int count = 0;
    SessionListEntry* entries = session_list_joinable(registry, &count);

    ASSERT(count == 2, "Should have 2 joinable sessions");
    ASSERT(entries != NULL, "Should return entries");

    /* Both sessions should be in the list */
    bool found_s1 = false, found_s2 = false;
    for (int i = 0; i < count; i++) {
        if (entries[i].id == s1) found_s1 = true;
        if (entries[i].id == s2) found_s2 = true;
    }
    ASSERT(found_s1 && found_s2, "Both sessions should be listed");

    free(entries);
    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_list_joinable_full */
static void test_list_joinable_full(void) {
    TEST("Full session not in joinable list");

    SessionRegistry* registry = session_registry_create();

    int session_id = session_create(registry, 100, "Alice", 2);
    session_join(registry, session_id, 101, "Bob");  /* Now full */

    int count = 0;
    SessionListEntry* entries = session_list_joinable(registry, &count);

    ASSERT(count == 0, "No joinable sessions");
    ASSERT(entries == NULL, "Should return NULL for empty list");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Statistics Tests                               */
/* ========================================================================== */

/* {{{ test_session_count_by_state */
static void test_session_count_by_state(void) {
    TEST("Count sessions by state");

    SessionRegistry* registry = session_registry_create();

    session_create(registry, 100, "Alice", 2);
    session_create(registry, 101, "Bob", 2);
    session_create(registry, 102, "Charlie", 2);

    ASSERT(session_count_by_state(registry, SESSION_WAITING) == 3, "3 waiting");
    ASSERT(session_count_by_state(registry, SESSION_PLAYING) == 0, "0 playing");
    ASSERT(session_count_by_state(registry, SESSION_FINISHED) == 0, "0 finished");

    session_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Utility Tests                                  */
/* ========================================================================== */

/* {{{ test_state_to_string */
static void test_state_to_string(void) {
    TEST("Session state to string");

    ASSERT(strcmp(session_state_to_string(SESSION_WAITING), "waiting") == 0, "waiting");
    ASSERT(strcmp(session_state_to_string(SESSION_PLAYING), "playing") == 0, "playing");
    ASSERT(strcmp(session_state_to_string(SESSION_FINISHED), "finished") == 0, "finished");

    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Main                                           */
/* ========================================================================== */

int main(void) {
    printf("\n");
    printf("==========================================\n");
    printf("  Session Manager Tests\n");
    printf("==========================================\n\n");

    /* Registry tests */
    printf("Registry Management:\n");
    test_registry_create();
    test_registry_destroy_null();
    printf("\n");

    /* Session creation tests */
    printf("Session Creation:\n");
    test_session_create();
    test_session_create_invalid_players();
    test_session_destroy();
    printf("\n");

    /* Player join/leave tests */
    printf("Player Join/Leave:\n");
    test_session_join();
    test_session_leave();
    test_session_host_leave();
    printf("\n");

    /* Ready status tests */
    printf("Ready Status:\n");
    test_session_ready();
    test_session_can_start_not_enough_players();
    printf("\n");

    /* Session start tests */
    printf("Session Start:\n");
    test_session_start_without_card_types();
    test_session_start_not_ready();
    printf("\n");

    /* Session lookup tests */
    printf("Session Lookup:\n");
    test_session_find_by_conn();
    test_session_is_host();
    test_session_get_player_slot();
    printf("\n");

    /* Spectator tests */
    printf("Spectators:\n");
    test_spectator_add();
    test_spectator_remove();
    test_spectator_find_by_conn();
    test_spectator_leave_via_session_leave();
    printf("\n");

    /* Lobby listing tests */
    printf("Lobby Listings:\n");
    test_list_joinable();
    test_list_joinable_full();
    printf("\n");

    /* Statistics tests */
    printf("Statistics:\n");
    test_session_count_by_state();
    printf("\n");

    /* Utility tests */
    printf("Utility:\n");
    test_state_to_string();
    printf("\n");

    /* Summary */
    printf("==========================================\n");
    printf("  Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("==========================================\n\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
