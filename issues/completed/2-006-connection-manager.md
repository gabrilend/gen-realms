# 2-006: Connection Manager

## Status
**COMPLETE** - 2026-02-11

## Current Behavior
Unified connection manager implemented in src/net/06-connections.h/c:
- ConnectionRegistry tracks both WebSocket and SSH connections
- Auto-incrementing connection IDs with lookup by ID, player, or handle
- Player and game assignment for routing messages
- Unified send interface (conn_send, conn_send_to_player, conn_broadcast_game)
- 19 unit tests passing

## Previous Behavior
No connection tracking existed.

## Intended Behavior
A connection manager that:
- [x] Tracks all connected clients (WebSocket and SSH)
- [x] Maps connections to players and games
- [x] Handles join/leave events
- [x] Broadcasts to game participants
- [x] Manages spectator connections (via game_id assignment)

## Implementation

### Files Created
- `src/net/06-connections.h` - API definitions and type structures
- `src/net/06-connections.c` - Implementation with extern function declarations
- `tests/test-connections.c` - 19 unit tests with stub implementations

### Key Design Decisions

1. **Forward declarations for testability**: The connection manager uses forward declarations for WSConnection and SSHConnection types, with extern function declarations for ws_send and ssh_connection_send_string. This allows unit testing with stubs without linking actual WebSocket/SSH code.

2. **Unified Connection type**: A single Connection struct wraps both transport types using a union, allowing transport-agnostic game logic.

3. **Auto-incrementing IDs**: Connection IDs are never reused within a session to prevent stale reference bugs.

### API Summary

```c
/* Registry lifecycle */
ConnectionRegistry* conn_registry_create(void);
void conn_registry_destroy(ConnectionRegistry* registry);

/* Registration */
int conn_register_ws(ConnectionRegistry* registry, WSConnection* ws);
int conn_register_ssh(ConnectionRegistry* registry, SSHConnection* ssh);
void conn_unregister(ConnectionRegistry* registry, int conn_id);

/* Lookup */
Connection* conn_get(ConnectionRegistry* registry, int conn_id);
Connection* conn_find_by_player(ConnectionRegistry* registry, int player_id);
Connection* conn_find_by_ws(ConnectionRegistry* registry, WSConnection* ws);
Connection* conn_find_by_ssh(ConnectionRegistry* registry, SSHConnection* ssh);

/* Player assignment */
bool conn_assign_player(ConnectionRegistry* registry, int conn_id,
                        int player_id, int game_id);
bool conn_clear_player(ConnectionRegistry* registry, int conn_id);

/* Message sending */
bool conn_send(ConnectionRegistry* registry, int conn_id, const char* message);
bool conn_send_to_player(ConnectionRegistry* registry, int player_id,
                         const char* message);
int conn_broadcast_game(ConnectionRegistry* registry, int game_id,
                        const char* message, int exclude_player);

/* Statistics */
int conn_count(const ConnectionRegistry* registry);
int conn_count_in_game(const ConnectionRegistry* registry, int game_id);
int conn_count_by_type(const ConnectionRegistry* registry, ConnectionType type);
```

## Related Documents
- docs/04-architecture-c-server.md
- 2-003-websocket-handler.md
- 2-004-ssh-server-integration.md

## Dependencies
- 2-003: WebSocket Handler (COMPLETE)
- 2-004: SSH Server Integration (COMPLETE)

## Acceptance Criteria
- [x] Connections register on connect
- [x] Connections unregister on disconnect
- [x] Player assignment works
- [x] Game broadcast reaches all participants
- [x] Mixed WebSocket/SSH games work
