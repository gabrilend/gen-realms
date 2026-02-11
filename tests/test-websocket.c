/* test-websocket.c - WebSocket Handler Tests (2-003)
 *
 * Tests for WebSocket context and connection management.
 * Note: Full integration tests with actual WebSocket connections
 * would require a test client, so we focus on unit testing
 * the context/connection management functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../src/net/05-websocket.h"
#include "../src/net/04-protocol.h"
#include "../src/core/05-game.h"

/* Forward declaration - we don't need full lws_protocols details for tests */
struct lws_protocols;

/* Test counters */
static int tests_passed = 0;
static int tests_failed = 0;

/* {{{ test macros */
#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  "); \
    test_##name(); \
} while(0)

#define ASSERT(cond, msg) do { \
    if (cond) { \
        printf("[PASS] %s\n", msg); \
        tests_passed++; \
    } else { \
        printf("[FAIL] %s\n", msg); \
        tests_failed++; \
    } \
} while(0)
/* }}} */

/* ========================================================================== */
/*                              Context Tests                                  */
/* ========================================================================== */

/* {{{ test_context_create */
TEST(context_create) {
    WSContext* ctx = ws_context_create();
    ASSERT(ctx != NULL, "Context created");
    ASSERT(ws_get_connection_count(ctx) == 0, "Initial connection count is 0");
    ASSERT(ws_get_authenticated_count(ctx) == 0, "Initial authenticated count is 0");
    ws_context_destroy(ctx);
}
/* }}} */

/* {{{ test_context_destroy_null */
TEST(context_destroy_null) {
    /* Should not crash */
    ws_context_destroy(NULL);
    ASSERT(1, "Destroy NULL context doesn't crash");
}
/* }}} */

/* {{{ test_context_set_game */
TEST(context_set_game) {
    WSContext* ctx = ws_context_create();

    /* Create a simple game for testing */
    Game* game = game_create(2);
    ASSERT(game != NULL, "Game created");

    ws_context_set_game(ctx, game);
    /* No getter for game, but should not crash */
    ASSERT(1, "Set game doesn't crash");

    ws_context_destroy(ctx);
    game_free(game);
}
/* }}} */

/* ========================================================================== */
/*                          Connection Management Tests                        */
/* ========================================================================== */

/* Note: We can't test actual WebSocket connections without a real lws context,
 * but we can test the find functions with NULL values */

/* {{{ test_connection_find_null */
TEST(connection_find_null) {
    WSContext* ctx = ws_context_create();

    WSConnection* conn = ws_connection_find_by_wsi(ctx, NULL);
    ASSERT(conn == NULL, "Find by NULL wsi returns NULL");

    conn = ws_connection_find_by_player(ctx, 0);
    ASSERT(conn == NULL, "Find non-existent player returns NULL");

    conn = ws_connection_find_by_player(ctx, -1);
    ASSERT(conn == NULL, "Find player -1 returns NULL");

    conn = ws_connection_find_by_player(NULL, 0);
    ASSERT(conn == NULL, "Find in NULL context returns NULL");

    ws_context_destroy(ctx);
}
/* }}} */

/* ========================================================================== */
/*                          Send Function Tests                                */
/* ========================================================================== */

/* {{{ test_send_null */
TEST(send_null) {
    /* All should handle NULL gracefully */
    bool result = ws_send(NULL, "test");
    ASSERT(result == false, "Send to NULL connection returns false");

    WSContext* ctx = ws_context_create();

    result = ws_send_to_player(ctx, 0, "test");
    ASSERT(result == false, "Send to non-existent player returns false");

    /* Broadcast to empty context should not crash */
    ws_broadcast(ctx, "test", -1);
    ASSERT(1, "Broadcast to empty context doesn't crash");

    ws_broadcast(NULL, "test", -1);
    ASSERT(1, "Broadcast to NULL context doesn't crash");

    ws_context_destroy(ctx);
}
/* }}} */

/* {{{ test_broadcast_gamestate_null */
TEST(broadcast_gamestate_null) {
    WSContext* ctx = ws_context_create();

    /* Should not crash with NULL game */
    ws_broadcast_gamestate(ctx, NULL);
    ASSERT(1, "Broadcast gamestate with NULL game doesn't crash");

    ws_broadcast_gamestate(NULL, NULL);
    ASSERT(1, "Broadcast gamestate with NULL context doesn't crash");

    ws_context_destroy(ctx);
}
/* }}} */

/* ========================================================================== */
/*                          Protocol Integration Tests                         */
/* ========================================================================== */

/* {{{ test_get_protocol */
TEST(get_protocol) {
    const struct lws_protocols* proto = ws_get_protocol();
    ASSERT(proto != NULL, "Protocol returned");
    /* Note: Cannot access internal lws_protocols fields without full header */
    /* Just verify the function returns non-NULL */
}
/* }}} */

/* {{{ test_handle_message_null */
TEST(handle_message_null) {
    WSContext* ctx = ws_context_create();

    /* Should handle NULL gracefully */
    ws_handle_message(NULL, NULL, NULL, 0);
    ASSERT(1, "Handle message with all NULLs doesn't crash");

    ws_handle_message(ctx, NULL, "test", 4);
    ASSERT(1, "Handle message with NULL connection doesn't crash");

    ws_handle_message(ctx, NULL, NULL, 0);
    ASSERT(1, "Handle message with NULL data doesn't crash");

    ws_context_destroy(ctx);
}
/* }}} */

/* ========================================================================== */
/*                          Utility Function Tests                             */
/* ========================================================================== */

/* {{{ test_connection_count */
TEST(connection_count) {
    int count = ws_get_connection_count(NULL);
    ASSERT(count == 0, "NULL context returns 0 connections");

    WSContext* ctx = ws_context_create();
    count = ws_get_connection_count(ctx);
    ASSERT(count == 0, "Empty context returns 0 connections");

    ws_context_destroy(ctx);
}
/* }}} */

/* {{{ test_authenticated_count */
TEST(authenticated_count) {
    int count = ws_get_authenticated_count(NULL);
    ASSERT(count == 0, "NULL context returns 0 authenticated");

    WSContext* ctx = ws_context_create();
    count = ws_get_authenticated_count(ctx);
    ASSERT(count == 0, "Empty context returns 0 authenticated");

    ws_context_destroy(ctx);
}
/* }}} */

/* ========================================================================== */
/*                          Constants Tests                                    */
/* ========================================================================== */

/* {{{ test_constants */
TEST(constants) {
    ASSERT(WS_MAX_CONNECTIONS > 0, "WS_MAX_CONNECTIONS is positive");
    ASSERT(WS_SEND_BUFFER_SIZE > 0, "WS_SEND_BUFFER_SIZE is positive");
    ASSERT(WS_RECV_BUFFER_SIZE > 0, "WS_RECV_BUFFER_SIZE is positive");
    ASSERT(WS_PROTOCOL_NAME != NULL, "WS_PROTOCOL_NAME is defined");
    ASSERT(strlen(WS_PROTOCOL_NAME) > 0, "WS_PROTOCOL_NAME is non-empty");
}
/* }}} */

/* ========================================================================== */
/*                              Main                                           */
/* ========================================================================== */

int main(void) {
    printf("Symbeline Realms - WebSocket Handler Tests (2-003)\n");
    printf("===================================================\n\n");

    printf("=== Context Tests ===\n");
    RUN_TEST(context_create);
    RUN_TEST(context_destroy_null);
    RUN_TEST(context_set_game);

    printf("\n=== Connection Management Tests ===\n");
    RUN_TEST(connection_find_null);

    printf("\n=== Send Function Tests ===\n");
    RUN_TEST(send_null);
    RUN_TEST(broadcast_gamestate_null);

    printf("\n=== Protocol Integration Tests ===\n");
    RUN_TEST(get_protocol);
    RUN_TEST(handle_message_null);

    printf("\n=== Utility Function Tests ===\n");
    RUN_TEST(connection_count);
    RUN_TEST(authenticated_count);

    printf("\n=== Constants Tests ===\n");
    RUN_TEST(constants);

    printf("\n===================================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
