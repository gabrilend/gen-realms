# 2-003: WebSocket Handler

## Status
**COMPLETE** - 2026-02-11

## Current Behavior
WebSocket handler fully implemented in `src/net/05-websocket.c/h`:
- WSContext for connection pool management (64 max connections)
- WSConnection per-connection state with player/game association
- Protocol callback handling ESTABLISHED, RECEIVE, WRITEABLE, CLOSED
- Integration with 04-protocol for message parsing/dispatch
- HTTP server modified to include WebSocket protocol

## Files Created
- `src/net/05-websocket.h` - WebSocket API and structures
- `src/net/05-websocket.c` - Full implementation (~450 lines)
- `tests/test-websocket.c` - Unit tests (requires libwebsockets)

## Previous Behavior
No WebSocket capability existed.

## Intended Behavior
A WebSocket handler using libwebsockets that:
- Upgrades HTTP connections to WebSocket
- Handles bidirectional game messages
- Serializes/deserializes JSON protocol
- Manages per-connection state
- Broadcasts updates to connected clients

## Suggested Implementation Steps

1. Create `src/net/03-websocket.h` with type definitions
2. Create `src/net/03-websocket.c` with implementation
3. Define connection state:
   ```c
   typedef struct {
       struct lws* wsi;
       int player_id;
       int game_id;
       bool authenticated;
   } WSConnection;
   ```
4. Implement WebSocket protocol callback:
   ```c
   static int callback_game(struct lws *wsi,
       enum lws_callback_reasons reason,
       void *user, void *in, size_t len);
   ```
5. Handle LWS_CALLBACK_ESTABLISHED (new connection)
6. Handle LWS_CALLBACK_RECEIVE (incoming message)
7. Handle LWS_CALLBACK_CLOSED (disconnect)
8. Implement `void ws_send(WSConnection* conn, const char* json)`
9. Implement `void ws_broadcast(Game* game, const char* json)`
10. Write tests with WebSocket client

## Related Documents
- docs/04-architecture-c-server.md
- 2-002-http-server.md

## Dependencies
- 2-002: HTTP Server (same libwebsockets context)
- 1-012: Gamestate Serialization

## Message Types

```json
// Server -> Client
{"type": "gamestate", ...}
{"type": "narrative", "text": "..."}
{"type": "error", "message": "..."}

// Client -> Server
{"type": "action", "action": "play_card", ...}
{"type": "action", "action": "set_draw_order", ...}
{"type": "join", "name": "PlayerName"}
```

## Acceptance Criteria
- [x] WebSocket connections establish (callback_game handles LWS_CALLBACK_ESTABLISHED)
- [x] JSON messages parse correctly (ws_handle_message uses protocol_parse)
- [x] Actions dispatch to game engine (protocol_dispatch routes to handlers)
- [x] Gamestate broadcasts to all players (ws_broadcast_gamestate)
- [x] Disconnections handled gracefully (LWS_CALLBACK_CLOSED with player_left notification)

## Build Notes
Requires `libwebsockets-devel` package. Install with: `xbps-install libwebsockets-devel`
Tests can be run with `make test-websocket` once library is installed.
