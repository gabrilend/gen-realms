/* 05-websocket.c - WebSocket Handler Implementation
 *
 * Implements WebSocket communication for browser clients using libwebsockets.
 * Handles connection lifecycle, message parsing via protocol layer, and
 * broadcasting game state updates to all connected players.
 */

#define _POSIX_C_SOURCE 200809L

#include "05-websocket.h"
#include "04-protocol.h"
#include "../core/09-serialize.h"
#include "../../libs/cJSON.h"
#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================== */
/*                              Context Management                             */
/* ========================================================================== */

/* {{{ ws_context_create */
WSContext* ws_context_create(void) {
    WSContext* ctx = malloc(sizeof(WSContext));
    if (ctx == NULL) {
        return NULL;
    }

    memset(ctx->connections, 0, sizeof(ctx->connections));
    ctx->connection_count = 0;
    ctx->game = NULL;
    ctx->user_data = NULL;

    return ctx;
}
/* }}} */

/* {{{ ws_context_destroy */
void ws_context_destroy(WSContext* ctx) {
    if (ctx == NULL) {
        return;
    }

    /* Free all connections */
    for (int i = 0; i < WS_MAX_CONNECTIONS; i++) {
        if (ctx->connections[i] != NULL) {
            if (ctx->connections[i]->send_buffer != NULL) {
                free(ctx->connections[i]->send_buffer);
            }
            free(ctx->connections[i]);
        }
    }

    free(ctx);
}
/* }}} */

/* {{{ ws_context_set_game */
void ws_context_set_game(WSContext* ctx, Game* game) {
    if (ctx != NULL) {
        ctx->game = game;
    }
}
/* }}} */

/* ========================================================================== */
/*                              Connection Management                          */
/* ========================================================================== */

/* {{{ ws_connection_create */
WSConnection* ws_connection_create(WSContext* ctx, struct lws* wsi) {
    if (ctx == NULL || wsi == NULL) {
        return NULL;
    }

    /* Find empty slot */
    int slot = -1;
    for (int i = 0; i < WS_MAX_CONNECTIONS; i++) {
        if (ctx->connections[i] == NULL) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        fprintf(stderr, "WebSocket: Connection pool full\n");
        return NULL;
    }

    WSConnection* conn = malloc(sizeof(WSConnection));
    if (conn == NULL) {
        return NULL;
    }

    conn->wsi = wsi;
    conn->player_id = -1;
    conn->game_id = -1;
    conn->authenticated = false;
    conn->send_buffer = NULL;
    conn->send_len = 0;
    conn->send_pending = false;
    conn->index = slot;
    memset(conn->remote_addr, 0, sizeof(conn->remote_addr));

    /* Get remote address if available */
    char name[128];
    lws_get_peer_simple(wsi, name, sizeof(name));
    strncpy(conn->remote_addr, name, sizeof(conn->remote_addr) - 1);

    ctx->connections[slot] = conn;
    ctx->connection_count++;

    printf("WebSocket: New connection from %s (slot %d, total: %d)\n",
           conn->remote_addr, slot, ctx->connection_count);

    return conn;
}
/* }}} */

/* {{{ ws_connection_destroy */
void ws_connection_destroy(WSContext* ctx, WSConnection* conn) {
    if (ctx == NULL || conn == NULL) {
        return;
    }

    int slot = conn->index;
    if (slot < 0 || slot >= WS_MAX_CONNECTIONS) {
        return;
    }

    printf("WebSocket: Connection closed from %s (player %d)\n",
           conn->remote_addr, conn->player_id);

    if (conn->send_buffer != NULL) {
        free(conn->send_buffer);
    }

    free(conn);
    ctx->connections[slot] = NULL;
    ctx->connection_count--;
}
/* }}} */

/* {{{ ws_connection_find_by_wsi */
WSConnection* ws_connection_find_by_wsi(WSContext* ctx, struct lws* wsi) {
    if (ctx == NULL || wsi == NULL) {
        return NULL;
    }

    for (int i = 0; i < WS_MAX_CONNECTIONS; i++) {
        if (ctx->connections[i] != NULL &&
            ctx->connections[i]->wsi == wsi) {
            return ctx->connections[i];
        }
    }

    return NULL;
}
/* }}} */

/* {{{ ws_connection_find_by_player */
WSConnection* ws_connection_find_by_player(WSContext* ctx, int player_id) {
    if (ctx == NULL || player_id < 0) {
        return NULL;
    }

    for (int i = 0; i < WS_MAX_CONNECTIONS; i++) {
        if (ctx->connections[i] != NULL &&
            ctx->connections[i]->player_id == player_id) {
            return ctx->connections[i];
        }
    }

    return NULL;
}
/* }}} */

/* ========================================================================== */
/*                              Message Sending                                */
/* ========================================================================== */

/* {{{ ws_send */
bool ws_send(WSConnection* conn, const char* json) {
    if (conn == NULL || json == NULL) {
        return false;
    }

    size_t len = strlen(json);
    if (len > WS_SEND_BUFFER_SIZE - LWS_PRE) {
        fprintf(stderr, "WebSocket: Message too large (%zu bytes)\n", len);
        return false;
    }

    /* Allocate or reallocate send buffer */
    if (conn->send_buffer == NULL) {
        conn->send_buffer = malloc(WS_SEND_BUFFER_SIZE);
        if (conn->send_buffer == NULL) {
            return false;
        }
    }

    /* Copy message with LWS_PRE padding */
    memcpy(conn->send_buffer + LWS_PRE, json, len);
    conn->send_len = len;
    conn->send_pending = true;

    /* Request callback to send */
    lws_callback_on_writable(conn->wsi);

    return true;
}
/* }}} */

/* {{{ ws_send_to_player */
bool ws_send_to_player(WSContext* ctx, int player_id, const char* json) {
    WSConnection* conn = ws_connection_find_by_player(ctx, player_id);
    if (conn == NULL) {
        return false;
    }
    return ws_send(conn, json);
}
/* }}} */

/* {{{ ws_broadcast */
void ws_broadcast(WSContext* ctx, const char* json, int exclude_player) {
    if (ctx == NULL || json == NULL) {
        return;
    }

    for (int i = 0; i < WS_MAX_CONNECTIONS; i++) {
        WSConnection* conn = ctx->connections[i];
        if (conn != NULL && conn->authenticated) {
            if (exclude_player < 0 || conn->player_id != exclude_player) {
                ws_send(conn, json);
            }
        }
    }
}
/* }}} */

/* {{{ ws_broadcast_gamestate */
void ws_broadcast_gamestate(WSContext* ctx, Game* game) {
    if (ctx == NULL || game == NULL) {
        return;
    }

    for (int i = 0; i < WS_MAX_CONNECTIONS; i++) {
        WSConnection* conn = ctx->connections[i];
        if (conn != NULL && conn->authenticated && conn->player_id >= 0) {
            /* Create player-specific gamestate message */
            Message* msg = protocol_create_gamestate(game, conn->player_id);
            if (msg != NULL) {
                char* json = protocol_serialize(msg);
                if (json != NULL) {
                    ws_send(conn, json);
                    free(json);
                }
                message_free(msg);
            }
        }
    }
}
/* }}} */

/* ========================================================================== */
/*                              Message Handling                               */
/* ========================================================================== */

/* {{{ send_error_response
 * Helper to send a protocol error back to the client.
 */
static void send_error_response(WSConnection* conn, ProtocolError error,
                                const char* details) {
    Message* msg = protocol_create_error(error, details);
    if (msg != NULL) {
        char* json = protocol_serialize(msg);
        if (json != NULL) {
            ws_send(conn, json);
            free(json);
        }
        message_free(msg);
    }
}
/* }}} */

/* {{{ ws_handle_message */
void ws_handle_message(WSContext* ctx, WSConnection* conn,
                       const char* data, size_t len) {
    if (ctx == NULL || conn == NULL || data == NULL || len == 0) {
        return;
    }

    /* Create null-terminated copy for parsing */
    char* json_str = malloc(len + 1);
    if (json_str == NULL) {
        send_error_response(conn, PROTOCOL_ERROR_MALFORMED_JSON,
                           "Memory allocation failed");
        return;
    }
    memcpy(json_str, data, len);
    json_str[len] = '\0';

    /* Parse message */
    ProtocolError error = PROTOCOL_OK;
    Message* msg = protocol_parse(json_str, &error);
    free(json_str);

    if (msg == NULL) {
        send_error_response(conn, error, protocol_error_to_string(error));
        return;
    }

    /* Set player ID from connection */
    msg->player_id = conn->player_id;

    /* Handle special case: join message (player not yet authenticated) */
    if (msg->type == MSG_JOIN && !conn->authenticated) {
        cJSON* name_item = cJSON_GetObjectItem(msg->payload, "name");
        if (cJSON_IsString(name_item)) {
            /* For now, assign a player ID based on connection count */
            /* Full game session management will be in 2-007 */
            conn->player_id = ctx->connection_count - 1;
            conn->authenticated = true;

            printf("WebSocket: Player '%s' joined as player %d\n",
                   name_item->valuestring, conn->player_id);

            /* Send player_joined notification to all */
            Message* joined_msg = protocol_create_player_joined(
                conn->player_id, name_item->valuestring);
            if (joined_msg != NULL) {
                char* json = protocol_serialize(joined_msg);
                if (json != NULL) {
                    ws_broadcast(ctx, json, -1);
                    free(json);
                }
                message_free(joined_msg);
            }

            /* Send initial gamestate if game exists */
            if (ctx->game != NULL) {
                Message* state_msg = protocol_create_gamestate(
                    ctx->game, conn->player_id);
                if (state_msg != NULL) {
                    char* json = protocol_serialize(state_msg);
                    if (json != NULL) {
                        ws_send(conn, json);
                        free(json);
                    }
                    message_free(state_msg);
                }
            }
        } else {
            send_error_response(conn, PROTOCOL_ERROR_MISSING_FIELD,
                               "Join message requires 'name' field");
        }
        message_free(msg);
        return;
    }

    /* Reject non-join messages from unauthenticated connections */
    if (!conn->authenticated) {
        send_error_response(conn, PROTOCOL_ERROR_NOT_IN_GAME,
                           "Must send 'join' message first");
        message_free(msg);
        return;
    }

    /* Dispatch to protocol handler */
    error = protocol_dispatch(ctx->game, conn->player_id, msg, ctx);

    if (error != PROTOCOL_OK) {
        send_error_response(conn, error, protocol_error_to_string(error));
    } else {
        /* On successful action, broadcast updated gamestate */
        if (msg->type == MSG_ACTION || msg->type == MSG_END_TURN) {
            ws_broadcast_gamestate(ctx, ctx->game);
        }
    }

    message_free(msg);
}
/* }}} */

/* ========================================================================== */
/*                              WebSocket Callback                             */
/* ========================================================================== */

/* {{{ LwsUserData
 * Wrapper struct used by http server to store both HTTP and WS context.
 * Must match the definition in 02-http.c.
 */
typedef struct {
    void* server;           /* HttpServer* */
    WSContext* ws_context;
} LwsUserData;
/* }}} */

/* {{{ callback_game
 * Main WebSocket protocol callback for game communication.
 * Handles connection lifecycle and message routing.
 */
static int callback_game(struct lws* wsi, enum lws_callback_reasons reason,
                        void* user, void* in, size_t len) {
    /* Get context from lws user data wrapper */
    LwsUserData* lws_user = (LwsUserData*)lws_context_user(lws_get_context(wsi));
    WSContext* ctx = lws_user ? lws_user->ws_context : NULL;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: {
            /* New WebSocket connection */
            WSConnection* conn = ws_connection_create(ctx, wsi);
            if (conn == NULL) {
                /* Connection pool full, reject */
                return -1;
            }
            /* Store connection pointer in per-session user data */
            *(WSConnection**)user = conn;
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            /* Incoming message */
            WSConnection* conn = *(WSConnection**)user;
            if (conn != NULL) {
                ws_handle_message(ctx, conn, (const char*)in, len);
            }
            break;
        }

        case LWS_CALLBACK_SERVER_WRITEABLE: {
            /* Ready to send queued message */
            WSConnection* conn = *(WSConnection**)user;
            if (conn != NULL && conn->send_pending && conn->send_buffer != NULL) {
                int written = lws_write(wsi,
                    (unsigned char*)(conn->send_buffer + LWS_PRE),
                    conn->send_len,
                    LWS_WRITE_TEXT);

                if (written < 0) {
                    fprintf(stderr, "WebSocket: Write failed\n");
                    return -1;
                }

                conn->send_pending = false;
            }
            break;
        }

        case LWS_CALLBACK_CLOSED: {
            /* Connection closed */
            WSConnection* conn = *(WSConnection**)user;
            if (conn != NULL) {
                /* Notify other players if authenticated */
                if (conn->authenticated && ctx != NULL) {
                    Message* msg = protocol_create_player_left(conn->player_id);
                    if (msg != NULL) {
                        char* json = protocol_serialize(msg);
                        if (json != NULL) {
                            ws_broadcast(ctx, json, conn->player_id);
                            free(json);
                        }
                        message_free(msg);
                    }
                }
                ws_connection_destroy(ctx, conn);
            }
            break;
        }

        default:
            break;
    }

    return 0;
}
/* }}} */

/* {{{ WebSocket protocol definition */
static const struct lws_protocols ws_protocol = {
    .name = WS_PROTOCOL_NAME,
    .callback = callback_game,
    .per_session_data_size = sizeof(WSConnection*),
    .rx_buffer_size = WS_RECV_BUFFER_SIZE,
};
/* }}} */

/* {{{ ws_get_protocol */
const struct lws_protocols* ws_get_protocol(void) {
    return &ws_protocol;
}
/* }}} */

/* ========================================================================== */
/*                              Utility Functions                              */
/* ========================================================================== */

/* {{{ ws_get_connection_count */
int ws_get_connection_count(const WSContext* ctx) {
    if (ctx == NULL) {
        return 0;
    }
    return ctx->connection_count;
}
/* }}} */

/* {{{ ws_get_authenticated_count */
int ws_get_authenticated_count(const WSContext* ctx) {
    if (ctx == NULL) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < WS_MAX_CONNECTIONS; i++) {
        if (ctx->connections[i] != NULL &&
            ctx->connections[i]->authenticated) {
            count++;
        }
    }
    return count;
}
/* }}} */
