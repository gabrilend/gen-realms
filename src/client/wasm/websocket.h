/*
 * websocket.h - WebSocket Communication for WASM Client
 *
 * Handles connection to game server, message parsing,
 * and event dispatch using Emscripten WebSocket API.
 */

#ifndef WASM_WEBSOCKET_H
#define WASM_WEBSOCKET_H

#include <stdbool.h>
#include <stdint.h>

/* {{{ Connection states */
typedef enum {
    WS_DISCONNECTED,
    WS_CONNECTING,
    WS_CONNECTED,
    WS_ERROR,
    WS_CLOSING
} WebSocketState;
/* }}} */

/* {{{ Message types */
typedef enum {
    MSG_UNKNOWN,
    MSG_GAME_STATE,
    MSG_PLAYER_ACTION,
    MSG_NARRATIVE_UPDATE,
    MSG_ERROR,
    MSG_CHAT,
    MSG_PING,
    MSG_PONG
} MessageType;
/* }}} */

/* {{{ WebSocketMessage
 * Parsed message from server.
 */
typedef struct {
    MessageType type;
    const char* raw_json;           /* Original JSON string */
    int json_len;

    /* Convenience fields for common messages */
    int player_id;
    int turn_number;
    const char* action_type;
    const char* narrative_text;
    const char* error_message;
} WebSocketMessage;
/* }}} */

/* {{{ Callback types */
typedef void (*OnConnectCallback)(void* user_data);
typedef void (*OnDisconnectCallback)(int code, const char* reason, void* user_data);
typedef void (*OnMessageCallback)(const WebSocketMessage* msg, void* user_data);
typedef void (*OnErrorCallback)(const char* error, void* user_data);
/* }}} */

/* {{{ WebSocketCallbacks
 * All callbacks for WebSocket events.
 */
typedef struct {
    OnConnectCallback on_connect;
    OnDisconnectCallback on_disconnect;
    OnMessageCallback on_message;
    OnErrorCallback on_error;
    void* user_data;
} WebSocketCallbacks;
/* }}} */

/* {{{ ws_init
 * Initialize WebSocket system.
 * @param callbacks - Event callbacks
 * @return true on success
 */
bool ws_init(const WebSocketCallbacks* callbacks);
/* }}} */

/* {{{ ws_cleanup
 * Clean up WebSocket system.
 */
void ws_cleanup(void);
/* }}} */

/* {{{ ws_connect
 * Connect to a WebSocket server.
 * @param url - Server URL (ws:// or wss://)
 * @return true if connection initiated
 */
bool ws_connect(const char* url);
/* }}} */

/* {{{ ws_disconnect
 * Disconnect from server.
 */
void ws_disconnect(void);
/* }}} */

/* {{{ ws_get_state
 * Get current connection state.
 * @return Current state
 */
WebSocketState ws_get_state(void);
/* }}} */

/* {{{ ws_send
 * Send a raw message to server.
 * @param data - Message data
 * @param len - Message length
 * @return true if sent
 */
bool ws_send(const char* data, int len);
/* }}} */

/* {{{ ws_send_json
 * Send a JSON message to server.
 * @param json - JSON string (null-terminated)
 * @return true if sent
 */
bool ws_send_json(const char* json);
/* }}} */

/* {{{ ws_send_action
 * Send a player action to server.
 * @param action_type - Action type string
 * @param target_id - Target card/base ID (-1 if none)
 * @param extra_json - Additional JSON data (or NULL)
 * @return true if sent
 */
bool ws_send_action(const char* action_type, int target_id, const char* extra_json);
/* }}} */

/* {{{ ws_send_chat
 * Send a chat message.
 * @param message - Chat text
 * @return true if sent
 */
bool ws_send_chat(const char* message);
/* }}} */

/* {{{ ws_request_state
 * Request full game state from server.
 * @return true if request sent
 */
bool ws_request_state(void);
/* }}} */

/* {{{ ws_update
 * Process pending WebSocket events.
 * Should be called each frame.
 */
void ws_update(void);
/* }}} */

/* {{{ ws_get_latency
 * Get current round-trip latency in milliseconds.
 * @return Latency, or -1 if unknown
 */
int ws_get_latency(void);
/* }}} */

#endif /* WASM_WEBSOCKET_H */
