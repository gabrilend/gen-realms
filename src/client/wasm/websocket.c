/*
 * websocket.c - WebSocket Communication Implementation for WASM Client
 *
 * Uses Emscripten's WebSocket API via EM_ASM for browser integration.
 * Handles connection lifecycle, message parsing, and ping/pong.
 */

#include "websocket.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* {{{ Constants */
#define MAX_URL_LEN 256
#define MAX_MESSAGE_LEN 65536
#define PING_INTERVAL_MS 30000
#define PING_TIMEOUT_MS 5000
/* }}} */

/* {{{ Global state */
static WebSocketState g_state = WS_DISCONNECTED;
static WebSocketCallbacks g_callbacks = {0};
static char g_url[MAX_URL_LEN] = {0};
static int g_latency = -1;
static double g_ping_sent_time = 0.0;
static double g_last_ping_time = 0.0;
static bool g_initialized = false;
/* }}} */

/* {{{ parse_message_type
 * Parse message type from JSON.
 */
static MessageType parse_message_type(const char* json) {
    if (!json) return MSG_UNKNOWN;

    /* Simple type detection - look for "type" field */
    const char* type_str = strstr(json, "\"type\"");
    if (!type_str) return MSG_UNKNOWN;

    if (strstr(type_str, "\"game_state\"")) return MSG_GAME_STATE;
    if (strstr(type_str, "\"action\"")) return MSG_PLAYER_ACTION;
    if (strstr(type_str, "\"narrative\"")) return MSG_NARRATIVE_UPDATE;
    if (strstr(type_str, "\"error\"")) return MSG_ERROR;
    if (strstr(type_str, "\"chat\"")) return MSG_CHAT;
    if (strstr(type_str, "\"ping\"")) return MSG_PING;
    if (strstr(type_str, "\"pong\"")) return MSG_PONG;

    return MSG_UNKNOWN;
}
/* }}} */

/* {{{ ws_on_open_callback
 * Called when connection opens (from JS).
 */
EMSCRIPTEN_KEEPALIVE
void ws_on_open_callback(void) {
    g_state = WS_CONNECTED;
    g_latency = -1;

    if (g_callbacks.on_connect) {
        g_callbacks.on_connect(g_callbacks.user_data);
    }
}
/* }}} */

/* {{{ ws_on_close_callback
 * Called when connection closes (from JS).
 */
EMSCRIPTEN_KEEPALIVE
void ws_on_close_callback(int code, const char* reason) {
    g_state = WS_DISCONNECTED;

    if (g_callbacks.on_disconnect) {
        g_callbacks.on_disconnect(code, reason, g_callbacks.user_data);
    }
}
/* }}} */

/* {{{ ws_on_message_callback
 * Called when message received (from JS).
 */
EMSCRIPTEN_KEEPALIVE
void ws_on_message_callback(const char* data, int len) {
    if (!data || len <= 0) return;

    /* Handle ping/pong internally */
    MessageType type = parse_message_type(data);

    if (type == MSG_PONG) {
        /* Calculate latency */
        double now = emscripten_get_now();
        if (g_ping_sent_time > 0.0) {
            g_latency = (int)(now - g_ping_sent_time);
            g_ping_sent_time = 0.0;
        }
        return;
    }

    if (type == MSG_PING) {
        /* Respond with pong */
        ws_send_json("{\"type\":\"pong\"}");
        return;
    }

    /* Pass to application callback */
    if (g_callbacks.on_message) {
        WebSocketMessage msg = {0};
        msg.type = type;
        msg.raw_json = data;
        msg.json_len = len;

        /* Extract common fields for convenience */
        /* Note: Full JSON parsing should use cJSON in real implementation */

        g_callbacks.on_message(&msg, g_callbacks.user_data);
    }
}
/* }}} */

/* {{{ ws_on_error_callback
 * Called on WebSocket error (from JS).
 */
EMSCRIPTEN_KEEPALIVE
void ws_on_error_callback(const char* error) {
    g_state = WS_ERROR;

    if (g_callbacks.on_error) {
        g_callbacks.on_error(error, g_callbacks.user_data);
    }
}
/* }}} */

/* {{{ ws_init */
bool ws_init(const WebSocketCallbacks* callbacks) {
    if (g_initialized) {
        ws_cleanup();
    }

    if (callbacks) {
        g_callbacks = *callbacks;
    } else {
        memset(&g_callbacks, 0, sizeof(g_callbacks));
    }

    g_state = WS_DISCONNECTED;
    g_latency = -1;
    g_initialized = true;

    return true;
}
/* }}} */

/* {{{ ws_cleanup */
void ws_cleanup(void) {
    if (g_state == WS_CONNECTED || g_state == WS_CONNECTING) {
        ws_disconnect();
    }

    memset(&g_callbacks, 0, sizeof(g_callbacks));
    g_initialized = false;
}
/* }}} */

/* {{{ ws_connect */
bool ws_connect(const char* url) {
    if (!url || strlen(url) >= MAX_URL_LEN) return false;
    if (g_state == WS_CONNECTED || g_state == WS_CONNECTING) return false;

    strncpy(g_url, url, MAX_URL_LEN - 1);
    g_url[MAX_URL_LEN - 1] = '\0';
    g_state = WS_CONNECTING;

    /* Create WebSocket in JavaScript */
    EM_ASM({
        var url = UTF8ToString($0);

        if (Module.ws) {
            Module.ws.close();
        }

        try {
            Module.ws = new WebSocket(url);

            Module.ws.onopen = function() {
                _ws_on_open_callback();
            };

            Module.ws.onclose = function(e) {
                var reason = e.reason || '';
                var reasonPtr = Module._malloc(reason.length + 1);
                Module.stringToUTF8(reason, reasonPtr, reason.length + 1);
                _ws_on_close_callback(e.code, reasonPtr);
                Module._free(reasonPtr);
            };

            Module.ws.onmessage = function(e) {
                var data = e.data;
                var dataPtr = Module._malloc(data.length + 1);
                Module.stringToUTF8(data, dataPtr, data.length + 1);
                _ws_on_message_callback(dataPtr, data.length);
                Module._free(dataPtr);
            };

            Module.ws.onerror = function(e) {
                var error = 'WebSocket error';
                var errorPtr = Module._malloc(error.length + 1);
                Module.stringToUTF8(error, errorPtr, error.length + 1);
                _ws_on_error_callback(errorPtr);
                Module._free(errorPtr);
            };
        } catch (err) {
            var error = err.message || 'Connection failed';
            var errorPtr = Module._malloc(error.length + 1);
            Module.stringToUTF8(error, errorPtr, error.length + 1);
            _ws_on_error_callback(errorPtr);
            Module._free(errorPtr);
        }
    }, url);

    return true;
}
/* }}} */

/* {{{ ws_disconnect */
void ws_disconnect(void) {
    if (g_state != WS_CONNECTED && g_state != WS_CONNECTING) return;

    g_state = WS_CLOSING;

    EM_ASM({
        if (Module.ws) {
            Module.ws.close(1000, 'Client disconnect');
            Module.ws = null;
        }
    });

    g_state = WS_DISCONNECTED;
}
/* }}} */

/* {{{ ws_get_state */
WebSocketState ws_get_state(void) {
    return g_state;
}
/* }}} */

/* {{{ ws_send */
bool ws_send(const char* data, int len) {
    if (g_state != WS_CONNECTED || !data || len <= 0) return false;

    EM_ASM({
        if (Module.ws && Module.ws.readyState === WebSocket.OPEN) {
            var data = UTF8ToString($0, $1);
            Module.ws.send(data);
        }
    }, data, len);

    return true;
}
/* }}} */

/* {{{ ws_send_json */
bool ws_send_json(const char* json) {
    if (!json) return false;
    return ws_send(json, strlen(json));
}
/* }}} */

/* {{{ ws_send_action */
bool ws_send_action(const char* action_type, int target_id, const char* extra_json) {
    if (g_state != WS_CONNECTED || !action_type) return false;

    char buf[1024];
    int len;

    if (extra_json) {
        len = snprintf(buf, sizeof(buf),
            "{\"type\":\"action\",\"action\":\"%s\",\"target\":%d,\"extra\":%s}",
            action_type, target_id, extra_json);
    } else if (target_id >= 0) {
        len = snprintf(buf, sizeof(buf),
            "{\"type\":\"action\",\"action\":\"%s\",\"target\":%d}",
            action_type, target_id);
    } else {
        len = snprintf(buf, sizeof(buf),
            "{\"type\":\"action\",\"action\":\"%s\"}",
            action_type);
    }

    return ws_send(buf, len);
}
/* }}} */

/* {{{ ws_send_chat */
bool ws_send_chat(const char* message) {
    if (g_state != WS_CONNECTED || !message) return false;

    char buf[512];
    int len = snprintf(buf, sizeof(buf), "{\"type\":\"chat\",\"message\":\"%s\"}", message);
    return ws_send(buf, len);
}
/* }}} */

/* {{{ ws_request_state */
bool ws_request_state(void) {
    return ws_send_json("{\"type\":\"request_state\"}");
}
/* }}} */

/* {{{ ws_update */
void ws_update(void) {
    if (g_state != WS_CONNECTED) return;

    /* Send periodic ping */
    double now = emscripten_get_now();

    if (now - g_last_ping_time >= PING_INTERVAL_MS) {
        ws_send_json("{\"type\":\"ping\"}");
        g_ping_sent_time = now;
        g_last_ping_time = now;
    }

    /* Check for ping timeout */
    if (g_ping_sent_time > 0.0 && now - g_ping_sent_time > PING_TIMEOUT_MS) {
        /* Ping timed out - connection may be dead */
        g_latency = -1;
        g_ping_sent_time = 0.0;

        if (g_callbacks.on_error) {
            g_callbacks.on_error("Ping timeout", g_callbacks.user_data);
        }
    }
}
/* }}} */

/* {{{ ws_get_latency */
int ws_get_latency(void) {
    return g_latency;
}
/* }}} */
