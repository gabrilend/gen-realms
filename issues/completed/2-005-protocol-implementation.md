# 2-005: Protocol Implementation

## Status
**COMPLETE** - 2026-02-11

## Current Behavior
Protocol layer fully implemented with:
- 14 message types (6 client→server, 8 server→client)
- 21 error codes covering all failure modes
- Dispatch table routing to handlers
- JSON serialization with cJSON
- 129 tests passing

## Files Created
- `src/net/04-protocol.h` - Protocol definitions and API
- `src/net/04-protocol.c` - Full implementation (~500 lines)
- `tests/test-protocol.c` - 129 comprehensive tests

## Previous Behavior
No unified protocol for client-server communication.

## Intended Behavior
A well-defined protocol layer that:
- Defines all message types
- Validates incoming messages
- Routes messages to appropriate handlers
- Abstracts transport (WebSocket vs SSH)
- Provides consistent error handling

## Sub-Issues

This issue has been split into the following sub-issues:

| ID | Description | Status |
|----|-------------|--------|
| 2-005a | Message Type Definitions | COMPLETE |
| 2-005b | Client to Server Handlers | COMPLETE |
| 2-005c | Server to Client Generation | COMPLETE |
| 2-005d | Validation and Error Handling | COMPLETE |

## Implementation Order

1. **2-005a** first - defines types everything depends on
2. **2-005d** second - validation needed before handlers
3. **2-005b** third - client message handling
4. **2-005c** last - server responses

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
- [x] All message types defined (14 types in MessageType enum)
- [x] Parsing handles malformed JSON (PROTOCOL_ERROR_MALFORMED_JSON)
- [x] Validation catches invalid messages (validation helpers)
- [x] Dispatch routes to correct handler (dispatch table pattern)
- [x] Errors include helpful messages (21 error codes with messages)
