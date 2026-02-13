# 3-011g: WASM WebSocket Communication

## Current Behavior

WebSocket connection managed by JavaScript (`symbeline.js`). Handles connection
lifecycle, message sending/receiving, and reconnection logic.

## Intended Behavior

WebSocket communication handled in C using Emscripten's WebSocket API via
EM_ASM. Connection management, message handling, and ping/pong all in C.

## Dependencies

- 3-011a: Core Canvas Infrastructure (provides update loop)

## Suggested Implementation Steps

1. Create `src/client/wasm/websocket.h` with:
   - WebSocketState enum (disconnected, connecting, connected, error, closing)
   - MessageType enum (game_state, player_action, narrative, error, chat, ping, pong)
   - WebSocketMessage structure with parsed fields
   - Callback types for connect, disconnect, message, error

2. Create `src/client/wasm/websocket.c` implementing:
   - `ws_init()` - Initialize with callbacks
   - `ws_connect()` - Open WebSocket via EM_ASM
   - `ws_disconnect()` - Close connection
   - `ws_send()` / `ws_send_json()` - Send raw/JSON messages
   - `ws_send_action()` - Send player action
   - `ws_send_chat()` - Send chat message
   - `ws_request_state()` - Request full game state
   - `ws_update()` - Process events, handle ping/pong
   - `ws_get_latency()` - RTT measurement

## Files Created

- `src/client/wasm/websocket.h` - WebSocket API
- `src/client/wasm/websocket.c` - WebSocket implementation

## JS Portions Replaced

- WebSocket handling in `assets/web/symbeline.js`

## Acceptance Criteria

- [x] WebSocket connects via EM_ASM
- [x] Connection lifecycle callbacks work
- [x] Messages sent and received correctly
- [x] Message type parsing
- [x] Ping/pong for latency measurement
- [x] Ping timeout detection
- [x] State exposed (connected, disconnected, etc.)

## Implementation Notes

Uses EM_ASM for WebSocket creation since Emscripten's built-in websocket.js
would be an external JS dependency.

WebSocket created in Module context for persistence:
```c
EM_ASM({
    Module.ws = new WebSocket(url);
    Module.ws.onmessage = function(e) { ... };
});
```

Callbacks use EMSCRIPTEN_KEEPALIVE to export C functions callable from JS.
