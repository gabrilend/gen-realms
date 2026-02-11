/* 06-connections.c - Unified Connection Manager Implementation
 *
 * Provides a unified interface for managing both WebSocket and SSH connections.
 * Abstracts transport details to allow game logic to work with connections
 * regardless of underlying protocol.
 *
 * Note: This file uses forward declarations and extern function declarations
 * to avoid direct header dependencies. This allows unit testing with stubs.
 */

#include "06-connections.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================== */
/*                              External Dependencies                          */
/* ========================================================================== */

/* {{{ WebSocket and SSH send functions
 * Declared as extern to allow stub implementations for testing.
 * Real implementations come from 05-websocket.c and 03-ssh.c.
 */
extern bool ws_send(WSConnection* conn, const char* json);
extern int ssh_connection_send_string(SSHConnection* conn, const char* str);
/* }}} */

/* ========================================================================== */
/*                              Registry Management                            */
/* ========================================================================== */

/* {{{ conn_registry_create */
ConnectionRegistry* conn_registry_create(void) {
    ConnectionRegistry* registry = malloc(sizeof(ConnectionRegistry));
    if (registry == NULL) {
        return NULL;
    }

    memset(registry->connections, 0, sizeof(registry->connections));
    registry->count = 0;
    registry->next_id = 0;

    return registry;
}
/* }}} */

/* {{{ conn_registry_destroy */
void conn_registry_destroy(ConnectionRegistry* registry) {
    if (registry == NULL) {
        return;
    }

    /* Clear all connections (does not close underlying handles) */
    conn_registry_clear(registry);

    free(registry);
}
/* }}} */

/* {{{ conn_registry_clear */
void conn_registry_clear(ConnectionRegistry* registry) {
    if (registry == NULL) {
        return;
    }

    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        registry->connections[i].active = false;
        registry->connections[i].handle.ws = NULL;
        registry->connections[i].handle.ssh = NULL;
        registry->connections[i].player_id = -1;
        registry->connections[i].game_id = -1;
    }

    registry->count = 0;
    /* Keep next_id to avoid ID reuse within session */
}
/* }}} */

/* ========================================================================== */
/*                              Connection Registration                        */
/* ========================================================================== */

/* {{{ find_empty_slot
 * Internal helper to find an empty slot in the registry.
 */
static int find_empty_slot(ConnectionRegistry* registry) {
    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        if (!registry->connections[i].active) {
            return i;
        }
    }
    return -1;
}
/* }}} */

/* {{{ conn_register_ws */
int conn_register_ws(ConnectionRegistry* registry, WSConnection* ws) {
    if (registry == NULL || ws == NULL) {
        return CONN_INVALID_ID;
    }

    int slot = find_empty_slot(registry);
    if (slot < 0) {
        fprintf(stderr, "ConnectionRegistry: Pool full, cannot register WebSocket\n");
        return CONN_INVALID_ID;
    }

    Connection* conn = &registry->connections[slot];
    conn->id = registry->next_id++;
    conn->type = CONN_TYPE_WEBSOCKET;
    conn->handle.ws = ws;
    conn->player_id = -1;
    conn->game_id = -1;
    conn->active = true;

    registry->count++;

    printf("ConnectionRegistry: Registered WebSocket connection (id=%d, slot=%d, total=%d)\n",
           conn->id, slot, registry->count);

    return conn->id;
}
/* }}} */

/* {{{ conn_register_ssh */
int conn_register_ssh(ConnectionRegistry* registry, SSHConnection* ssh) {
    if (registry == NULL || ssh == NULL) {
        return CONN_INVALID_ID;
    }

    int slot = find_empty_slot(registry);
    if (slot < 0) {
        fprintf(stderr, "ConnectionRegistry: Pool full, cannot register SSH\n");
        return CONN_INVALID_ID;
    }

    Connection* conn = &registry->connections[slot];
    conn->id = registry->next_id++;
    conn->type = CONN_TYPE_SSH;
    conn->handle.ssh = ssh;
    conn->player_id = -1;
    conn->game_id = -1;
    conn->active = true;

    registry->count++;

    printf("ConnectionRegistry: Registered SSH connection (id=%d, slot=%d, total=%d)\n",
           conn->id, slot, registry->count);

    return conn->id;
}
/* }}} */

/* {{{ conn_unregister */
void conn_unregister(ConnectionRegistry* registry, int conn_id) {
    if (registry == NULL || conn_id < 0) {
        return;
    }

    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        if (registry->connections[i].active &&
            registry->connections[i].id == conn_id) {

            printf("ConnectionRegistry: Unregistered connection (id=%d, type=%s)\n",
                   conn_id,
                   registry->connections[i].type == CONN_TYPE_WEBSOCKET ? "WebSocket" : "SSH");

            registry->connections[i].active = false;
            registry->connections[i].handle.ws = NULL;
            registry->connections[i].handle.ssh = NULL;
            registry->connections[i].player_id = -1;
            registry->connections[i].game_id = -1;
            registry->count--;
            return;
        }
    }

    fprintf(stderr, "ConnectionRegistry: Connection %d not found for unregister\n", conn_id);
}
/* }}} */

/* ========================================================================== */
/*                              Connection Lookup                              */
/* ========================================================================== */

/* {{{ conn_get */
Connection* conn_get(ConnectionRegistry* registry, int conn_id) {
    if (registry == NULL || conn_id < 0) {
        return NULL;
    }

    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        if (registry->connections[i].active &&
            registry->connections[i].id == conn_id) {
            return &registry->connections[i];
        }
    }

    return NULL;
}
/* }}} */

/* {{{ conn_find_by_player */
Connection* conn_find_by_player(ConnectionRegistry* registry, int player_id) {
    if (registry == NULL || player_id < 0) {
        return NULL;
    }

    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        if (registry->connections[i].active &&
            registry->connections[i].player_id == player_id) {
            return &registry->connections[i];
        }
    }

    return NULL;
}
/* }}} */

/* {{{ conn_find_by_ws */
Connection* conn_find_by_ws(ConnectionRegistry* registry, WSConnection* ws) {
    if (registry == NULL || ws == NULL) {
        return NULL;
    }

    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        if (registry->connections[i].active &&
            registry->connections[i].type == CONN_TYPE_WEBSOCKET &&
            registry->connections[i].handle.ws == ws) {
            return &registry->connections[i];
        }
    }

    return NULL;
}
/* }}} */

/* {{{ conn_find_by_ssh */
Connection* conn_find_by_ssh(ConnectionRegistry* registry, SSHConnection* ssh) {
    if (registry == NULL || ssh == NULL) {
        return NULL;
    }

    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        if (registry->connections[i].active &&
            registry->connections[i].type == CONN_TYPE_SSH &&
            registry->connections[i].handle.ssh == ssh) {
            return &registry->connections[i];
        }
    }

    return NULL;
}
/* }}} */

/* ========================================================================== */
/*                              Player Assignment                              */
/* ========================================================================== */

/* {{{ conn_assign_player */
bool conn_assign_player(ConnectionRegistry* registry, int conn_id,
                        int player_id, int game_id) {
    Connection* conn = conn_get(registry, conn_id);
    if (conn == NULL) {
        return false;
    }

    conn->player_id = player_id;
    conn->game_id = game_id;

    printf("ConnectionRegistry: Assigned player %d, game %d to connection %d\n",
           player_id, game_id, conn_id);

    return true;
}
/* }}} */

/* {{{ conn_clear_player */
bool conn_clear_player(ConnectionRegistry* registry, int conn_id) {
    Connection* conn = conn_get(registry, conn_id);
    if (conn == NULL) {
        return false;
    }

    printf("ConnectionRegistry: Cleared player %d, game %d from connection %d\n",
           conn->player_id, conn->game_id, conn_id);

    conn->player_id = -1;
    conn->game_id = -1;

    return true;
}
/* }}} */

/* ========================================================================== */
/*                              Message Sending                                */
/* ========================================================================== */

/* {{{ send_to_connection
 * Internal helper that sends a message to a single connection.
 * Handles transport-specific sending.
 */
static bool send_to_connection(Connection* conn, const char* message) {
    if (conn == NULL || message == NULL || !conn->active) {
        return false;
    }

    switch (conn->type) {
        case CONN_TYPE_WEBSOCKET:
            if (conn->handle.ws != NULL) {
                return ws_send(conn->handle.ws, message);
            }
            break;

        case CONN_TYPE_SSH:
            if (conn->handle.ssh != NULL) {
                /* SSH sends raw terminal output */
                int result = ssh_connection_send_string(conn->handle.ssh, message);
                return (result >= 0);
            }
            break;
    }

    return false;
}
/* }}} */

/* {{{ conn_send */
bool conn_send(ConnectionRegistry* registry, int conn_id, const char* message) {
    Connection* conn = conn_get(registry, conn_id);
    return send_to_connection(conn, message);
}
/* }}} */

/* {{{ conn_send_to_player */
bool conn_send_to_player(ConnectionRegistry* registry, int player_id,
                         const char* message) {
    Connection* conn = conn_find_by_player(registry, player_id);
    return send_to_connection(conn, message);
}
/* }}} */

/* {{{ conn_broadcast_game */
int conn_broadcast_game(ConnectionRegistry* registry, int game_id,
                        const char* message, int exclude_player) {
    if (registry == NULL || message == NULL || game_id < 0) {
        return 0;
    }

    int sent = 0;
    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        Connection* conn = &registry->connections[i];
        if (conn->active &&
            conn->game_id == game_id &&
            (exclude_player < 0 || conn->player_id != exclude_player)) {
            if (send_to_connection(conn, message)) {
                sent++;
            }
        }
    }

    return sent;
}
/* }}} */

/* {{{ conn_broadcast_all */
int conn_broadcast_all(ConnectionRegistry* registry, const char* message) {
    if (registry == NULL || message == NULL) {
        return 0;
    }

    int sent = 0;
    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        Connection* conn = &registry->connections[i];
        if (conn->active) {
            if (send_to_connection(conn, message)) {
                sent++;
            }
        }
    }

    return sent;
}
/* }}} */

/* ========================================================================== */
/*                              Statistics                                     */
/* ========================================================================== */

/* {{{ conn_count */
int conn_count(const ConnectionRegistry* registry) {
    if (registry == NULL) {
        return 0;
    }
    return registry->count;
}
/* }}} */

/* {{{ conn_count_in_game */
int conn_count_in_game(const ConnectionRegistry* registry, int game_id) {
    if (registry == NULL || game_id < 0) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        if (registry->connections[i].active &&
            registry->connections[i].game_id == game_id) {
            count++;
        }
    }
    return count;
}
/* }}} */

/* {{{ conn_count_by_type */
int conn_count_by_type(const ConnectionRegistry* registry, ConnectionType type) {
    if (registry == NULL) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < CONN_MAX_CONNECTIONS; i++) {
        if (registry->connections[i].active &&
            registry->connections[i].type == type) {
            count++;
        }
    }
    return count;
}
/* }}} */
