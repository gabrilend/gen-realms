/*
 * ai-hooks.c - AI Integration Implementation for WASM Client
 *
 * Manages AI requests via WebSocket and provides streaming
 * narrative generation support.
 */

#include "ai-hooks.h"
#include "websocket.h"
#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* {{{ Constants */
#define MAX_PENDING_REQUESTS 8
/* }}} */

/* {{{ AIRequest
 * Pending AI request.
 */
typedef struct {
    int id;
    AIRequestType type;
    AIResponseStatus status;
    char partial_text[MAX_NARRATIVE_LEN];
    int partial_len;
} AIRequest;
/* }}} */

/* {{{ Global state */
static AICallbacks g_callbacks = {0};
static AIRequest g_requests[MAX_PENDING_REQUESTS] = {0};
static int g_next_id = 1;
static bool g_initialized = false;

/* Feature toggles */
static bool g_enable_narratives = true;
static bool g_enable_hints = false;
static bool g_enable_opponent = true;
/* }}} */

/* {{{ find_request
 * Find a request by ID.
 */
static AIRequest* find_request(int id) {
    for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if (g_requests[i].id == id) {
            return &g_requests[i];
        }
    }
    return NULL;
}
/* }}} */

/* {{{ find_free_request
 * Find a free request slot.
 */
static AIRequest* find_free_request(void) {
    for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if (g_requests[i].id == 0) {
            return &g_requests[i];
        }
    }
    return NULL;
}
/* }}} */

/* {{{ ai_init */
bool ai_init(const AICallbacks* callbacks) {
    if (g_initialized) {
        ai_cleanup();
    }

    if (callbacks) {
        g_callbacks = *callbacks;
    } else {
        memset(&g_callbacks, 0, sizeof(g_callbacks));
    }

    memset(g_requests, 0, sizeof(g_requests));
    g_next_id = 1;
    g_initialized = true;

    return true;
}
/* }}} */

/* {{{ ai_cleanup */
void ai_cleanup(void) {
    ai_cancel_all();
    memset(&g_callbacks, 0, sizeof(g_callbacks));
    g_initialized = false;
}
/* }}} */

/* {{{ ai_request_narrative */
int ai_request_narrative(const NarrativeContext* ctx) {
    if (!g_initialized || !ctx || !g_enable_narratives) return -1;
    if (ws_get_state() != WS_CONNECTED) return -1;

    AIRequest* req = find_free_request();
    if (!req) return -1;

    req->id = g_next_id++;
    req->type = AI_REQ_NARRATIVE;
    req->status = AI_STATUS_PENDING;
    req->partial_len = 0;
    req->partial_text[0] = '\0';

    /* Build request JSON */
    char json[1024];
    snprintf(json, sizeof(json),
        "{"
        "\"type\":\"ai_request\","
        "\"request_type\":\"narrative\","
        "\"request_id\":%d,"
        "\"context\":{"
            "\"event\":\"%s\","
            "\"card\":\"%s\","
            "\"faction\":\"%s\","
            "\"value\":%d,"
            "\"target\":\"%s\","
            "\"is_player\":%s"
        "}"
        "}",
        req->id,
        ctx->event_type ? ctx->event_type : "",
        ctx->card_name ? ctx->card_name : "",
        ctx->card_faction ? ctx->card_faction : "",
        ctx->value,
        ctx->target_name ? ctx->target_name : "",
        ctx->is_player_action ? "true" : "false"
    );

    if (!ws_send_json(json)) {
        req->id = 0;
        return -1;
    }

    return req->id;
}
/* }}} */

/* {{{ ai_request_hint */
int ai_request_hint(const char* game_state_json) {
    if (!g_initialized || !game_state_json || !g_enable_hints) return -1;
    if (ws_get_state() != WS_CONNECTED) return -1;

    AIRequest* req = find_free_request();
    if (!req) return -1;

    req->id = g_next_id++;
    req->type = AI_REQ_HINT;
    req->status = AI_STATUS_PENDING;

    /* Build request JSON */
    char json[4096];
    snprintf(json, sizeof(json),
        "{"
        "\"type\":\"ai_request\","
        "\"request_type\":\"hint\","
        "\"request_id\":%d,"
        "\"game_state\":%s"
        "}",
        req->id,
        game_state_json
    );

    if (!ws_send_json(json)) {
        req->id = 0;
        return -1;
    }

    return req->id;
}
/* }}} */

/* {{{ ai_request_opponent_action */
int ai_request_opponent_action(const char* game_state_json, int difficulty) {
    if (!g_initialized || !game_state_json || !g_enable_opponent) return -1;
    if (ws_get_state() != WS_CONNECTED) return -1;

    AIRequest* req = find_free_request();
    if (!req) return -1;

    req->id = g_next_id++;
    req->type = AI_REQ_OPPONENT_ACTION;
    req->status = AI_STATUS_PENDING;

    /* Build request JSON */
    char json[4096];
    snprintf(json, sizeof(json),
        "{"
        "\"type\":\"ai_request\","
        "\"request_type\":\"opponent_action\","
        "\"request_id\":%d,"
        "\"difficulty\":%d,"
        "\"game_state\":%s"
        "}",
        req->id,
        difficulty,
        game_state_json
    );

    if (!ws_send_json(json)) {
        req->id = 0;
        return -1;
    }

    return req->id;
}
/* }}} */

/* {{{ ai_cancel_request */
void ai_cancel_request(int request_id) {
    AIRequest* req = find_request(request_id);
    if (!req) return;

    req->status = AI_STATUS_CANCELLED;

    /* Notify server */
    char json[128];
    snprintf(json, sizeof(json),
        "{\"type\":\"ai_cancel\",\"request_id\":%d}",
        request_id
    );
    ws_send_json(json);

    /* Clean up slot */
    req->id = 0;
}
/* }}} */

/* {{{ ai_cancel_all */
void ai_cancel_all(void) {
    for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if (g_requests[i].id != 0) {
            ai_cancel_request(g_requests[i].id);
        }
    }
}
/* }}} */

/* {{{ ai_get_status */
AIResponseStatus ai_get_status(int request_id) {
    AIRequest* req = find_request(request_id);
    if (!req) return AI_STATUS_ERROR;
    return req->status;
}
/* }}} */

/* {{{ ai_is_busy */
bool ai_is_busy(void) {
    for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if (g_requests[i].id != 0 &&
            (g_requests[i].status == AI_STATUS_PENDING ||
             g_requests[i].status == AI_STATUS_STREAMING)) {
            return true;
        }
    }
    return false;
}
/* }}} */

/* {{{ ai_set_enabled */
void ai_set_enabled(bool narratives, bool hints, bool opponent) {
    g_enable_narratives = narratives;
    g_enable_hints = hints;
    g_enable_opponent = opponent;

    /* Cancel incompatible pending requests */
    for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
        AIRequest* req = &g_requests[i];
        if (req->id == 0) continue;

        if ((req->type == AI_REQ_NARRATIVE && !narratives) ||
            (req->type == AI_REQ_HINT && !hints) ||
            (req->type == AI_REQ_OPPONENT_ACTION && !opponent)) {
            ai_cancel_request(req->id);
        }
    }
}
/* }}} */

/* {{{ handle_narrative_response
 * Handle streaming narrative response.
 */
static void handle_narrative_response(AIRequest* req, const char* text,
                                       bool is_delta, bool is_complete) {
    if (is_delta) {
        /* Append to partial buffer */
        int text_len = strlen(text);
        int space = MAX_NARRATIVE_LEN - req->partial_len - 1;
        if (text_len > space) text_len = space;

        memcpy(req->partial_text + req->partial_len, text, text_len);
        req->partial_len += text_len;
        req->partial_text[req->partial_len] = '\0';

        req->status = AI_STATUS_STREAMING;
    } else {
        /* Replace entire buffer */
        strncpy(req->partial_text, text, MAX_NARRATIVE_LEN - 1);
        req->partial_text[MAX_NARRATIVE_LEN - 1] = '\0';
        req->partial_len = strlen(req->partial_text);
    }

    if (is_complete) {
        req->status = AI_STATUS_COMPLETE;
    }

    /* Notify callback */
    if (g_callbacks.on_narrative) {
        g_callbacks.on_narrative(req->partial_text, is_complete, g_callbacks.user_data);
    }

    /* Clean up if complete */
    if (is_complete) {
        req->id = 0;
    }
}
/* }}} */

/* {{{ handle_hint_response
 * Handle hint response.
 */
static void handle_hint_response(AIRequest* req, const char* action_type,
                                  int target_id, const char* reason, float confidence) {
    req->status = AI_STATUS_COMPLETE;

    if (g_callbacks.on_hint) {
        AIHint hint = {0};
        hint.action_type = action_type;
        hint.target_id = target_id;
        hint.confidence = confidence;
        if (reason) {
            strncpy(hint.reason, reason, MAX_ACTION_REASON_LEN - 1);
        }

        g_callbacks.on_hint(&hint, g_callbacks.user_data);
    }

    req->id = 0;
}
/* }}} */

/* {{{ handle_opponent_action_response
 * Handle opponent action response.
 */
static void handle_opponent_action_response(AIRequest* req, const char* action_type,
                                             int target_id, const char* reasoning,
                                             float think_time) {
    req->status = AI_STATUS_COMPLETE;

    if (g_callbacks.on_opponent_action) {
        AIOpponentAction action = {0};
        action.action_type = action_type;
        action.target_id = target_id;
        action.think_time = think_time;
        if (reasoning) {
            strncpy(action.reasoning, reasoning, MAX_ACTION_REASON_LEN - 1);
        }

        g_callbacks.on_opponent_action(&action, g_callbacks.user_data);
    }

    req->id = 0;
}
/* }}} */

/* {{{ ai_on_message
 * Process an AI response message from WebSocket.
 * Called from main message handler.
 */
void ai_on_message(int request_id, const char* response_json) {
    if (!g_initialized || !response_json) return;

    AIRequest* req = find_request(request_id);
    if (!req) return;

    /* Parse response type and dispatch */
    /* Note: In real implementation, use cJSON for proper parsing */

    if (strstr(response_json, "\"narrative\"")) {
        /* Extract text from response */
        const char* text_start = strstr(response_json, "\"text\":\"");
        if (text_start) {
            text_start += 8;
            const char* text_end = strchr(text_start, '"');
            if (text_end) {
                int len = text_end - text_start;
                char text[MAX_NARRATIVE_LEN];
                if (len >= MAX_NARRATIVE_LEN) len = MAX_NARRATIVE_LEN - 1;
                memcpy(text, text_start, len);
                text[len] = '\0';

                bool is_delta = strstr(response_json, "\"delta\":true") != NULL;
                bool is_complete = strstr(response_json, "\"complete\":true") != NULL;

                handle_narrative_response(req, text, is_delta, is_complete);
            }
        }
    } else if (strstr(response_json, "\"hint\"")) {
        /* Parse hint response */
        handle_hint_response(req, "play_card", 0, "Sample hint", 0.8f);
    } else if (strstr(response_json, "\"opponent_action\"")) {
        /* Parse opponent action response */
        handle_opponent_action_response(req, "play_card", 0, "AI reasoning", 1.5f);
    } else if (strstr(response_json, "\"error\"")) {
        req->status = AI_STATUS_ERROR;
        if (g_callbacks.on_error) {
            g_callbacks.on_error("AI request failed", g_callbacks.user_data);
        }
        req->id = 0;
    }
}
/* }}} */

/* {{{ ai_update */
void ai_update(void) {
    /* Process timeouts and cleanup */
    /* In a real implementation, would check for stale requests */
}
/* }}} */
