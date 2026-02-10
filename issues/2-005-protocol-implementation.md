# 2-005: Protocol Implementation

## Current Behavior
No unified protocol for client-server communication.

## Intended Behavior
A well-defined protocol layer that:
- Defines all message types
- Validates incoming messages
- Routes messages to appropriate handlers
- Abstracts transport (WebSocket vs SSH)
- Provides consistent error handling

## Suggested Implementation Steps

1. Create `src/net/05-protocol.h` with message definitions
2. Create `src/net/05-protocol.c` with implementation
3. Define message types:
   ```c
   typedef enum {
       MSG_JOIN,
       MSG_LEAVE,
       MSG_ACTION,
       MSG_DRAW_ORDER,
       MSG_CHAT,
       MSG_GAMESTATE,
       MSG_NARRATIVE,
       MSG_ERROR
   } MessageType;

   typedef struct {
       MessageType type;
       int player_id;
       cJSON* payload;
   } Message;
   ```
4. Implement `Message* protocol_parse(const char* json)`
5. Implement `char* protocol_serialize(Message* msg)`
6. Implement `void protocol_free(Message* msg)`
7. Implement `bool protocol_validate(Message* msg)`
8. Create dispatch table for message handlers
9. Implement `void protocol_dispatch(Message* msg, Game* game)`
10. Write tests for all message types

## Related Documents
- docs/04-architecture-c-server.md
- 2-003-websocket-handler.md

## Dependencies
- 1-012: Gamestate Serialization
- cJSON library

## Protocol Specification

### Client → Server

```json
{"type": "join", "name": "PlayerName"}
{"type": "action", "action": "play_card", "card_instance_id": "abc123"}
{"type": "action", "action": "buy_card", "slot": 2}
{"type": "action", "action": "attack", "target": "player"}
{"type": "action", "action": "attack", "target": "base", "base_id": "xyz"}
{"type": "draw_order", "order": [3, 1, 5, 2, 4]}
{"type": "end_turn"}
```

### Server → Client

```json
{"type": "gamestate", ...}  // Full gamestate
{"type": "narrative", "text": "The dire bear charges..."}
{"type": "error", "code": "invalid_action", "message": "..."}
{"type": "player_joined", "player_id": 1, "name": "..."}
{"type": "player_left", "player_id": 1}
```

## Acceptance Criteria
- [ ] All message types defined
- [ ] Parsing handles malformed JSON
- [ ] Validation catches invalid messages
- [ ] Dispatch routes to correct handler
- [ ] Errors include helpful messages
