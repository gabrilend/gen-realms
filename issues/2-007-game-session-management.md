# 2-007: Game Session Management

## Current Behavior
No game session management exists.

## Intended Behavior
A session manager that:
- Creates new game sessions
- Allows players to join existing games
- Handles game lifecycle (waiting, playing, finished)
- Supports multiple concurrent games
- Manages spectator access

## Suggested Implementation Steps

1. Create `src/net/07-sessions.h` with type definitions
2. Create `src/net/07-sessions.c` with implementation
3. Define session structure:
   ```c
   typedef enum {
       SESSION_WAITING,
       SESSION_PLAYING,
       SESSION_FINISHED
   } SessionState;

   typedef struct {
       int session_id;
       Game* game;
       SessionState state;
       int host_conn_id;
       int player_conn_ids[4];
       int spectator_conn_ids[10];
   } GameSession;
   ```
4. Implement `GameSession* session_create(int host_conn_id, const char* host_name)`
5. Implement `bool session_join(int session_id, int conn_id, const char* name)`
6. Implement `void session_start(int session_id)`
7. Implement `void session_end(int session_id, int winner)`
8. Implement `void session_add_spectator(int session_id, int conn_id)`
9. Implement session listing for lobby
10. Write tests for session lifecycle

## Related Documents
- docs/04-architecture-c-server.md
- 2-006-connection-manager.md

## Dependencies
- 2-006: Connection Manager
- 1-005: Turn Loop (Game struct)

## Acceptance Criteria
- [ ] Sessions can be created
- [ ] Players can join waiting sessions
- [ ] Game starts when ready
- [ ] Session state tracked correctly
- [ ] Spectators can watch
