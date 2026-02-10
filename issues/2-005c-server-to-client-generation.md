# 2-005c: Server to Client Message Generation

## Parent Issue
2-005: Protocol Implementation

## Current Behavior
No server→client message generation.

## Intended Behavior
Functions for generating all server→client messages:
- Gamestate updates (filtered per player)
- Narrative text
- Error responses
- Player join/leave notifications
- Choice requests (draw order, discard, etc.)

## Suggested Implementation Steps

1. Implement `protocol_send_gamestate()`:
   ```c
   // {{{ protocol_send_gamestate
   void protocol_send_gamestate(Connection* conn, Game* game) {
       cJSON* state = gamestate_serialize_for_player(game, conn->player_id);
       Message msg = {
           .type = MSG_GAMESTATE,
           .player_id = -1,  // server
           .payload = state
       };
       char* json = protocol_serialize(&msg);
       connection_send(conn, json);
       free(json);
       cJSON_Delete(state);
   }
   // }}}
   ```

2. Implement `protocol_send_narrative()`:
   ```c
   // {{{ protocol_send_narrative
   void protocol_send_narrative(Connection* conn, const char* text) {
       cJSON* payload = cJSON_CreateObject();
       cJSON_AddStringToObject(payload, "text", text);
       Message msg = {
           .type = MSG_NARRATIVE,
           .payload = payload
       };
       char* json = protocol_serialize(&msg);
       connection_send(conn, json);
       free(json);
       cJSON_Delete(payload);
   }
   // }}}
   ```

3. Implement `protocol_send_error()`:
   ```c
   // {{{ protocol_send_error
   void protocol_send_error(Connection* conn, const char* code, const char* message) {
       cJSON* payload = cJSON_CreateObject();
       cJSON_AddStringToObject(payload, "code", code);
       cJSON_AddStringToObject(payload, "message", message);
       // ... send
   }
   // }}}
   ```

4. Implement `protocol_broadcast_player_joined()`

5. Implement `protocol_broadcast_player_left()`

6. Implement `protocol_request_draw_order()`:
   ```c
   // {{{ protocol_request_draw_order
   void protocol_request_draw_order(Connection* conn, int card_count) {
       cJSON* payload = cJSON_CreateObject();
       cJSON_AddNumberToObject(payload, "count", card_count);
       // ... send MSG_DRAW_ORDER_REQUEST
   }
   // }}}
   ```

7. Implement `protocol_request_choice()` for effect choices

8. Implement broadcast helpers for sending to all connections

9. Write tests for message serialization

## Related Documents
- 2-005a-message-type-definitions.md
- 1-012-gamestate-serialization.md
- 2-008-hidden-information-handling.md

## Dependencies
- 2-005a: Message Type Definitions
- 1-012: Gamestate Serialization
- 2-008: Hidden Information Handling (filtering)

## Acceptance Criteria
- [ ] Gamestate messages serialize correctly
- [ ] Narrative messages include text
- [ ] Error messages include code and description
- [ ] Broadcast functions send to all players
- [ ] Request messages prompt for player input
- [ ] Hidden information filtered before sending
