/* test-protocol.c - Tests for Protocol Implementation
 *
 * Tests message parsing, validation, serialization, and handlers.
 * Run with: make test-protocol
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "../src/net/04-protocol.h"
#include "../src/core/05-game.h"
#include "../libs/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========================================================================== */
/*                              Test Helpers                                  */
/* ========================================================================== */

static int tests_passed = 0;
static int tests_failed = 0;

/* {{{ TEST macro */
#define TEST(name, condition) do { \
    if (condition) { \
        printf("  [PASS] %s\n", name); \
        tests_passed++; \
    } else { \
        printf("  [FAIL] %s\n", name); \
        tests_failed++; \
    } \
} while(0)
/* }}} */

/* ========================================================================== */
/*                        Message Type Conversion Tests                       */
/* ========================================================================== */

/* {{{ test_message_type_conversion */
static void test_message_type_conversion(void) {
    printf("\n=== Message Type Conversion Tests ===\n");

    /* Test string to enum */
    TEST("join -> MSG_JOIN",
         message_type_from_string("join") == MSG_JOIN);
    TEST("leave -> MSG_LEAVE",
         message_type_from_string("leave") == MSG_LEAVE);
    TEST("action -> MSG_ACTION",
         message_type_from_string("action") == MSG_ACTION);
    TEST("draw_order -> MSG_DRAW_ORDER",
         message_type_from_string("draw_order") == MSG_DRAW_ORDER);
    TEST("chat -> MSG_CHAT",
         message_type_from_string("chat") == MSG_CHAT);
    TEST("end_turn -> MSG_END_TURN",
         message_type_from_string("end_turn") == MSG_END_TURN);
    TEST("gamestate -> MSG_GAMESTATE",
         message_type_from_string("gamestate") == MSG_GAMESTATE);
    TEST("narrative -> MSG_NARRATIVE",
         message_type_from_string("narrative") == MSG_NARRATIVE);
    TEST("error -> MSG_ERROR",
         message_type_from_string("error") == MSG_ERROR);
    TEST("player_joined -> MSG_PLAYER_JOINED",
         message_type_from_string("player_joined") == MSG_PLAYER_JOINED);
    TEST("player_left -> MSG_PLAYER_LEFT",
         message_type_from_string("player_left") == MSG_PLAYER_LEFT);

    /* Test unknown type */
    TEST("unknown -> MSG_TYPE_COUNT",
         message_type_from_string("unknown_type") == MSG_TYPE_COUNT);
    TEST("NULL -> MSG_TYPE_COUNT",
         message_type_from_string(NULL) == MSG_TYPE_COUNT);

    /* Test enum to string */
    TEST("MSG_JOIN -> join",
         strcmp(message_type_to_string(MSG_JOIN), "join") == 0);
    TEST("MSG_GAMESTATE -> gamestate",
         strcmp(message_type_to_string(MSG_GAMESTATE), "gamestate") == 0);
    TEST("Invalid -> unknown",
         strcmp(message_type_to_string(MSG_TYPE_COUNT), "unknown") == 0);
}
/* }}} */

/* ========================================================================== */
/*                        Error Code Tests                                    */
/* ========================================================================== */

/* {{{ test_error_codes */
static void test_error_codes(void) {
    printf("\n=== Error Code Tests ===\n");

    TEST("PROTOCOL_OK message",
         strcmp(protocol_error_to_string(PROTOCOL_OK), "OK") == 0);
    TEST("MALFORMED_JSON message",
         strstr(protocol_error_to_string(PROTOCOL_ERROR_MALFORMED_JSON), "JSON") != NULL);
    TEST("NOT_YOUR_TURN message",
         strstr(protocol_error_to_string(PROTOCOL_ERROR_NOT_YOUR_TURN), "turn") != NULL);

    TEST("PROTOCOL_OK code",
         strcmp(protocol_error_code(PROTOCOL_OK), "ok") == 0);
    TEST("NOT_YOUR_TURN code",
         strcmp(protocol_error_code(PROTOCOL_ERROR_NOT_YOUR_TURN), "not_your_turn") == 0);
    TEST("CARD_NOT_FOUND code",
         strcmp(protocol_error_code(PROTOCOL_ERROR_CARD_NOT_FOUND), "card_not_found") == 0);
}
/* }}} */

/* ========================================================================== */
/*                        Message Lifecycle Tests                             */
/* ========================================================================== */

/* {{{ test_message_lifecycle */
static void test_message_lifecycle(void) {
    printf("\n=== Message Lifecycle Tests ===\n");

    /* Test creation */
    Message* msg = message_create(MSG_JOIN);
    TEST("Message created", msg != NULL);
    TEST("Message type set", msg && msg->type == MSG_JOIN);
    TEST("Message has payload", msg && msg->payload != NULL);
    TEST("Payload has type field", msg &&
         cJSON_GetObjectItem(msg->payload, "type") != NULL);
    TEST("Timestamp set", msg && msg->timestamp > 0);

    /* Test free */
    message_free(msg);
    TEST("Message freed (no crash)", true);

    /* Test NULL free */
    message_free(NULL);
    TEST("NULL free safe", true);

    /* Test all message types can be created */
    for (int i = 0; i < MSG_TYPE_COUNT; i++) {
        msg = message_create((MessageType)i);
        char test_name[64];
        snprintf(test_name, sizeof(test_name), "Create message type %d", i);
        TEST(test_name, msg != NULL);
        message_free(msg);
    }
}
/* }}} */

/* ========================================================================== */
/*                        Parsing Tests                                       */
/* ========================================================================== */

/* {{{ test_protocol_parse */
static void test_protocol_parse(void) {
    printf("\n=== Protocol Parsing Tests ===\n");

    ProtocolError err;

    /* Test valid join message */
    const char* join_json = "{\"type\": \"join\", \"name\": \"Alice\"}";
    Message* msg = protocol_parse(join_json, &err);
    TEST("Parse join message", msg != NULL);
    TEST("Join parse no error", err == PROTOCOL_OK);
    TEST("Join type correct", msg && msg->type == MSG_JOIN);
    message_free(msg);

    /* Test valid action message */
    const char* action_json = "{\"type\": \"action\", \"action\": \"play_card\", \"card_id\": \"abc123\"}";
    msg = protocol_parse(action_json, &err);
    TEST("Parse action message", msg != NULL);
    TEST("Action type correct", msg && msg->type == MSG_ACTION);
    message_free(msg);

    /* Test malformed JSON */
    msg = protocol_parse("not json at all", &err);
    TEST("Malformed JSON returns NULL", msg == NULL);
    TEST("Malformed JSON error code", err == PROTOCOL_ERROR_MALFORMED_JSON);

    /* Test missing type */
    msg = protocol_parse("{\"name\": \"Alice\"}", &err);
    TEST("Missing type returns NULL", msg == NULL);
    TEST("Missing type error code", err == PROTOCOL_ERROR_MISSING_TYPE);

    /* Test unknown type */
    msg = protocol_parse("{\"type\": \"invalid_type\"}", &err);
    TEST("Unknown type returns NULL", msg == NULL);
    TEST("Unknown type error code", err == PROTOCOL_ERROR_UNKNOWN_TYPE);

    /* Test NULL input */
    msg = protocol_parse(NULL, &err);
    TEST("NULL input returns NULL", msg == NULL);
    TEST("NULL input error code", err == PROTOCOL_ERROR_MALFORMED_JSON);
}
/* }}} */

/* ========================================================================== */
/*                        Serialization Tests                                 */
/* ========================================================================== */

/* {{{ test_protocol_serialize */
static void test_protocol_serialize(void) {
    printf("\n=== Protocol Serialization Tests ===\n");

    /* Create and serialize a message */
    Message* msg = message_create(MSG_JOIN);
    cJSON_AddStringToObject(msg->payload, "name", "Alice");

    char* json = protocol_serialize(msg);
    TEST("Serialize returns string", json != NULL);
    TEST("Contains type field", json && strstr(json, "\"type\"") != NULL);
    TEST("Contains join type", json && strstr(json, "\"join\"") != NULL);
    TEST("Contains name", json && strstr(json, "\"Alice\"") != NULL);

    free(json);
    message_free(msg);

    /* Test NULL message */
    json = protocol_serialize(NULL);
    TEST("NULL message returns NULL", json == NULL);
}
/* }}} */

/* ========================================================================== */
/*                        Validation Helper Tests                             */
/* ========================================================================== */

/* {{{ test_validation_helpers */
static void test_validation_helpers(void) {
    printf("\n=== Validation Helper Tests ===\n");

    cJSON* obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "name", "Alice");
    cJSON_AddNumberToObject(obj, "slot", 3);
    cJSON* arr = cJSON_AddArrayToObject(obj, "order");
    cJSON_AddItemToArray(arr, cJSON_CreateNumber(1));
    cJSON* nested = cJSON_AddObjectToObject(obj, "nested");
    cJSON_AddStringToObject(nested, "field", "value");

    /* Test string validation */
    TEST("String present: OK",
         protocol_validate_has_string(obj, "name") == PROTOCOL_OK);
    TEST("String missing: MISSING_FIELD",
         protocol_validate_has_string(obj, "missing") == PROTOCOL_ERROR_MISSING_FIELD);
    TEST("String wrong type: INVALID_FIELD_TYPE",
         protocol_validate_has_string(obj, "slot") == PROTOCOL_ERROR_INVALID_FIELD_TYPE);

    /* Test number validation */
    TEST("Number present: OK",
         protocol_validate_has_number(obj, "slot") == PROTOCOL_OK);
    TEST("Number missing: MISSING_FIELD",
         protocol_validate_has_number(obj, "missing") == PROTOCOL_ERROR_MISSING_FIELD);
    TEST("Number wrong type: INVALID_FIELD_TYPE",
         protocol_validate_has_number(obj, "name") == PROTOCOL_ERROR_INVALID_FIELD_TYPE);

    /* Test array validation */
    TEST("Array present: OK",
         protocol_validate_has_array(obj, "order") == PROTOCOL_OK);
    TEST("Array missing: MISSING_FIELD",
         protocol_validate_has_array(obj, "missing") == PROTOCOL_ERROR_MISSING_FIELD);
    TEST("Array wrong type: INVALID_FIELD_TYPE",
         protocol_validate_has_array(obj, "name") == PROTOCOL_ERROR_INVALID_FIELD_TYPE);

    /* Test object validation */
    TEST("Object present: OK",
         protocol_validate_has_object(obj, "nested") == PROTOCOL_OK);
    TEST("Object missing: MISSING_FIELD",
         protocol_validate_has_object(obj, "missing") == PROTOCOL_ERROR_MISSING_FIELD);
    TEST("Object wrong type: INVALID_FIELD_TYPE",
         protocol_validate_has_object(obj, "name") == PROTOCOL_ERROR_INVALID_FIELD_TYPE);

    /* Test range validation */
    TEST("Range valid: OK",
         protocol_validate_number_range(obj, "slot", 0, 4) == PROTOCOL_OK);
    TEST("Range too low: INVALID_VALUE",
         protocol_validate_number_range(obj, "slot", 4, 5) == PROTOCOL_ERROR_INVALID_VALUE);
    TEST("Range too high: INVALID_VALUE",
         protocol_validate_number_range(obj, "slot", 0, 2) == PROTOCOL_ERROR_INVALID_VALUE);

    cJSON_Delete(obj);
}
/* }}} */

/* ========================================================================== */
/*                        Server Message Generation Tests                     */
/* ========================================================================== */

/* {{{ test_server_messages */
static void test_server_messages(void) {
    printf("\n=== Server Message Generation Tests ===\n");

    /* Test narrative message */
    Message* msg = protocol_create_narrative("The dire bear charges!");
    TEST("Narrative created", msg != NULL);
    TEST("Narrative type correct", msg && msg->type == MSG_NARRATIVE);
    cJSON* text = msg ? cJSON_GetObjectItem(msg->payload, "text") : NULL;
    TEST("Narrative has text", text && cJSON_IsString(text));
    TEST("Narrative text correct", text &&
         strcmp(text->valuestring, "The dire bear charges!") == 0);
    message_free(msg);

    /* Test error message */
    msg = protocol_create_error(PROTOCOL_ERROR_NOT_YOUR_TURN, "Wait for Alice");
    TEST("Error created", msg != NULL);
    TEST("Error type correct", msg && msg->type == MSG_ERROR);
    cJSON* code = msg ? cJSON_GetObjectItem(msg->payload, "code") : NULL;
    TEST("Error has code", code && cJSON_IsString(code));
    TEST("Error code correct", code &&
         strcmp(code->valuestring, "not_your_turn") == 0);
    cJSON* details = msg ? cJSON_GetObjectItem(msg->payload, "details") : NULL;
    TEST("Error has details", details && cJSON_IsString(details));
    message_free(msg);

    /* Test player joined */
    msg = protocol_create_player_joined(1, "Bob");
    TEST("Player joined created", msg != NULL);
    TEST("Player joined type", msg && msg->type == MSG_PLAYER_JOINED);
    cJSON* pid = msg ? cJSON_GetObjectItem(msg->payload, "player_id") : NULL;
    TEST("Player ID present", pid && pid->valuedouble == 1);
    cJSON* name = msg ? cJSON_GetObjectItem(msg->payload, "name") : NULL;
    TEST("Player name present", name && strcmp(name->valuestring, "Bob") == 0);
    message_free(msg);

    /* Test player left */
    msg = protocol_create_player_left(2);
    TEST("Player left created", msg != NULL);
    TEST("Player left type", msg && msg->type == MSG_PLAYER_LEFT);
    pid = msg ? cJSON_GetObjectItem(msg->payload, "player_id") : NULL;
    TEST("Player ID in left", pid && pid->valuedouble == 2);
    message_free(msg);

    /* Test draw order request */
    msg = protocol_create_draw_order_request(5);
    TEST("Draw order request created", msg != NULL);
    TEST("Draw order request type", msg && msg->type == MSG_DRAW_ORDER_REQUEST);
    cJSON* count = msg ? cJSON_GetObjectItem(msg->payload, "count") : NULL;
    TEST("Count field present", count && count->valuedouble == 5);
    message_free(msg);

    /* Test choice request */
    cJSON* options = cJSON_CreateArray();
    cJSON_AddItemToArray(options, cJSON_CreateString("card_1"));
    cJSON_AddItemToArray(options, cJSON_CreateString("card_2"));
    msg = protocol_create_choice_request(CHOICE_SCRAP_HAND, options);
    TEST("Choice request created", msg != NULL);
    TEST("Choice request type", msg && msg->type == MSG_CHOICE_REQUEST);
    cJSON* choice_type = msg ? cJSON_GetObjectItem(msg->payload, "choice_type") : NULL;
    TEST("Choice type field", choice_type &&
         strcmp(choice_type->valuestring, "scrap_hand") == 0);
    cJSON* opts = msg ? cJSON_GetObjectItem(msg->payload, "options") : NULL;
    TEST("Options present", opts && cJSON_IsArray(opts));
    TEST("Options count", opts && cJSON_GetArraySize(opts) == 2);
    message_free(msg);
    cJSON_Delete(options);

    /* Test game over */
    msg = protocol_create_game_over(0, "Player 2 has no authority");
    TEST("Game over created", msg != NULL);
    TEST("Game over type", msg && msg->type == MSG_GAME_OVER);
    cJSON* winner = msg ? cJSON_GetObjectItem(msg->payload, "winner_id") : NULL;
    TEST("Winner ID present", winner && winner->valuedouble == 0);
    cJSON* reason = msg ? cJSON_GetObjectItem(msg->payload, "reason") : NULL;
    TEST("Reason present", reason && strstr(reason->valuestring, "authority") != NULL);
    message_free(msg);
}
/* }}} */

/* ========================================================================== */
/*                        Handler Dispatch Tests                              */
/* ========================================================================== */

/* {{{ test_handler_dispatch */
static void test_handler_dispatch(void) {
    printf("\n=== Handler Dispatch Tests ===\n");

    /* Test handler lookup */
    TEST("Join has handler", protocol_get_handler(MSG_JOIN) != NULL);
    TEST("Action has handler", protocol_get_handler(MSG_ACTION) != NULL);
    TEST("End turn has handler", protocol_get_handler(MSG_END_TURN) != NULL);
    TEST("Chat has handler", protocol_get_handler(MSG_CHAT) != NULL);

    /* Server messages should NOT have handlers */
    TEST("Gamestate no handler", protocol_get_handler(MSG_GAMESTATE) == NULL);
    TEST("Narrative no handler", protocol_get_handler(MSG_NARRATIVE) == NULL);
    TEST("Error no handler", protocol_get_handler(MSG_ERROR) == NULL);

    /* Test invalid type */
    TEST("Invalid type no handler", protocol_get_handler(MSG_TYPE_COUNT) == NULL);
}
/* }}} */

/* ========================================================================== */
/*                        Handler Validation Tests                            */
/* ========================================================================== */

/* {{{ test_handler_validation */
static void test_handler_validation(void) {
    printf("\n=== Handler Validation Tests ===\n");

    ProtocolError err;

    /* Test join validation */
    Message* msg = protocol_parse("{\"type\": \"join\", \"name\": \"Alice\"}", &err);
    TEST("Valid join parses", msg != NULL);

    /* Join without game - should fail */
    err = protocol_dispatch(NULL, -1, msg, NULL);
    TEST("Join without game fails", err == PROTOCOL_ERROR_GAME_NOT_STARTED);
    message_free(msg);

    /* Test action validation (need proper game state) */
    msg = protocol_parse("{\"type\": \"action\", \"action\": \"play_card\", \"card_id\": \"abc\"}", &err);
    TEST("Valid action parses", msg != NULL);
    err = protocol_dispatch(NULL, 0, msg, NULL);
    TEST("Action without game fails", err == PROTOCOL_ERROR_GAME_NOT_STARTED);
    message_free(msg);

    /* Test invalid action type */
    msg = protocol_parse("{\"type\": \"action\", \"action\": \"invalid_action\"}", &err);
    TEST("Invalid action parses", msg != NULL);

    /* Create minimal game for action testing */
    Game* game = game_create(2);
    game_add_player(game, "Alice");
    game_add_player(game, "Bob");
    game->phase = PHASE_MAIN;
    game->active_player = 0;

    err = protocol_dispatch(game, 0, msg, NULL);
    TEST("Invalid action rejected", err == PROTOCOL_ERROR_INVALID_ACTION);
    message_free(msg);

    /* Test not your turn */
    msg = protocol_parse("{\"type\": \"action\", \"action\": \"play_card\", \"card_id\": \"x\"}", &err);
    err = protocol_dispatch(game, 1, msg, NULL);  /* Player 1 but it's player 0's turn */
    TEST("Not your turn rejected", err == PROTOCOL_ERROR_NOT_YOUR_TURN);
    message_free(msg);

    /* Test invalid phase */
    game->phase = PHASE_DRAW_ORDER;
    msg = protocol_parse("{\"type\": \"action\", \"action\": \"play_card\", \"card_id\": \"x\"}", &err);
    err = protocol_dispatch(game, 0, msg, NULL);
    TEST("Wrong phase rejected", err == PROTOCOL_ERROR_INVALID_PHASE);
    message_free(msg);

    /* Test draw order in correct phase */
    msg = protocol_parse("{\"type\": \"draw_order\", \"order\": [0, 1, 2]}", &err);
    err = protocol_dispatch(game, 0, msg, NULL);
    TEST("Draw order in correct phase OK", err == PROTOCOL_OK);
    message_free(msg);

    /* Test draw order wrong phase */
    game->phase = PHASE_MAIN;
    msg = protocol_parse("{\"type\": \"draw_order\", \"order\": [0, 1, 2]}", &err);
    err = protocol_dispatch(game, 0, msg, NULL);
    TEST("Draw order wrong phase rejected", err == PROTOCOL_ERROR_INVALID_PHASE);
    message_free(msg);

    /* Test end turn validation */
    msg = protocol_parse("{\"type\": \"end_turn\"}", &err);
    err = protocol_dispatch(game, 0, msg, NULL);
    TEST("End turn in main phase OK", err == PROTOCOL_OK);
    message_free(msg);

    /* Test chat (always valid) */
    msg = protocol_parse("{\"type\": \"chat\", \"message\": \"Hello!\"}", &err);
    err = protocol_dispatch(game, 0, msg, NULL);
    TEST("Chat always OK", err == PROTOCOL_OK);
    message_free(msg);

    game_free(game);
}
/* }}} */

/* ========================================================================== */
/*                        Round-Trip Tests                                    */
/* ========================================================================== */

/* {{{ test_round_trip */
static void test_round_trip(void) {
    printf("\n=== Round-Trip Tests ===\n");

    ProtocolError err;

    /* Create message, serialize, parse, verify */
    Message* original = message_create(MSG_ACTION);
    cJSON_AddStringToObject(original->payload, "action", "buy_card");
    cJSON_AddNumberToObject(original->payload, "slot", 2);

    char* json = protocol_serialize(original);
    TEST("Serialize succeeded", json != NULL);

    Message* parsed = protocol_parse(json, &err);
    TEST("Parse succeeded", parsed != NULL);
    TEST("Type preserved", parsed && parsed->type == MSG_ACTION);

    cJSON* action = parsed ? cJSON_GetObjectItem(parsed->payload, "action") : NULL;
    TEST("Action field preserved", action &&
         strcmp(action->valuestring, "buy_card") == 0);

    cJSON* slot = parsed ? cJSON_GetObjectItem(parsed->payload, "slot") : NULL;
    TEST("Slot field preserved", slot && slot->valuedouble == 2);

    free(json);
    message_free(original);
    message_free(parsed);
}
/* }}} */

/* ========================================================================== */
/*                               Main                                         */
/* ========================================================================== */

/* {{{ main */
int main(void) {
    printf("Symbeline Realms - Protocol Tests (2-005)\n");
    printf("==========================================\n");

    test_message_type_conversion();
    test_error_codes();
    test_message_lifecycle();
    test_protocol_parse();
    test_protocol_serialize();
    test_validation_helpers();
    test_server_messages();
    test_handler_dispatch();
    test_handler_validation();
    test_round_trip();

    printf("\n==========================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
/* }}} */
