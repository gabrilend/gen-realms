# 2-006: Connection Manager

## Current Behavior
No connection tracking exists.

## Intended Behavior
A connection manager that:
- Tracks all connected clients (WebSocket and SSH)
- Maps connections to players and games
- Handles join/leave events
- Broadcasts to game participants
- Manages spectator connections

## Suggested Implementation Steps

1. Create `src/net/06-connections.h` with type definitions
2. Create `src/net/06-connections.c` with implementation
3. Define connection registry:
   ```c
   typedef struct {
       void* connections[MAX_CONNECTIONS];
       ConnectionType types[MAX_CONNECTIONS];
       int count;
   } ConnectionRegistry;
   ```
4. Implement `int conn_register(void* conn, ConnectionType type)`
5. Implement `void conn_unregister(int conn_id)`
6. Implement `void conn_assign_player(int conn_id, int player_id, int game_id)`
7. Implement `void conn_broadcast_game(int game_id, const char* msg)`
8. Implement `void conn_send(int conn_id, const char* msg)`
9. Handle both WebSocket and SSH transparently
10. Write tests for connection lifecycle

## Related Documents
- docs/04-architecture-c-server.md
- 2-003-websocket-handler.md
- 2-004-ssh-server-integration.md

## Dependencies
- 2-003: WebSocket Handler
- 2-004: SSH Server Integration

## Acceptance Criteria
- [ ] Connections register on connect
- [ ] Connections unregister on disconnect
- [ ] Player assignment works
- [ ] Game broadcast reaches all participants
- [ ] Mixed WebSocket/SSH games work
