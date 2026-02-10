# 2-003: WebSocket Handler

## Current Behavior
No WebSocket capability exists.

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
- [ ] WebSocket connections establish
- [ ] JSON messages parse correctly
- [ ] Actions dispatch to game engine
- [ ] Gamestate broadcasts to all players
- [ ] Disconnections handled gracefully
