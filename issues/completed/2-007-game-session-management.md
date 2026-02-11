# 2-007: Game Session Management

## Status
**COMPLETE** - 2026-02-11

## Current Behavior
Session management system implemented in src/net/07-sessions.h/c:
- SessionRegistry tracks multiple concurrent game sessions
- Sessions have WAITING, PLAYING, FINISHED lifecycle states
- Players can create, join, leave, and ready-up in sessions
- Host leaving a WAITING session destroys it
- Spectator support with configurable allowance
- Lobby listing functions for joinable/spectatable sessions
- 23 unit tests passing

## Previous Behavior
No game session management existed.

## Intended Behavior
A session manager that:
- [x] Creates new game sessions
- [x] Allows players to join existing games
- [x] Handles game lifecycle (waiting, playing, finished)
- [x] Supports multiple concurrent games
- [x] Manages spectator access

## Implementation

### Files Created
- `src/net/07-sessions.h` - API definitions and type structures
- `src/net/07-sessions.c` - Full implementation
- `tests/test-sessions.c` - 23 unit tests

### Key Structures

```c
typedef enum {
    SESSION_WAITING,   /* Waiting for players */
    SESSION_PLAYING,   /* Game in progress */
    SESSION_FINISHED   /* Game ended */
} SessionState;

typedef struct {
    int id;
    SessionState state;
    char name[SESSION_NAME_MAX];
    Game* game;                    /* NULL until started */
    int required_players;
    SessionPlayer players[SESSION_MAX_PLAYERS];
    int player_count;
    int host_slot;
    int spectator_conn_ids[SESSION_MAX_SPECTATORS];
    int spectator_count;
    bool allow_spectators;
    bool auto_start;
} GameSession;
```

### API Summary

```c
/* Session lifecycle */
int session_create(SessionRegistry*, int host_conn_id, const char* host_name, int required_players);
void session_destroy(SessionRegistry*, int session_id);
bool session_join(SessionRegistry*, int session_id, int conn_id, const char* name);
bool session_leave(SessionRegistry*, int session_id, int conn_id);
bool session_set_ready(SessionRegistry*, int session_id, int conn_id, bool ready);
bool session_can_start(SessionRegistry*, int session_id);
bool session_start(SessionRegistry*, int session_id);
void session_end(SessionRegistry*, int session_id, int winner);

/* Spectators */
bool session_add_spectator(SessionRegistry*, int session_id, int conn_id);
bool session_remove_spectator(SessionRegistry*, int session_id, int conn_id);

/* Lookup */
GameSession* session_get(SessionRegistry*, int session_id);
GameSession* session_find_by_conn(SessionRegistry*, int conn_id);
int session_get_player_slot(GameSession*, int conn_id);
bool session_is_host(GameSession*, int conn_id);

/* Lobby */
SessionListEntry* session_list_joinable(SessionRegistry*, int* count);
SessionListEntry* session_list_spectatable(SessionRegistry*, int* count);
```

### Design Notes

1. **Session owns Game**: The session creates and owns the Game instance. When the session is destroyed, the Game is freed.

2. **Host authority**: Only the host can start the game. If the host leaves a WAITING session, it's destroyed.

3. **Ready system**: All players must be marked ready before the game can start. This gives players control over when the game begins.

4. **Spectator tracking**: Spectators are tracked separately from players. They can be found via session_find_by_conn.

5. **Integration with Game module**: session_start creates a Game, adds players, and calls game_start. If game_start fails (e.g., missing card types), the session cleans up and remains in WAITING state.

## Related Documents
- docs/04-architecture-c-server.md
- 2-006-connection-manager.md

## Dependencies
- 2-006: Connection Manager (COMPLETE)
- 1-005: Turn Loop / Game struct (COMPLETE)

## Acceptance Criteria
- [x] Sessions can be created
- [x] Players can join waiting sessions
- [x] Game starts when ready
- [x] Session state tracked correctly
- [x] Spectators can watch
