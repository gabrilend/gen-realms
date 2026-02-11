/* 05-websocket.h - WebSocket Handler for Game Communication
 *
 * Provides WebSocket functionality for real-time bidirectional communication
 * with browser clients. Integrates with the protocol layer (04-protocol) for
 * message parsing and handling.
 *
 * Uses libwebsockets to upgrade HTTP connections to WebSocket and manages
 * per-connection state for game sessions.
 *
 * Dependencies: libwebsockets, 04-protocol
 */

#ifndef SYMBELINE_WEBSOCKET_H
#define SYMBELINE_WEBSOCKET_H

#include <stdbool.h>
#include <stddef.h>
#include "../core/05-game.h"

/* Forward declarations */
struct lws;
struct lws_protocols;

/* ========================================================================== */
/*                              Constants                                      */
/* ========================================================================== */

/* {{{ Constants */
#define WS_MAX_CONNECTIONS 64
#define WS_SEND_BUFFER_SIZE 65536
#define WS_RECV_BUFFER_SIZE 65536
#define WS_PROTOCOL_NAME "symbeline-game"
/* }}} */

/* ========================================================================== */
/*                              Structures                                     */
/* ========================================================================== */

/* {{{ WSConnection
 * Per-connection state for WebSocket clients.
 * Created when a WebSocket connection is established.
 */
typedef struct WSConnection {
    struct lws* wsi;            /* libwebsockets instance */
    int player_id;              /* Player ID in game (-1 if not joined) */
    int game_id;                /* Game session ID (-1 if not in game) */
    bool authenticated;         /* Has player completed join handshake */

    /* Send queue - messages waiting to be sent */
    char* send_buffer;          /* Buffer for pending message */
    size_t send_len;            /* Length of pending message */
    bool send_pending;          /* Message waiting to send */

    /* Connection metadata */
    char remote_addr[64];       /* Remote IP address */
    int index;                  /* Index in connection pool */
} WSConnection;
/* }}} */

/* {{{ WSContext
 * WebSocket server context managing all connections.
 * Provides connection pool and broadcast capabilities.
 */
typedef struct {
    WSConnection* connections[WS_MAX_CONNECTIONS];
    int connection_count;
    Game* game;                 /* Reference to active game (single-game mode) */
    void* user_data;            /* Optional user context */
} WSContext;
/* }}} */

/* ========================================================================== */
/*                              Context Management                             */
/* ========================================================================== */

/* {{{ ws_context_create
 * Creates a new WebSocket context.
 * Returns NULL on allocation failure.
 */
WSContext* ws_context_create(void);
/* }}} */

/* {{{ ws_context_destroy
 * Frees the WebSocket context and all connections.
 * Safe to call with NULL.
 */
void ws_context_destroy(WSContext* ctx);
/* }}} */

/* {{{ ws_context_set_game
 * Associates a game with the WebSocket context.
 * Used for single-game server mode.
 */
void ws_context_set_game(WSContext* ctx, Game* game);
/* }}} */

/* ========================================================================== */
/*                              Connection Management                          */
/* ========================================================================== */

/* {{{ ws_connection_create
 * Creates a new connection in the context's pool.
 * Called during LWS_CALLBACK_ESTABLISHED.
 * Returns NULL if pool is full or on allocation failure.
 */
WSConnection* ws_connection_create(WSContext* ctx, struct lws* wsi);
/* }}} */

/* {{{ ws_connection_destroy
 * Removes and frees a connection from the pool.
 * Called during LWS_CALLBACK_CLOSED.
 */
void ws_connection_destroy(WSContext* ctx, WSConnection* conn);
/* }}} */

/* {{{ ws_connection_find_by_wsi
 * Finds a connection by its libwebsockets instance.
 * Returns NULL if not found.
 */
WSConnection* ws_connection_find_by_wsi(WSContext* ctx, struct lws* wsi);
/* }}} */

/* {{{ ws_connection_find_by_player
 * Finds a connection by player ID.
 * Returns NULL if not found.
 */
WSConnection* ws_connection_find_by_player(WSContext* ctx, int player_id);
/* }}} */

/* ========================================================================== */
/*                              Message Sending                                */
/* ========================================================================== */

/* {{{ ws_send
 * Queues a JSON message to be sent to a specific connection.
 * The message is copied internally.
 * Returns true on success, false if send buffer is full.
 */
bool ws_send(WSConnection* conn, const char* json);
/* }}} */

/* {{{ ws_send_to_player
 * Sends a JSON message to a player by ID.
 * Returns true if player found and message queued.
 */
bool ws_send_to_player(WSContext* ctx, int player_id, const char* json);
/* }}} */

/* {{{ ws_broadcast
 * Sends a JSON message to all connected clients.
 * Optionally excludes one player (set exclude_player to -1 to send to all).
 */
void ws_broadcast(WSContext* ctx, const char* json, int exclude_player);
/* }}} */

/* {{{ ws_broadcast_gamestate
 * Broadcasts filtered gamestate to all players.
 * Each player receives their own view (hidden information filtered).
 */
void ws_broadcast_gamestate(WSContext* ctx, Game* game);
/* }}} */

/* ========================================================================== */
/*                              Protocol Integration                           */
/* ========================================================================== */

/* {{{ ws_get_protocols
 * Returns the libwebsockets protocol definition for WebSocket.
 * Used when creating the lws context alongside HTTP.
 */
const struct lws_protocols* ws_get_protocol(void);
/* }}} */

/* {{{ ws_handle_message
 * Processes an incoming WebSocket message.
 * Parses JSON, dispatches to protocol handlers.
 * Called from the WebSocket callback on LWS_CALLBACK_RECEIVE.
 */
void ws_handle_message(WSContext* ctx, WSConnection* conn,
                       const char* data, size_t len);
/* }}} */

/* ========================================================================== */
/*                              Utility Functions                              */
/* ========================================================================== */

/* {{{ ws_get_connection_count
 * Returns the number of active WebSocket connections.
 */
int ws_get_connection_count(const WSContext* ctx);
/* }}} */

/* {{{ ws_get_authenticated_count
 * Returns the number of authenticated (joined) players.
 */
int ws_get_authenticated_count(const WSContext* ctx);
/* }}} */

#endif /* SYMBELINE_WEBSOCKET_H */
