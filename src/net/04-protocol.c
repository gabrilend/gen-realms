/* 04-protocol.c - Game Protocol Implementation
 *
 * Implements message parsing, validation, serialization, and dispatch.
 * Uses cJSON for JSON handling. Integrates with gamestate serialization
 * from src/core/09-serialize.h.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "04-protocol.h"
#include "../core/05-game.h"
#include "../core/09-serialize.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========================================================================== */
/*                           Static String Tables                             */
/* ========================================================================== */

/* {{{ Message type strings (for JSON) */
static const char* MESSAGE_TYPE_STRINGS[] = {
    [MSG_JOIN] = "join",
    [MSG_LEAVE] = "leave",
    [MSG_ACTION] = "action",
    [MSG_DRAW_ORDER] = "draw_order",
    [MSG_CHAT] = "chat",
    [MSG_END_TURN] = "end_turn",
    [MSG_GAMESTATE] = "gamestate",
    [MSG_NARRATIVE] = "narrative",
    [MSG_ERROR] = "error",
    [MSG_PLAYER_JOINED] = "player_joined",
    [MSG_PLAYER_LEFT] = "player_left",
    [MSG_DRAW_ORDER_REQUEST] = "draw_order_request",
    [MSG_CHOICE_REQUEST] = "choice_request",
    [MSG_GAME_OVER] = "game_over"
};
/* }}} */

/* {{{ Error messages */
static const char* ERROR_MESSAGES[] = {
    [PROTOCOL_OK] = "OK",
    [PROTOCOL_ERROR_MALFORMED_JSON] = "Malformed JSON",
    [PROTOCOL_ERROR_MISSING_TYPE] = "Missing 'type' field",
    [PROTOCOL_ERROR_UNKNOWN_TYPE] = "Unknown message type",
    [PROTOCOL_ERROR_MISSING_FIELD] = "Missing required field",
    [PROTOCOL_ERROR_INVALID_FIELD_TYPE] = "Invalid field type",
    [PROTOCOL_ERROR_INVALID_VALUE] = "Invalid value",
    [PROTOCOL_ERROR_NOT_YOUR_TURN] = "It's not your turn",
    [PROTOCOL_ERROR_INVALID_PHASE] = "Invalid action for current phase",
    [PROTOCOL_ERROR_GAME_NOT_STARTED] = "Game has not started",
    [PROTOCOL_ERROR_GAME_ALREADY_STARTED] = "Game has already started",
    [PROTOCOL_ERROR_GAME_FULL] = "Game is full",
    [PROTOCOL_ERROR_NOT_IN_GAME] = "You are not in the game",
    [PROTOCOL_ERROR_INVALID_ACTION] = "Invalid action",
    [PROTOCOL_ERROR_CARD_NOT_FOUND] = "Card not found",
    [PROTOCOL_ERROR_CARD_NOT_IN_HAND] = "Card is not in your hand",
    [PROTOCOL_ERROR_INSUFFICIENT_TRADE] = "Not enough trade",
    [PROTOCOL_ERROR_INSUFFICIENT_COMBAT] = "Not enough combat",
    [PROTOCOL_ERROR_INVALID_TARGET] = "Invalid target",
    [PROTOCOL_ERROR_INVALID_SLOT] = "Invalid slot index",
    [PROTOCOL_ERROR_INVALID_DRAW_ORDER] = "Invalid draw order"
};
/* }}} */

/* {{{ Error codes (short codes for JSON) */
static const char* ERROR_CODES[] = {
    [PROTOCOL_OK] = "ok",
    [PROTOCOL_ERROR_MALFORMED_JSON] = "malformed_json",
    [PROTOCOL_ERROR_MISSING_TYPE] = "missing_type",
    [PROTOCOL_ERROR_UNKNOWN_TYPE] = "unknown_type",
    [PROTOCOL_ERROR_MISSING_FIELD] = "missing_field",
    [PROTOCOL_ERROR_INVALID_FIELD_TYPE] = "invalid_field_type",
    [PROTOCOL_ERROR_INVALID_VALUE] = "invalid_value",
    [PROTOCOL_ERROR_NOT_YOUR_TURN] = "not_your_turn",
    [PROTOCOL_ERROR_INVALID_PHASE] = "invalid_phase",
    [PROTOCOL_ERROR_GAME_NOT_STARTED] = "game_not_started",
    [PROTOCOL_ERROR_GAME_ALREADY_STARTED] = "game_already_started",
    [PROTOCOL_ERROR_GAME_FULL] = "game_full",
    [PROTOCOL_ERROR_NOT_IN_GAME] = "not_in_game",
    [PROTOCOL_ERROR_INVALID_ACTION] = "invalid_action",
    [PROTOCOL_ERROR_CARD_NOT_FOUND] = "card_not_found",
    [PROTOCOL_ERROR_CARD_NOT_IN_HAND] = "card_not_in_hand",
    [PROTOCOL_ERROR_INSUFFICIENT_TRADE] = "insufficient_trade",
    [PROTOCOL_ERROR_INSUFFICIENT_COMBAT] = "insufficient_combat",
    [PROTOCOL_ERROR_INVALID_TARGET] = "invalid_target",
    [PROTOCOL_ERROR_INVALID_SLOT] = "invalid_slot",
    [PROTOCOL_ERROR_INVALID_DRAW_ORDER] = "invalid_draw_order"
};
/* }}} */

/* {{{ Choice type strings */
static const char* CHOICE_TYPE_STRINGS[] = {
    [CHOICE_DRAW_ORDER] = "draw_order",
    [CHOICE_SCRAP_HAND] = "scrap_hand",
    [CHOICE_SCRAP_DISCARD] = "scrap_discard",
    [CHOICE_SCRAP_TRADE_ROW] = "scrap_trade_row",
    [CHOICE_DISCARD] = "discard",
    [CHOICE_ACQUIRE_FREE] = "acquire_free",
    [CHOICE_BASE_PLACEMENT] = "base_placement"
};
/* }}} */

/* ========================================================================== */
/*                           Utility Functions                                */
/* ========================================================================== */

/* {{{ get_timestamp_ms */
static uint64_t get_timestamp_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}
/* }}} */

/* ========================================================================== */
/*                            Message Lifecycle                               */
/* ========================================================================== */

/* {{{ message_create */
Message* message_create(MessageType type) {
    Message* msg = malloc(sizeof(Message));
    if (!msg) return NULL;

    msg->type = type;
    msg->player_id = -1;
    msg->payload = cJSON_CreateObject();
    msg->timestamp = get_timestamp_ms();

    if (!msg->payload) {
        free(msg);
        return NULL;
    }

    /* Add type string to payload */
    if (type < MSG_TYPE_COUNT) {
        cJSON_AddStringToObject(msg->payload, "type", MESSAGE_TYPE_STRINGS[type]);
    }

    return msg;
}
/* }}} */

/* {{{ message_free */
void message_free(Message* msg) {
    if (!msg) return;

    if (msg->payload) {
        cJSON_Delete(msg->payload);
    }
    free(msg);
}
/* }}} */

/* ========================================================================== */
/*                            Type Conversion                                 */
/* ========================================================================== */

/* {{{ message_type_from_string */
MessageType message_type_from_string(const char* str) {
    if (!str) return MSG_TYPE_COUNT;

    for (int i = 0; i < MSG_TYPE_COUNT; i++) {
        if (MESSAGE_TYPE_STRINGS[i] && strcmp(str, MESSAGE_TYPE_STRINGS[i]) == 0) {
            return (MessageType)i;
        }
    }

    return MSG_TYPE_COUNT;
}
/* }}} */

/* {{{ message_type_to_string */
const char* message_type_to_string(MessageType type) {
    if (type >= MSG_TYPE_COUNT) return "unknown";
    return MESSAGE_TYPE_STRINGS[type] ? MESSAGE_TYPE_STRINGS[type] : "unknown";
}
/* }}} */

/* {{{ protocol_error_to_string */
const char* protocol_error_to_string(ProtocolError error) {
    if (error >= PROTOCOL_ERROR_COUNT) return "Unknown error";
    return ERROR_MESSAGES[error] ? ERROR_MESSAGES[error] : "Unknown error";
}
/* }}} */

/* {{{ protocol_error_code */
const char* protocol_error_code(ProtocolError error) {
    if (error >= PROTOCOL_ERROR_COUNT) return "unknown_error";
    return ERROR_CODES[error] ? ERROR_CODES[error] : "unknown_error";
}
/* }}} */

/* {{{ choice_type_to_string */
const char* choice_type_to_string(ChoiceType type) {
    if (type >= CHOICE_TYPE_COUNT) return "unknown";
    return CHOICE_TYPE_STRINGS[type] ? CHOICE_TYPE_STRINGS[type] : "unknown";
}
/* }}} */

/* ========================================================================== */
/*                            Protocol Parsing                                */
/* ========================================================================== */

/* {{{ protocol_parse */
Message* protocol_parse(const char* json, ProtocolError* error) {
    if (!json) {
        if (error) *error = PROTOCOL_ERROR_MALFORMED_JSON;
        return NULL;
    }

    /* Parse JSON */
    cJSON* root = cJSON_Parse(json);
    if (!root) {
        if (error) *error = PROTOCOL_ERROR_MALFORMED_JSON;
        return NULL;
    }

    /* Get message type */
    cJSON* type_item = cJSON_GetObjectItem(root, "type");
    if (!type_item || !cJSON_IsString(type_item)) {
        if (error) *error = PROTOCOL_ERROR_MISSING_TYPE;
        cJSON_Delete(root);
        return NULL;
    }

    /* Convert type string to enum */
    MessageType type = message_type_from_string(type_item->valuestring);
    if (type == MSG_TYPE_COUNT) {
        if (error) *error = PROTOCOL_ERROR_UNKNOWN_TYPE;
        cJSON_Delete(root);
        return NULL;
    }

    /* Create message */
    Message* msg = malloc(sizeof(Message));
    if (!msg) {
        cJSON_Delete(root);
        if (error) *error = PROTOCOL_ERROR_MALFORMED_JSON;
        return NULL;
    }

    msg->type = type;
    msg->player_id = -1;  /* Set by caller based on connection */
    msg->payload = root;   /* Transfer ownership */
    msg->timestamp = get_timestamp_ms();

    if (error) *error = PROTOCOL_OK;
    return msg;
}
/* }}} */

/* {{{ protocol_serialize */
char* protocol_serialize(Message* msg) {
    if (!msg || !msg->payload) return NULL;

    return cJSON_PrintUnformatted(msg->payload);
}
/* }}} */

/* ========================================================================== */
/*                            Validation Helpers                              */
/* ========================================================================== */

/* {{{ protocol_validate_has_string */
ProtocolError protocol_validate_has_string(cJSON* payload, const char* field) {
    if (!payload || !field) return PROTOCOL_ERROR_MISSING_FIELD;

    cJSON* item = cJSON_GetObjectItem(payload, field);
    if (!item) return PROTOCOL_ERROR_MISSING_FIELD;
    if (!cJSON_IsString(item)) return PROTOCOL_ERROR_INVALID_FIELD_TYPE;

    return PROTOCOL_OK;
}
/* }}} */

/* {{{ protocol_validate_has_number */
ProtocolError protocol_validate_has_number(cJSON* payload, const char* field) {
    if (!payload || !field) return PROTOCOL_ERROR_MISSING_FIELD;

    cJSON* item = cJSON_GetObjectItem(payload, field);
    if (!item) return PROTOCOL_ERROR_MISSING_FIELD;
    if (!cJSON_IsNumber(item)) return PROTOCOL_ERROR_INVALID_FIELD_TYPE;

    return PROTOCOL_OK;
}
/* }}} */

/* {{{ protocol_validate_has_array */
ProtocolError protocol_validate_has_array(cJSON* payload, const char* field) {
    if (!payload || !field) return PROTOCOL_ERROR_MISSING_FIELD;

    cJSON* item = cJSON_GetObjectItem(payload, field);
    if (!item) return PROTOCOL_ERROR_MISSING_FIELD;
    if (!cJSON_IsArray(item)) return PROTOCOL_ERROR_INVALID_FIELD_TYPE;

    return PROTOCOL_OK;
}
/* }}} */

/* {{{ protocol_validate_has_object */
ProtocolError protocol_validate_has_object(cJSON* payload, const char* field) {
    if (!payload || !field) return PROTOCOL_ERROR_MISSING_FIELD;

    cJSON* item = cJSON_GetObjectItem(payload, field);
    if (!item) return PROTOCOL_ERROR_MISSING_FIELD;
    if (!cJSON_IsObject(item)) return PROTOCOL_ERROR_INVALID_FIELD_TYPE;

    return PROTOCOL_OK;
}
/* }}} */

/* {{{ protocol_validate_number_range */
ProtocolError protocol_validate_number_range(cJSON* payload, const char* field,
                                             int min, int max) {
    ProtocolError err = protocol_validate_has_number(payload, field);
    if (err != PROTOCOL_OK) return err;

    cJSON* item = cJSON_GetObjectItem(payload, field);
    int value = (int)item->valuedouble;

    if (value < min || value > max) {
        return PROTOCOL_ERROR_INVALID_VALUE;
    }

    return PROTOCOL_OK;
}
/* }}} */

/* ========================================================================== */
/*                         Server Message Generation                          */
/* ========================================================================== */

/* {{{ protocol_create_gamestate */
Message* protocol_create_gamestate(Game* game, int player_id) {
    if (!game) return NULL;

    /* Use the serialization from 09-serialize */
    cJSON* state = serialize_game_for_player(game, player_id);
    if (!state) return NULL;

    /* Create message - note we need to add "type" field */
    Message* msg = malloc(sizeof(Message));
    if (!msg) {
        cJSON_Delete(state);
        return NULL;
    }

    msg->type = MSG_GAMESTATE;
    msg->player_id = -1;
    msg->timestamp = get_timestamp_ms();

    /* Add type to the existing state object */
    cJSON_AddStringToObject(state, "type", "gamestate");
    msg->payload = state;

    return msg;
}
/* }}} */

/* {{{ protocol_create_narrative */
Message* protocol_create_narrative(const char* text) {
    Message* msg = message_create(MSG_NARRATIVE);
    if (!msg) return NULL;

    cJSON_AddStringToObject(msg->payload, "text", text ? text : "");

    return msg;
}
/* }}} */

/* {{{ protocol_create_error */
Message* protocol_create_error(ProtocolError error, const char* details) {
    Message* msg = message_create(MSG_ERROR);
    if (!msg) return NULL;

    cJSON_AddStringToObject(msg->payload, "code", protocol_error_code(error));
    cJSON_AddStringToObject(msg->payload, "message", protocol_error_to_string(error));

    if (details) {
        cJSON_AddStringToObject(msg->payload, "details", details);
    }

    return msg;
}
/* }}} */

/* {{{ protocol_create_player_joined */
Message* protocol_create_player_joined(int player_id, const char* name) {
    Message* msg = message_create(MSG_PLAYER_JOINED);
    if (!msg) return NULL;

    cJSON_AddNumberToObject(msg->payload, "player_id", player_id);
    cJSON_AddStringToObject(msg->payload, "name", name ? name : "");

    return msg;
}
/* }}} */

/* {{{ protocol_create_player_left */
Message* protocol_create_player_left(int player_id) {
    Message* msg = message_create(MSG_PLAYER_LEFT);
    if (!msg) return NULL;

    cJSON_AddNumberToObject(msg->payload, "player_id", player_id);

    return msg;
}
/* }}} */

/* {{{ protocol_create_draw_order_request */
Message* protocol_create_draw_order_request(int card_count) {
    Message* msg = message_create(MSG_DRAW_ORDER_REQUEST);
    if (!msg) return NULL;

    cJSON_AddNumberToObject(msg->payload, "count", card_count);

    return msg;
}
/* }}} */

/* {{{ protocol_create_choice_request */
Message* protocol_create_choice_request(ChoiceType type, cJSON* options) {
    Message* msg = message_create(MSG_CHOICE_REQUEST);
    if (!msg) return NULL;

    cJSON_AddStringToObject(msg->payload, "choice_type", choice_type_to_string(type));

    if (options) {
        /* Create a copy to avoid ownership issues */
        cJSON* options_copy = cJSON_Duplicate(options, 1);
        cJSON_AddItemToObject(msg->payload, "options", options_copy);
    }

    return msg;
}
/* }}} */

/* {{{ protocol_create_game_over */
Message* protocol_create_game_over(int winner_id, const char* reason) {
    Message* msg = message_create(MSG_GAME_OVER);
    if (!msg) return NULL;

    cJSON_AddNumberToObject(msg->payload, "winner_id", winner_id);
    if (reason) {
        cJSON_AddStringToObject(msg->payload, "reason", reason);
    }

    return msg;
}
/* }}} */

/* ========================================================================== */
/*                         Client Message Handlers                            */
/* ========================================================================== */

/* Forward declarations for handlers */
static ProtocolError handle_join(Game* game, int player_id, Message* msg, void* ctx);
static ProtocolError handle_leave(Game* game, int player_id, Message* msg, void* ctx);
static ProtocolError handle_action(Game* game, int player_id, Message* msg, void* ctx);
static ProtocolError handle_draw_order(Game* game, int player_id, Message* msg, void* ctx);
static ProtocolError handle_chat(Game* game, int player_id, Message* msg, void* ctx);
static ProtocolError handle_end_turn(Game* game, int player_id, Message* msg, void* ctx);

/* {{{ Handler dispatch table */
static MessageHandler CLIENT_HANDLERS[MSG_TYPE_COUNT] = {
    [MSG_JOIN] = handle_join,
    [MSG_LEAVE] = handle_leave,
    [MSG_ACTION] = handle_action,
    [MSG_DRAW_ORDER] = handle_draw_order,
    [MSG_CHAT] = handle_chat,
    [MSG_END_TURN] = handle_end_turn,
    /* Server→Client messages have no handlers */
    [MSG_GAMESTATE] = NULL,
    [MSG_NARRATIVE] = NULL,
    [MSG_ERROR] = NULL,
    [MSG_PLAYER_JOINED] = NULL,
    [MSG_PLAYER_LEFT] = NULL,
    [MSG_DRAW_ORDER_REQUEST] = NULL,
    [MSG_CHOICE_REQUEST] = NULL,
    [MSG_GAME_OVER] = NULL
};
/* }}} */

/* {{{ protocol_get_handler */
MessageHandler protocol_get_handler(MessageType type) {
    if (type >= MSG_TYPE_COUNT) return NULL;
    return CLIENT_HANDLERS[type];
}
/* }}} */

/* {{{ protocol_dispatch */
ProtocolError protocol_dispatch(Game* game, int player_id, Message* msg, void* context) {
    if (!msg) return PROTOCOL_ERROR_MALFORMED_JSON;

    MessageHandler handler = protocol_get_handler(msg->type);
    if (!handler) {
        return PROTOCOL_ERROR_UNKNOWN_TYPE;
    }

    return handler(game, player_id, msg, context);
}
/* }}} */

/* ========================================================================== */
/*                         Handler Implementations                            */
/* ========================================================================== */

/* {{{ handle_join */
static ProtocolError handle_join(Game* game, int player_id, Message* msg, void* ctx) {
    (void)player_id;  /* Player ID not assigned yet */
    (void)ctx;

    /* Validate name field */
    ProtocolError err = protocol_validate_has_string(msg->payload, "name");
    if (err != PROTOCOL_OK) return err;

    /* Check if game exists and can accept players */
    if (!game) return PROTOCOL_ERROR_GAME_NOT_STARTED;
    if (game->phase != PHASE_NOT_STARTED) {
        return PROTOCOL_ERROR_GAME_ALREADY_STARTED;
    }
    if (game->player_count >= MAX_PLAYERS) {
        return PROTOCOL_ERROR_GAME_FULL;
    }

    /* Name is valid, game can accept - actual player creation done by caller */
    return PROTOCOL_OK;
}
/* }}} */

/* {{{ handle_leave */
static ProtocolError handle_leave(Game* game, int player_id, Message* msg, void* ctx) {
    (void)msg;
    (void)ctx;

    if (!game) return PROTOCOL_ERROR_GAME_NOT_STARTED;
    if (player_id < 0 || player_id >= game->player_count) {
        return PROTOCOL_ERROR_NOT_IN_GAME;
    }

    /* Actual player removal handled by caller */
    return PROTOCOL_OK;
}
/* }}} */

/* {{{ handle_action */
static ProtocolError handle_action(Game* game, int player_id, Message* msg, void* ctx) {
    (void)ctx;

    if (!game) return PROTOCOL_ERROR_GAME_NOT_STARTED;
    if (game->phase != PHASE_MAIN) return PROTOCOL_ERROR_INVALID_PHASE;
    if (game->active_player != player_id) return PROTOCOL_ERROR_NOT_YOUR_TURN;

    /* Validate action field */
    ProtocolError err = protocol_validate_has_string(msg->payload, "action");
    if (err != PROTOCOL_OK) return err;

    cJSON* action_item = cJSON_GetObjectItem(msg->payload, "action");
    const char* action_str = action_item->valuestring;

    /* Dispatch based on action type */
    if (strcmp(action_str, "play_card") == 0) {
        err = protocol_validate_has_string(msg->payload, "card_id");
        if (err != PROTOCOL_OK) return err;
        /* Actual card play handled by caller using game_process_action */
    }
    else if (strcmp(action_str, "buy_card") == 0) {
        err = protocol_validate_number_range(msg->payload, "slot", 0, TRADE_ROW_SLOTS - 1);
        if (err != PROTOCOL_OK) return err;
    }
    else if (strcmp(action_str, "buy_explorer") == 0) {
        /* No additional validation needed */
    }
    else if (strcmp(action_str, "attack_player") == 0) {
        err = protocol_validate_has_number(msg->payload, "target");
        if (err != PROTOCOL_OK) return err;
        err = protocol_validate_has_number(msg->payload, "amount");
        if (err != PROTOCOL_OK) return err;
    }
    else if (strcmp(action_str, "attack_base") == 0) {
        err = protocol_validate_has_number(msg->payload, "target");
        if (err != PROTOCOL_OK) return err;
        err = protocol_validate_has_string(msg->payload, "base_id");
        if (err != PROTOCOL_OK) return err;
        err = protocol_validate_has_number(msg->payload, "amount");
        if (err != PROTOCOL_OK) return err;
    }
    else if (strcmp(action_str, "scrap_hand") == 0 ||
             strcmp(action_str, "scrap_discard") == 0) {
        err = protocol_validate_has_string(msg->payload, "card_id");
        if (err != PROTOCOL_OK) return err;
    }
    else if (strcmp(action_str, "scrap_trade_row") == 0) {
        err = protocol_validate_number_range(msg->payload, "slot", 0, TRADE_ROW_SLOTS - 1);
        if (err != PROTOCOL_OK) return err;
    }
    else {
        return PROTOCOL_ERROR_INVALID_ACTION;
    }

    return PROTOCOL_OK;
}
/* }}} */

/* {{{ handle_draw_order */
static ProtocolError handle_draw_order(Game* game, int player_id, Message* msg, void* ctx) {
    (void)ctx;

    if (!game) return PROTOCOL_ERROR_GAME_NOT_STARTED;
    if (game->phase != PHASE_DRAW_ORDER) return PROTOCOL_ERROR_INVALID_PHASE;
    if (game->active_player != player_id) return PROTOCOL_ERROR_NOT_YOUR_TURN;

    /* Validate order array */
    ProtocolError err = protocol_validate_has_array(msg->payload, "order");
    if (err != PROTOCOL_OK) return err;

    cJSON* order_array = cJSON_GetObjectItem(msg->payload, "order");
    int count = cJSON_GetArraySize(order_array);

    /* Validate each element is a number */
    for (int i = 0; i < count; i++) {
        cJSON* item = cJSON_GetArrayItem(order_array, i);
        if (!cJSON_IsNumber(item)) {
            return PROTOCOL_ERROR_INVALID_DRAW_ORDER;
        }
    }

    return PROTOCOL_OK;
}
/* }}} */

/* {{{ handle_chat */
static ProtocolError handle_chat(Game* game, int player_id, Message* msg, void* ctx) {
    (void)game;
    (void)player_id;
    (void)ctx;

    /* Validate message field */
    ProtocolError err = protocol_validate_has_string(msg->payload, "message");
    if (err != PROTOCOL_OK) return err;

    /* Chat doesn't require game to be started */
    return PROTOCOL_OK;
}
/* }}} */

/* {{{ handle_end_turn */
static ProtocolError handle_end_turn(Game* game, int player_id, Message* msg, void* ctx) {
    (void)msg;
    (void)ctx;

    if (!game) return PROTOCOL_ERROR_GAME_NOT_STARTED;
    if (game->phase != PHASE_MAIN) return PROTOCOL_ERROR_INVALID_PHASE;
    if (game->active_player != player_id) return PROTOCOL_ERROR_NOT_YOUR_TURN;

    return PROTOCOL_OK;
}
/* }}} */
