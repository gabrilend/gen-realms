# 2-005b: Client to Server Handlers

## Parent Issue
2-005: Protocol Implementation

## Current Behavior
No handlers for client messages.

## Intended Behavior
Message handlers for all client→server messages:
- Join game
- Game actions (play, buy, attack, etc.)
- Draw order selection
- End turn
- Chat messages

## Suggested Implementation Steps

1. Create handler function type:
   ```c
   // {{{ handler type
   typedef void (*MessageHandler)(Game* game, Connection* conn, Message* msg);
   // }}}
   ```

2. Create dispatch table:
   ```c
   // {{{ client message handlers
   static MessageHandler client_handlers[MSG_TYPE_COUNT] = {
       [MSG_JOIN] = handle_join,
       [MSG_ACTION] = handle_action,
       [MSG_DRAW_ORDER] = handle_draw_order,
       [MSG_END_TURN] = handle_end_turn,
       [MSG_CHAT] = handle_chat,
       [MSG_LEAVE] = handle_leave,
   };
   // }}}
   ```

3. Implement `handle_join()`:
   ```c
   // {{{ handle_join
   static void handle_join(Game* game, Connection* conn, Message* msg) {
       const char* name = cJSON_GetStringValue(
           cJSON_GetObjectItem(msg->payload, "name"));
       if (!name) {
           protocol_send_error(conn, "invalid_join", "Name required");
           return;
       }
       int player_id = game_add_player(game, name, conn);
       if (player_id < 0) {
           protocol_send_error(conn, "game_full", "Game is full");
           return;
       }
       conn->player_id = player_id;
       protocol_broadcast_player_joined(game, player_id, name);
   }
   // }}}
   ```

4. Implement `handle_action()` with action subtype dispatch

5. Implement `handle_draw_order()`:
   ```c
   // {{{ handle_draw_order
   static void handle_draw_order(Game* game, Connection* conn, Message* msg) {
       cJSON* order_array = cJSON_GetObjectItem(msg->payload, "order");
       // Validate and apply draw order
       // ...
   }
   // }}}
   ```

6. Implement `handle_end_turn()`

7. Implement `handle_chat()` for player communication

8. Write tests for each handler

## Related Documents
- 2-005a-message-type-definitions.md
- 2-007-game-session-management.md

## Dependencies
- 2-005a: Message Type Definitions
- 2-006: Connection Manager (Connection struct)
- 1-005: Turn Loop (game state transitions)

## Acceptance Criteria
- [ ] Join handler adds player to game
- [ ] Action handler dispatches to correct action
- [ ] Draw order handler validates and applies order
- [ ] End turn handler advances game state
- [ ] All handlers send appropriate responses
- [ ] Invalid messages return errors
