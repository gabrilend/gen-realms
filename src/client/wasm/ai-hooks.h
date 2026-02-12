/*
 * ai-hooks.h - AI Integration Hooks for WASM Client
 *
 * Provides hooks for LLM narrative generation and AI opponent
 * decision making. Communicates with server-side AI services.
 */

#ifndef WASM_AI_HOOKS_H
#define WASM_AI_HOOKS_H

#include <stdbool.h>
#include <stdint.h>

/* {{{ Maximum text lengths */
#define MAX_NARRATIVE_LEN 1024
#define MAX_HINT_LEN 256
#define MAX_ACTION_REASON_LEN 512
/* }}} */

/* {{{ AIRequestType
 * Types of AI requests.
 */
typedef enum {
    AI_REQ_NARRATIVE,        /* Generate narrative text */
    AI_REQ_HINT,             /* Get play suggestion */
    AI_REQ_OPPONENT_ACTION,  /* Get AI opponent's action */
    AI_REQ_CARD_DESCRIPTION  /* Get card flavor text */
} AIRequestType;
/* }}} */

/* {{{ AIResponseStatus
 * Status of AI response.
 */
typedef enum {
    AI_STATUS_PENDING,
    AI_STATUS_STREAMING,
    AI_STATUS_COMPLETE,
    AI_STATUS_ERROR,
    AI_STATUS_CANCELLED
} AIResponseStatus;
/* }}} */

/* {{{ NarrativeContext
 * Context for narrative generation.
 */
typedef struct {
    const char* event_type;        /* "play_card", "attack", "buy", etc. */
    const char* card_name;         /* Card involved (if any) */
    const char* card_faction;      /* Card faction */
    int value;                     /* Value involved (damage, cost, etc.) */
    const char* target_name;       /* Target name (if any) */
    bool is_player_action;         /* true if player, false if AI */
} NarrativeContext;
/* }}} */

/* {{{ AIHint
 * AI play suggestion.
 */
typedef struct {
    const char* action_type;       /* Suggested action */
    int target_id;                 /* Target card/base ID */
    char reason[MAX_ACTION_REASON_LEN]; /* Why this is recommended */
    float confidence;              /* 0.0-1.0 confidence score */
} AIHint;
/* }}} */

/* {{{ AIOpponentAction
 * AI opponent's chosen action.
 */
typedef struct {
    const char* action_type;
    int target_id;
    char reasoning[MAX_ACTION_REASON_LEN];
    float think_time;              /* Simulated "thinking" time in seconds */
} AIOpponentAction;
/* }}} */

/* {{{ Callback types */
typedef void (*NarrativeCallback)(const char* text, bool is_complete, void* user_data);
typedef void (*HintCallback)(const AIHint* hint, void* user_data);
typedef void (*OpponentActionCallback)(const AIOpponentAction* action, void* user_data);
typedef void (*ErrorCallback)(const char* error, void* user_data);
/* }}} */

/* {{{ AICallbacks
 * All AI-related callbacks.
 */
typedef struct {
    NarrativeCallback on_narrative;
    HintCallback on_hint;
    OpponentActionCallback on_opponent_action;
    ErrorCallback on_error;
    void* user_data;
} AICallbacks;
/* }}} */

/* {{{ ai_init
 * Initialize AI hooks.
 * @param callbacks - Event callbacks
 * @return true on success
 */
bool ai_init(const AICallbacks* callbacks);
/* }}} */

/* {{{ ai_cleanup
 * Clean up AI hooks.
 */
void ai_cleanup(void);
/* }}} */

/* {{{ ai_request_narrative
 * Request narrative text for an event.
 * @param ctx - Narrative context
 * @return Request ID, or -1 on error
 */
int ai_request_narrative(const NarrativeContext* ctx);
/* }}} */

/* {{{ ai_request_hint
 * Request a play suggestion.
 * @param game_state_json - Current game state as JSON
 * @return Request ID, or -1 on error
 */
int ai_request_hint(const char* game_state_json);
/* }}} */

/* {{{ ai_request_opponent_action
 * Request AI opponent's next action.
 * @param game_state_json - Current game state as JSON
 * @param difficulty - AI difficulty (0-2)
 * @return Request ID, or -1 on error
 */
int ai_request_opponent_action(const char* game_state_json, int difficulty);
/* }}} */

/* {{{ ai_cancel_request
 * Cancel a pending AI request.
 * @param request_id - Request to cancel
 */
void ai_cancel_request(int request_id);
/* }}} */

/* {{{ ai_cancel_all
 * Cancel all pending AI requests.
 */
void ai_cancel_all(void);
/* }}} */

/* {{{ ai_get_status
 * Get status of an AI request.
 * @param request_id - Request ID
 * @return Current status
 */
AIResponseStatus ai_get_status(int request_id);
/* }}} */

/* {{{ ai_is_busy
 * Check if any AI requests are pending.
 * @return true if requests are in progress
 */
bool ai_is_busy(void);
/* }}} */

/* {{{ ai_set_enabled
 * Enable or disable AI features.
 * @param narratives - Enable narrative generation
 * @param hints - Enable play hints
 * @param opponent - Enable AI opponent
 */
void ai_set_enabled(bool narratives, bool hints, bool opponent);
/* }}} */

/* {{{ ai_update
 * Process pending AI responses.
 * Should be called each frame.
 */
void ai_update(void);
/* }}} */

#endif /* WASM_AI_HOOKS_H */
