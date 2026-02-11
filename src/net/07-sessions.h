/* 07-sessions.h - Game Session Management
 *
 * Manages game sessions including creation, player joining, game lifecycle,
 * and spectator access. Each session wraps a Game instance and tracks
 * connection IDs for all participants.
 *
 * Session lifecycle:
 *   WAITING -> PLAYING -> FINISHED
 *
 * Dependencies: 05-game, 06-connections
 */

#ifndef SYMBELINE_SESSIONS_H
#define SYMBELINE_SESSIONS_H

#include <stdbool.h>
#include "../core/05-game.h"

/* ========================================================================== */
/*                              Constants                                      */
/* ========================================================================== */

/* {{{ Constants */
#define SESSION_MAX_SESSIONS 32
#define SESSION_MAX_PLAYERS 4      /* Matches MAX_PLAYERS in game.h */
#define SESSION_MAX_SPECTATORS 16
#define SESSION_NAME_MAX 64
#define SESSION_INVALID_ID -1
/* }}} */

/* ========================================================================== */
/*                              Type Definitions                               */
/* ========================================================================== */

/* {{{ SessionState
 * Lifecycle states for a game session.
 */
typedef enum {
    SESSION_WAITING,     /* Waiting for players to join */
    SESSION_PLAYING,     /* Game is in progress */
    SESSION_FINISHED     /* Game has ended */
} SessionState;
/* }}} */

/* {{{ SessionPlayer
 * Information about a player in the session.
 */
typedef struct {
    int conn_id;                    /* Connection ID (-1 if empty slot) */
    char name[SESSION_NAME_MAX];    /* Player display name */
    bool ready;                     /* Ready to start (while WAITING) */
    int player_index;               /* Index in Game->players array */
} SessionPlayer;
/* }}} */

/* {{{ GameSession
 * A game session containing a game and connection tracking.
 */
typedef struct {
    int id;                         /* Unique session ID */
    SessionState state;             /* Current session state */
    char name[SESSION_NAME_MAX];    /* Session name (e.g., "Player1's game") */

    /* Game instance */
    Game* game;                     /* NULL until game starts */
    int required_players;           /* How many players needed to start */

    /* Players */
    SessionPlayer players[SESSION_MAX_PLAYERS];
    int player_count;               /* Current number of joined players */
    int host_slot;                  /* Which player slot is the host */

    /* Spectators */
    int spectator_conn_ids[SESSION_MAX_SPECTATORS];
    int spectator_count;

    /* Settings */
    bool allow_spectators;          /* Can spectators join? */
    bool auto_start;                /* Start when enough players? */
} GameSession;
/* }}} */

/* {{{ SessionRegistry
 * Central registry for all game sessions.
 */
typedef struct {
    GameSession sessions[SESSION_MAX_SESSIONS];
    int session_count;              /* Number of active sessions */
    int next_id;                    /* Next session ID to assign */
} SessionRegistry;
/* }}} */

/* {{{ SessionListEntry
 * Information about a session for lobby listing.
 */
typedef struct {
    int id;
    char name[SESSION_NAME_MAX];
    SessionState state;
    int player_count;
    int required_players;
    bool allow_spectators;
} SessionListEntry;
/* }}} */

/* ========================================================================== */
/*                              Registry Management                            */
/* ========================================================================== */

/* {{{ session_registry_create
 * Creates a new session registry.
 * Returns NULL on allocation failure.
 */
SessionRegistry* session_registry_create(void);
/* }}} */

/* {{{ session_registry_destroy
 * Frees the session registry and all contained sessions.
 * Safe to call with NULL.
 */
void session_registry_destroy(SessionRegistry* registry);
/* }}} */

/* ========================================================================== */
/*                              Session Lifecycle                              */
/* ========================================================================== */

/* {{{ session_create
 * Creates a new game session with the given host.
 * Returns the session ID, or SESSION_INVALID_ID on failure.
 *
 * Parameters:
 *   registry - The session registry
 *   host_conn_id - Connection ID of the session host
 *   host_name - Display name of the host
 *   required_players - Number of players needed (2-4)
 */
int session_create(SessionRegistry* registry, int host_conn_id,
                   const char* host_name, int required_players);
/* }}} */

/* {{{ session_destroy
 * Removes a session from the registry and frees its resources.
 * Does NOT disconnect players - caller must handle that.
 */
void session_destroy(SessionRegistry* registry, int session_id);
/* }}} */

/* {{{ session_join
 * Adds a player to a waiting session.
 * Returns true on success, false if session is full or not waiting.
 */
bool session_join(SessionRegistry* registry, int session_id,
                  int conn_id, const char* name);
/* }}} */

/* {{{ session_leave
 * Removes a player from a session.
 * If the host leaves a WAITING session, the session is destroyed.
 * Returns true if player was found and removed.
 */
bool session_leave(SessionRegistry* registry, int session_id, int conn_id);
/* }}} */

/* {{{ session_set_ready
 * Sets a player's ready status in a waiting session.
 * Returns true on success.
 */
bool session_set_ready(SessionRegistry* registry, int session_id,
                       int conn_id, bool ready);
/* }}} */

/* {{{ session_can_start
 * Checks if a session has enough ready players to start.
 */
bool session_can_start(SessionRegistry* registry, int session_id);
/* }}} */

/* {{{ session_start
 * Starts the game for a session.
 * Creates the Game instance and transitions to PLAYING state.
 * Returns true on success, false if cannot start.
 */
bool session_start(SessionRegistry* registry, int session_id);
/* }}} */

/* {{{ session_end
 * Ends the game session, recording the winner.
 * Transitions to FINISHED state.
 */
void session_end(SessionRegistry* registry, int session_id, int winner_player_index);
/* }}} */

/* ========================================================================== */
/*                              Spectators                                     */
/* ========================================================================== */

/* {{{ session_add_spectator
 * Adds a spectator to watch the session.
 * Returns true on success, false if spectators not allowed or full.
 */
bool session_add_spectator(SessionRegistry* registry, int session_id, int conn_id);
/* }}} */

/* {{{ session_remove_spectator
 * Removes a spectator from the session.
 * Returns true if spectator was found and removed.
 */
bool session_remove_spectator(SessionRegistry* registry, int session_id, int conn_id);
/* }}} */

/* ========================================================================== */
/*                              Session Lookup                                 */
/* ========================================================================== */

/* {{{ session_get
 * Retrieves a session by its ID.
 * Returns NULL if not found.
 */
GameSession* session_get(SessionRegistry* registry, int session_id);
/* }}} */

/* {{{ session_find_by_conn
 * Finds the session containing a given connection (as player or spectator).
 * Returns NULL if connection is not in any session.
 */
GameSession* session_find_by_conn(SessionRegistry* registry, int conn_id);
/* }}} */

/* {{{ session_get_player_slot
 * Gets the player slot index for a connection in a session.
 * Returns -1 if not found.
 */
int session_get_player_slot(GameSession* session, int conn_id);
/* }}} */

/* {{{ session_is_host
 * Checks if a connection is the host of a session.
 */
bool session_is_host(GameSession* session, int conn_id);
/* }}} */

/* ========================================================================== */
/*                              Lobby Listing                                  */
/* ========================================================================== */

/* {{{ session_list_joinable
 * Returns a list of sessions that can be joined (WAITING state).
 * Caller must free the returned array.
 * Returns NULL if no joinable sessions or allocation failure.
 * Sets count to the number of entries in the returned array.
 */
SessionListEntry* session_list_joinable(SessionRegistry* registry, int* count);
/* }}} */

/* {{{ session_list_spectatable
 * Returns a list of sessions that can be spectated.
 * Caller must free the returned array.
 * Returns NULL if no spectatable sessions or allocation failure.
 * Sets count to the number of entries in the returned array.
 */
SessionListEntry* session_list_spectatable(SessionRegistry* registry, int* count);
/* }}} */

/* ========================================================================== */
/*                              Statistics                                     */
/* ========================================================================== */

/* {{{ session_count
 * Returns the total number of active sessions.
 */
int session_count(const SessionRegistry* registry);
/* }}} */

/* {{{ session_count_by_state
 * Returns the number of sessions in a specific state.
 */
int session_count_by_state(const SessionRegistry* registry, SessionState state);
/* }}} */

/* ========================================================================== */
/*                              Utility                                        */
/* ========================================================================== */

/* {{{ session_state_to_string
 * Returns a string representation of a session state.
 */
const char* session_state_to_string(SessionState state);
/* }}} */

#endif /* SYMBELINE_SESSIONS_H */
