# 2-005a: Message Type Definitions

## Parent Issue
2-005: Protocol Implementation

## Current Behavior
No message types defined for client-server communication.

## Intended Behavior
Complete message type definitions that:
- Define all message type enums
- Define Message struct with type, player_id, payload
- Provide JSON schema for each message type
- Support both client→server and server→client messages

## Suggested Implementation Steps

1. Create `src/server/05-protocol.h` with enums:
   ```c
   // {{{ MessageType enum
   typedef enum {
       // Client → Server
       MSG_JOIN,
       MSG_LEAVE,
       MSG_ACTION,
       MSG_DRAW_ORDER,
       MSG_CHAT,
       MSG_END_TURN,

       // Server → Client
       MSG_GAMESTATE,
       MSG_NARRATIVE,
       MSG_ERROR,
       MSG_PLAYER_JOINED,
       MSG_PLAYER_LEFT,
       MSG_DRAW_ORDER_REQUEST,
       MSG_CHOICE_REQUEST,

       MSG_TYPE_COUNT
   } MessageType;
   // }}}
   ```

2. Define Message struct:
   ```c
   // {{{ Message struct
   typedef struct {
       MessageType type;
       int player_id;
       cJSON* payload;
       uint64_t timestamp;
   } Message;
   // }}}
   ```

3. Define action subtypes:
   ```c
   // {{{ ActionType enum
   typedef enum {
       ACTION_PLAY_CARD,
       ACTION_BUY_CARD,
       ACTION_ATTACK_PLAYER,
       ACTION_ATTACK_BASE,
       ACTION_SCRAP_CARD,
       ACTION_USE_ABILITY,
       ACTION_COUNT
   } ActionType;
   // }}}
   ```

4. Create JSON schema documentation in comments

5. Implement `const char* message_type_name(MessageType type)` for debugging

6. Write tests validating enum coverage

## Related Documents
- docs/04-architecture-c-server.md
- 2-005-protocol-implementation.md (parent)

## Dependencies
- cJSON library

## JSON Schema

### Client → Server Messages
```json
// MSG_JOIN
{"type": "join", "name": "PlayerName"}

// MSG_ACTION
{"type": "action", "action": "play_card", "card_instance_id": "abc123"}
{"type": "action", "action": "buy_card", "slot": 2}
{"type": "action", "action": "attack", "target": "player"}
{"type": "action", "action": "attack", "target": "base", "base_id": "xyz"}

// MSG_DRAW_ORDER
{"type": "draw_order", "order": [3, 1, 5, 2, 4]}

// MSG_END_TURN
{"type": "end_turn"}
```

### Server → Client Messages
```json
// MSG_GAMESTATE
{"type": "gamestate", "turn": 12, "phase": "main", ...}

// MSG_NARRATIVE
{"type": "narrative", "text": "The dire bear charges..."}

// MSG_ERROR
{"type": "error", "code": "invalid_action", "message": "..."}
```

## Acceptance Criteria
- [ ] All message types enumerated
- [ ] Message struct defined with all fields
- [ ] Action subtypes defined
- [ ] JSON schema documented
- [ ] Type name function works for debugging
