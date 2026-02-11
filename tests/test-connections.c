/* test-connections.c - Connection Manager Tests
 *
 * Tests the unified connection manager for registering, looking up,
 * and managing both WebSocket and SSH connections.
 *
 * Run with: make test-connections
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

/* ========================================================================== */
/*                              Stub Definitions                               */
/* ========================================================================== */

/* {{{ Stub structures
 * Minimal definitions to allow 06-connections.c to compile.
 * These match the structures expected by the connection manager.
 */

/* Stub WSConnection - matches forward declaration in 06-connections.h */
typedef struct WSConnection {
    int stub_id;
} WSConnection;

/* Stub SSHConnection - matches forward declaration in 06-connections.h */
typedef struct SSHConnection {
    int stub_id;
} SSHConnection;

/* Track send calls for testing */
static int stub_ws_send_count = 0;
static int stub_ssh_send_count = 0;

/* Stub implementations that will be linked instead of real ones */
bool ws_send(WSConnection* conn, const char* json) {
    (void)conn;
    (void)json;
    stub_ws_send_count++;
    return true;
}

int ssh_connection_send_string(SSHConnection* conn, const char* str) {
    (void)conn;
    (void)str;
    stub_ssh_send_count++;
    return (int)strlen(str);
}
/* }}} */

#include "../src/net/06-connections.h"

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
/*                              Mock Structures                                */
/* ========================================================================== */

/* {{{ Mock Connection Instances
 * Arrays of stub connections for testing.
 */
static WSConnection mock_ws_connections[8];
static SSHConnection mock_ssh_connections[8];
/* }}} */

/* ========================================================================== */
/*                              Registry Tests                                 */
/* ========================================================================== */

/* {{{ test_registry_create */
static void test_registry_create(void) {
    TEST("Registry creation");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");
    ASSERT(conn_count(registry) == 0, "New registry should be empty");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_registry_destroy_null */
static void test_registry_destroy_null(void) {
    TEST("Registry destroy NULL");

    /* Should not crash */
    conn_registry_destroy(NULL);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Registration Tests                             */
/* ========================================================================== */

/* {{{ test_register_websocket */
static void test_register_websocket(void) {
    TEST("Register WebSocket connection");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id = conn_register_ws(registry, &mock_ws_connections[0]);
    ASSERT(id != CONN_INVALID_ID, "Failed to register WebSocket");
    ASSERT(conn_count(registry) == 1, "Count should be 1");

    Connection* conn = conn_get(registry, id);
    ASSERT(conn != NULL, "Should find connection by ID");
    ASSERT(conn->type == CONN_TYPE_WEBSOCKET, "Type should be WebSocket");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_register_ssh */
static void test_register_ssh(void) {
    TEST("Register SSH connection");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id = conn_register_ssh(registry, &mock_ssh_connections[0]);
    ASSERT(id != CONN_INVALID_ID, "Failed to register SSH");
    ASSERT(conn_count(registry) == 1, "Count should be 1");

    Connection* conn = conn_get(registry, id);
    ASSERT(conn != NULL, "Should find connection by ID");
    ASSERT(conn->type == CONN_TYPE_SSH, "Type should be SSH");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_register_mixed */
static void test_register_mixed(void) {
    TEST("Register mixed WebSocket and SSH connections");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int ws_id1 = conn_register_ws(registry, &mock_ws_connections[0]);
    int ssh_id1 = conn_register_ssh(registry, &mock_ssh_connections[0]);
    int ws_id2 = conn_register_ws(registry, &mock_ws_connections[1]);
    int ssh_id2 = conn_register_ssh(registry, &mock_ssh_connections[1]);

    ASSERT(conn_count(registry) == 4, "Should have 4 connections");
    ASSERT(conn_count_by_type(registry, CONN_TYPE_WEBSOCKET) == 2, "Should have 2 WebSocket");
    ASSERT(conn_count_by_type(registry, CONN_TYPE_SSH) == 2, "Should have 2 SSH");

    /* All IDs should be unique */
    ASSERT(ws_id1 != ws_id2, "WS IDs should be unique");
    ASSERT(ssh_id1 != ssh_id2, "SSH IDs should be unique");
    ASSERT(ws_id1 != ssh_id1, "WS and SSH IDs should be unique");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_register_null */
static void test_register_null(void) {
    TEST("Register NULL handles rejected");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id1 = conn_register_ws(registry, NULL);
    int id2 = conn_register_ssh(registry, NULL);
    int id3 = conn_register_ws(NULL, &mock_ws_connections[0]);

    ASSERT(id1 == CONN_INVALID_ID, "NULL WS should be rejected");
    ASSERT(id2 == CONN_INVALID_ID, "NULL SSH should be rejected");
    ASSERT(id3 == CONN_INVALID_ID, "NULL registry should be rejected");
    ASSERT(conn_count(registry) == 0, "No connections should be registered");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Unregister Tests                               */
/* ========================================================================== */

/* {{{ test_unregister */
static void test_unregister(void) {
    TEST("Unregister connection");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id = conn_register_ws(registry, &mock_ws_connections[0]);
    ASSERT(conn_count(registry) == 1, "Should have 1 connection");

    conn_unregister(registry, id);
    ASSERT(conn_count(registry) == 0, "Should have 0 connections");
    ASSERT(conn_get(registry, id) == NULL, "Connection should not be found");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_unregister_invalid */
static void test_unregister_invalid(void) {
    TEST("Unregister invalid ID");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id = conn_register_ws(registry, &mock_ws_connections[0]);

    /* Unregister non-existent ID - should not crash */
    conn_unregister(registry, 999);
    conn_unregister(registry, -1);
    conn_unregister(NULL, id);

    ASSERT(conn_count(registry) == 1, "Original connection should remain");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Lookup Tests                                   */
/* ========================================================================== */

/* {{{ test_find_by_handle */
static void test_find_by_handle(void) {
    TEST("Find connection by handle");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int ws_id = conn_register_ws(registry, &mock_ws_connections[0]);
    int ssh_id = conn_register_ssh(registry, &mock_ssh_connections[0]);

    Connection* ws_conn = conn_find_by_ws(registry, &mock_ws_connections[0]);
    Connection* ssh_conn = conn_find_by_ssh(registry, &mock_ssh_connections[0]);

    ASSERT(ws_conn != NULL, "Should find WS connection");
    ASSERT(ws_conn->id == ws_id, "WS ID should match");
    ASSERT(ssh_conn != NULL, "Should find SSH connection");
    ASSERT(ssh_conn->id == ssh_id, "SSH ID should match");

    /* Unknown handles */
    ASSERT(conn_find_by_ws(registry, &mock_ws_connections[5]) == NULL,
           "Unknown WS should return NULL");
    ASSERT(conn_find_by_ssh(registry, &mock_ssh_connections[5]) == NULL,
           "Unknown SSH should return NULL");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Player Assignment Tests                        */
/* ========================================================================== */

/* {{{ test_assign_player */
static void test_assign_player(void) {
    TEST("Assign player to connection");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id = conn_register_ws(registry, &mock_ws_connections[0]);

    bool result = conn_assign_player(registry, id, 42, 100);
    ASSERT(result == true, "Assignment should succeed");

    Connection* conn = conn_get(registry, id);
    ASSERT(conn != NULL, "Should find connection");
    ASSERT(conn->player_id == 42, "Player ID should be 42");
    ASSERT(conn->game_id == 100, "Game ID should be 100");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_find_by_player */
static void test_find_by_player(void) {
    TEST("Find connection by player ID");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id1 = conn_register_ws(registry, &mock_ws_connections[0]);
    int id2 = conn_register_ssh(registry, &mock_ssh_connections[0]);

    conn_assign_player(registry, id1, 1, 0);
    conn_assign_player(registry, id2, 2, 0);

    Connection* player1 = conn_find_by_player(registry, 1);
    Connection* player2 = conn_find_by_player(registry, 2);

    ASSERT(player1 != NULL, "Should find player 1");
    ASSERT(player1->id == id1, "Player 1 connection ID should match");
    ASSERT(player2 != NULL, "Should find player 2");
    ASSERT(player2->id == id2, "Player 2 connection ID should match");

    /* Unknown player */
    ASSERT(conn_find_by_player(registry, 999) == NULL, "Unknown player should return NULL");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_clear_player */
static void test_clear_player(void) {
    TEST("Clear player assignment");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id = conn_register_ws(registry, &mock_ws_connections[0]);
    conn_assign_player(registry, id, 42, 100);

    bool result = conn_clear_player(registry, id);
    ASSERT(result == true, "Clear should succeed");

    Connection* conn = conn_get(registry, id);
    ASSERT(conn != NULL, "Connection should still exist");
    ASSERT(conn->player_id == -1, "Player ID should be -1");
    ASSERT(conn->game_id == -1, "Game ID should be -1");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Game Statistics Tests                          */
/* ========================================================================== */

/* {{{ test_count_in_game */
static void test_count_in_game(void) {
    TEST("Count connections in game");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id1 = conn_register_ws(registry, &mock_ws_connections[0]);
    int id2 = conn_register_ssh(registry, &mock_ssh_connections[0]);
    int id3 = conn_register_ws(registry, &mock_ws_connections[1]);

    /* Assign to different games */
    conn_assign_player(registry, id1, 0, 100);
    conn_assign_player(registry, id2, 1, 100);
    conn_assign_player(registry, id3, 0, 200);

    ASSERT(conn_count_in_game(registry, 100) == 2, "Game 100 should have 2 players");
    ASSERT(conn_count_in_game(registry, 200) == 1, "Game 200 should have 1 player");
    ASSERT(conn_count_in_game(registry, 300) == 0, "Game 300 should have 0 players");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Clear Tests                                    */
/* ========================================================================== */

/* {{{ test_registry_clear */
static void test_registry_clear(void) {
    TEST("Registry clear");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    conn_register_ws(registry, &mock_ws_connections[0]);
    conn_register_ssh(registry, &mock_ssh_connections[0]);
    conn_register_ws(registry, &mock_ws_connections[1]);

    ASSERT(conn_count(registry) == 3, "Should have 3 connections");

    conn_registry_clear(registry);

    ASSERT(conn_count(registry) == 0, "Should have 0 connections after clear");

    /* Should be able to register again */
    int id = conn_register_ws(registry, &mock_ws_connections[2]);
    ASSERT(id != CONN_INVALID_ID, "Should be able to register after clear");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Send Tests                                     */
/* ========================================================================== */

/* {{{ test_send_to_connection */
static void test_send_to_connection(void) {
    TEST("Send to specific connection");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int ws_id = conn_register_ws(registry, &mock_ws_connections[0]);
    int ssh_id = conn_register_ssh(registry, &mock_ssh_connections[0]);

    /* Reset counters */
    stub_ws_send_count = 0;
    stub_ssh_send_count = 0;

    bool ws_result = conn_send(registry, ws_id, "test message");
    bool ssh_result = conn_send(registry, ssh_id, "test message");

    ASSERT(ws_result == true, "WS send should succeed");
    ASSERT(ssh_result == true, "SSH send should succeed");
    ASSERT(stub_ws_send_count == 1, "WS send should be called once");
    ASSERT(stub_ssh_send_count == 1, "SSH send should be called once");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_send_to_player */
static void test_send_to_player(void) {
    TEST("Send to player by ID");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id = conn_register_ws(registry, &mock_ws_connections[0]);
    conn_assign_player(registry, id, 42, 100);

    stub_ws_send_count = 0;

    bool result = conn_send_to_player(registry, 42, "hello player 42");
    ASSERT(result == true, "Send to player should succeed");
    ASSERT(stub_ws_send_count == 1, "WS send should be called");

    /* Send to unknown player */
    result = conn_send_to_player(registry, 999, "hello unknown");
    ASSERT(result == false, "Send to unknown player should fail");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_broadcast_game */
static void test_broadcast_game(void) {
    TEST("Broadcast to game");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id1 = conn_register_ws(registry, &mock_ws_connections[0]);
    int id2 = conn_register_ssh(registry, &mock_ssh_connections[0]);
    int id3 = conn_register_ws(registry, &mock_ws_connections[1]);

    conn_assign_player(registry, id1, 0, 100);
    conn_assign_player(registry, id2, 1, 100);
    conn_assign_player(registry, id3, 2, 200);  /* Different game */

    stub_ws_send_count = 0;
    stub_ssh_send_count = 0;

    int sent = conn_broadcast_game(registry, 100, "game 100 message", -1);
    ASSERT(sent == 2, "Should send to 2 connections in game 100");
    ASSERT(stub_ws_send_count == 1, "1 WS in game 100");
    ASSERT(stub_ssh_send_count == 1, "1 SSH in game 100");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_broadcast_game_exclude */
static void test_broadcast_game_exclude(void) {
    TEST("Broadcast to game with exclusion");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    int id1 = conn_register_ws(registry, &mock_ws_connections[0]);
    int id2 = conn_register_ssh(registry, &mock_ssh_connections[0]);

    conn_assign_player(registry, id1, 0, 100);
    conn_assign_player(registry, id2, 1, 100);

    stub_ws_send_count = 0;
    stub_ssh_send_count = 0;

    /* Exclude player 0 (WS) */
    int sent = conn_broadcast_game(registry, 100, "message", 0);
    ASSERT(sent == 1, "Should send to 1 connection (excluding player 0)");
    ASSERT(stub_ws_send_count == 0, "WS player excluded");
    ASSERT(stub_ssh_send_count == 1, "SSH player received");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* {{{ test_broadcast_all */
static void test_broadcast_all(void) {
    TEST("Broadcast to all connections");

    ConnectionRegistry* registry = conn_registry_create();
    ASSERT(registry != NULL, "Failed to create registry");

    conn_register_ws(registry, &mock_ws_connections[0]);
    conn_register_ssh(registry, &mock_ssh_connections[0]);
    conn_register_ws(registry, &mock_ws_connections[1]);

    stub_ws_send_count = 0;
    stub_ssh_send_count = 0;

    int sent = conn_broadcast_all(registry, "global message");
    ASSERT(sent == 3, "Should send to all 3 connections");
    ASSERT(stub_ws_send_count == 2, "2 WS connections");
    ASSERT(stub_ssh_send_count == 1, "1 SSH connection");

    conn_registry_destroy(registry);
    PASS();
}
/* }}} */

/* ========================================================================== */
/*                              Main                                           */
/* ========================================================================== */

int main(void) {
    printf("\n");
    printf("==========================================\n");
    printf("  Connection Manager Tests\n");
    printf("==========================================\n\n");

    /* Initialize mock connections with IDs */
    for (int i = 0; i < 8; i++) {
        mock_ws_connections[i].stub_id = i;
        mock_ssh_connections[i].stub_id = i + 100;
    }

    /* Registry tests */
    printf("Registry Management:\n");
    test_registry_create();
    test_registry_destroy_null();
    printf("\n");

    /* Registration tests */
    printf("Connection Registration:\n");
    test_register_websocket();
    test_register_ssh();
    test_register_mixed();
    test_register_null();
    printf("\n");

    /* Unregister tests */
    printf("Connection Unregistration:\n");
    test_unregister();
    test_unregister_invalid();
    printf("\n");

    /* Lookup tests */
    printf("Connection Lookup:\n");
    test_find_by_handle();
    printf("\n");

    /* Player assignment tests */
    printf("Player Assignment:\n");
    test_assign_player();
    test_find_by_player();
    test_clear_player();
    printf("\n");

    /* Game statistics tests */
    printf("Game Statistics:\n");
    test_count_in_game();
    printf("\n");

    /* Clear tests */
    printf("Registry Clear:\n");
    test_registry_clear();
    printf("\n");

    /* Send tests */
    printf("Message Sending:\n");
    test_send_to_connection();
    test_send_to_player();
    test_broadcast_game();
    test_broadcast_game_exclude();
    test_broadcast_all();
    printf("\n");

    /* Summary */
    printf("==========================================\n");
    printf("  Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("==========================================\n\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
