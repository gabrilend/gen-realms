/* 06-connections.h - Unified Connection Manager
 *
 * Provides a unified interface for managing both WebSocket and SSH connections.
 * Abstracts over transport differences to allow the game logic to work with
 * connections without knowing the underlying protocol.
 *
 * Features:
 * - Connection registry with auto-incrementing IDs
 * - Player/game assignment tracking
 * - Unified message sending across transports
 * - Game-scoped broadcasting
 *
 * Dependencies: 03-ssh, 05-websocket
 */

#ifndef SYMBELINE_CONNECTIONS_H
#define SYMBELINE_CONNECTIONS_H

#include <stdbool.h>
#include <stddef.h>

/* Forward declarations to avoid circular dependencies */
typedef struct WSConnection WSConnection;
typedef struct SSHConnection SSHConnection;

/* ========================================================================== */
/*                              Constants                                      */
/* ========================================================================== */

/* {{{ Constants */
#define CONN_MAX_CONNECTIONS 96  /* WS_MAX_CONNECTIONS + SSH_MAX_CONNECTIONS */
#define CONN_INVALID_ID -1
/* }}} */

/* ========================================================================== */
/*                              Type Definitions                               */
/* ========================================================================== */

/* {{{ ConnectionType
 * Identifies the transport protocol for a connection.
 */
typedef enum {
    CONN_TYPE_WEBSOCKET,
    CONN_TYPE_SSH
} ConnectionType;
/* }}} */

/* {{{ Connection
 * Unified connection handle wrapping either WebSocket or SSH.
 */
typedef struct Connection {
    int id;                     /* Unique connection ID (auto-assigned) */
    ConnectionType type;        /* Transport type */

    /* Transport-specific handle (only one is valid based on type) */
    union {
        WSConnection* ws;
        SSHConnection* ssh;
    } handle;

    /* Game state (managed by connection manager) */
    int player_id;              /* Player ID (-1 if not assigned) */
    int game_id;                /* Game session ID (-1 if not in game) */
    bool active;                /* Connection is live */
} Connection;
/* }}} */

/* {{{ ConnectionRegistry
 * Central registry for all active connections.
 */
typedef struct {
    Connection connections[CONN_MAX_CONNECTIONS];
    int count;                  /* Number of active connections */
    int next_id;                /* Next ID to assign */
} ConnectionRegistry;
/* }}} */

/* ========================================================================== */
/*                              Registry Management                            */
/* ========================================================================== */

/* {{{ conn_registry_create
 * Creates a new connection registry.
 * Returns NULL on allocation failure.
 */
ConnectionRegistry* conn_registry_create(void);
/* }}} */

/* {{{ conn_registry_destroy
 * Frees the connection registry.
 * Does NOT close underlying connections - caller must do that.
 * Safe to call with NULL.
 */
void conn_registry_destroy(ConnectionRegistry* registry);
/* }}} */

/* {{{ conn_registry_clear
 * Removes all connections from the registry without freeing it.
 * Useful for server restart.
 */
void conn_registry_clear(ConnectionRegistry* registry);
/* }}} */

/* ========================================================================== */
/*                              Connection Registration                        */
/* ========================================================================== */

/* {{{ conn_register_ws
 * Registers a WebSocket connection in the registry.
 * Returns the assigned connection ID, or CONN_INVALID_ID on failure.
 */
int conn_register_ws(ConnectionRegistry* registry, WSConnection* ws);
/* }}} */

/* {{{ conn_register_ssh
 * Registers an SSH connection in the registry.
 * Returns the assigned connection ID, or CONN_INVALID_ID on failure.
 */
int conn_register_ssh(ConnectionRegistry* registry, SSHConnection* ssh);
/* }}} */

/* {{{ conn_unregister
 * Removes a connection from the registry by ID.
 * Does NOT close the underlying connection.
 */
void conn_unregister(ConnectionRegistry* registry, int conn_id);
/* }}} */

/* ========================================================================== */
/*                              Connection Lookup                              */
/* ========================================================================== */

/* {{{ conn_get
 * Retrieves a connection by its ID.
 * Returns NULL if not found or inactive.
 */
Connection* conn_get(ConnectionRegistry* registry, int conn_id);
/* }}} */

/* {{{ conn_find_by_player
 * Finds a connection by player ID.
 * Returns NULL if no connection has that player assigned.
 */
Connection* conn_find_by_player(ConnectionRegistry* registry, int player_id);
/* }}} */

/* {{{ conn_find_by_ws
 * Finds a connection by its WebSocket handle.
 * Returns NULL if not found.
 */
Connection* conn_find_by_ws(ConnectionRegistry* registry, WSConnection* ws);
/* }}} */

/* {{{ conn_find_by_ssh
 * Finds a connection by its SSH handle.
 * Returns NULL if not found.
 */
Connection* conn_find_by_ssh(ConnectionRegistry* registry, SSHConnection* ssh);
/* }}} */

/* ========================================================================== */
/*                              Player Assignment                              */
/* ========================================================================== */

/* {{{ conn_assign_player
 * Assigns a player and game to a connection.
 * Returns true on success, false if connection not found.
 */
bool conn_assign_player(ConnectionRegistry* registry, int conn_id,
                        int player_id, int game_id);
/* }}} */

/* {{{ conn_clear_player
 * Removes player/game assignment from a connection.
 * Returns true on success, false if connection not found.
 */
bool conn_clear_player(ConnectionRegistry* registry, int conn_id);
/* }}} */

/* ========================================================================== */
/*                              Message Sending                                */
/* ========================================================================== */

/* {{{ conn_send
 * Sends a message to a specific connection.
 * Automatically formats for the connection type:
 * - WebSocket: sends as JSON
 * - SSH: sends as formatted terminal output
 * Returns true if message was sent/queued successfully.
 */
bool conn_send(ConnectionRegistry* registry, int conn_id, const char* message);
/* }}} */

/* {{{ conn_send_to_player
 * Sends a message to a player by player ID.
 * Returns true if player found and message sent.
 */
bool conn_send_to_player(ConnectionRegistry* registry, int player_id,
                         const char* message);
/* }}} */

/* {{{ conn_broadcast_game
 * Sends a message to all connections in a specific game.
 * Optionally excludes one player (set exclude_player to -1 to include all).
 * Returns the number of connections that received the message.
 */
int conn_broadcast_game(ConnectionRegistry* registry, int game_id,
                        const char* message, int exclude_player);
/* }}} */

/* {{{ conn_broadcast_all
 * Sends a message to all active connections.
 * Returns the number of connections that received the message.
 */
int conn_broadcast_all(ConnectionRegistry* registry, const char* message);
/* }}} */

/* ========================================================================== */
/*                              Statistics                                     */
/* ========================================================================== */

/* {{{ conn_count
 * Returns the total number of active connections.
 */
int conn_count(const ConnectionRegistry* registry);
/* }}} */

/* {{{ conn_count_in_game
 * Returns the number of connections in a specific game.
 */
int conn_count_in_game(const ConnectionRegistry* registry, int game_id);
/* }}} */

/* {{{ conn_count_by_type
 * Returns the number of connections of a specific type.
 */
int conn_count_by_type(const ConnectionRegistry* registry, ConnectionType type);
/* }}} */

#endif /* SYMBELINE_CONNECTIONS_H */
