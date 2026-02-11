/* 07-sessions.c - Game Session Management Implementation
 *
 * Manages game sessions including creation, player joining, game lifecycle,
 * and spectator access. Integrates with the Game struct from core/05-game.
 */

#include "07-sessions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================== */
/*                              Registry Management                            */
/* ========================================================================== */

/* {{{ session_registry_create */
SessionRegistry* session_registry_create(void) {
    SessionRegistry* registry = malloc(sizeof(SessionRegistry));
    if (registry == NULL) {
        return NULL;
    }

    /* Initialize all sessions as inactive */
    for (int i = 0; i < SESSION_MAX_SESSIONS; i++) {
        registry->sessions[i].id = SESSION_INVALID_ID;
        registry->sessions[i].game = NULL;
    }

    registry->session_count = 0;
    registry->next_id = 0;

    return registry;
}
/* }}} */

/* {{{ session_registry_destroy */
void session_registry_destroy(SessionRegistry* registry) {
    if (registry == NULL) {
        return;
    }

    /* Free all active sessions */
    for (int i = 0; i < SESSION_MAX_SESSIONS; i++) {
        if (registry->sessions[i].id != SESSION_INVALID_ID) {
            if (registry->sessions[i].game != NULL) {
                game_free(registry->sessions[i].game);
            }
        }
    }

    free(registry);
}
/* }}} */

/* ========================================================================== */
/*                              Internal Helpers                               */
/* ========================================================================== */

/* {{{ find_empty_slot
 * Finds an empty slot in the session array.
 * Returns -1 if no slots available.
 */
static int find_empty_slot(SessionRegistry* registry) {
    for (int i = 0; i < SESSION_MAX_SESSIONS; i++) {
        if (registry->sessions[i].id == SESSION_INVALID_ID) {
            return i;
        }
    }
    return -1;
}
/* }}} */

/* {{{ init_session
 * Initializes a session structure.
 */
static void init_session(GameSession* session, int id, int host_conn_id,
                         const char* host_name, int required_players) {
    session->id = id;
    session->state = SESSION_WAITING;
    snprintf(session->name, SESSION_NAME_MAX, "%s's game", host_name);

    session->game = NULL;
    session->required_players = required_players;

    /* Initialize player slots */
    for (int i = 0; i < SESSION_MAX_PLAYERS; i++) {
        session->players[i].conn_id = -1;
        session->players[i].name[0] = '\0';
        session->players[i].ready = false;
        session->players[i].player_index = -1;
    }

    /* Add host as first player */
    session->players[0].conn_id = host_conn_id;
    strncpy(session->players[0].name, host_name, SESSION_NAME_MAX - 1);
    session->players[0].name[SESSION_NAME_MAX - 1] = '\0';
    session->players[0].ready = false;  /* Host must explicitly ready up */
    session->player_count = 1;
    session->host_slot = 0;

    /* Initialize spectators */
    for (int i = 0; i < SESSION_MAX_SPECTATORS; i++) {
        session->spectator_conn_ids[i] = -1;
    }
    session->spectator_count = 0;

    /* Default settings */
    session->allow_spectators = true;
    session->auto_start = false;
}
/* }}} */

/* {{{ find_player_slot
 * Finds an empty player slot in a session.
 * Returns -1 if no slots available.
 */
static int find_player_slot(GameSession* session) {
    for (int i = 0; i < SESSION_MAX_PLAYERS; i++) {
        if (session->players[i].conn_id == -1) {
            return i;
        }
    }
    return -1;
}
/* }}} */

/* {{{ find_spectator_slot
 * Finds an empty spectator slot in a session.
 * Returns -1 if no slots available.
 */
static int find_spectator_slot(GameSession* session) {
    for (int i = 0; i < SESSION_MAX_SPECTATORS; i++) {
        if (session->spectator_conn_ids[i] == -1) {
            return i;
        }
    }
    return -1;
}
/* }}} */

/* ========================================================================== */
/*                              Session Lifecycle                              */
/* ========================================================================== */

/* {{{ session_create */
int session_create(SessionRegistry* registry, int host_conn_id,
                   const char* host_name, int required_players) {
    if (registry == NULL || host_name == NULL) {
        return SESSION_INVALID_ID;
    }

    if (required_players < 2 || required_players > SESSION_MAX_PLAYERS) {
        fprintf(stderr, "SessionRegistry: Invalid player count %d (must be 2-%d)\n",
                required_players, SESSION_MAX_PLAYERS);
        return SESSION_INVALID_ID;
    }

    int slot = find_empty_slot(registry);
    if (slot < 0) {
        fprintf(stderr, "SessionRegistry: No available session slots\n");
        return SESSION_INVALID_ID;
    }

    int session_id = registry->next_id++;
    init_session(&registry->sessions[slot], session_id, host_conn_id,
                 host_name, required_players);

    registry->session_count++;

    printf("SessionRegistry: Created session %d '%s' (host: %s, players: %d)\n",
           session_id, registry->sessions[slot].name, host_name, required_players);

    return session_id;
}
/* }}} */

/* {{{ session_destroy */
void session_destroy(SessionRegistry* registry, int session_id) {
    if (registry == NULL) {
        return;
    }

    for (int i = 0; i < SESSION_MAX_SESSIONS; i++) {
        if (registry->sessions[i].id == session_id) {
            printf("SessionRegistry: Destroying session %d\n", session_id);

            if (registry->sessions[i].game != NULL) {
                game_free(registry->sessions[i].game);
            }

            registry->sessions[i].id = SESSION_INVALID_ID;
            registry->sessions[i].game = NULL;
            registry->session_count--;
            return;
        }
    }

    fprintf(stderr, "SessionRegistry: Session %d not found for destroy\n", session_id);
}
/* }}} */

/* {{{ session_join */
bool session_join(SessionRegistry* registry, int session_id,
                  int conn_id, const char* name) {
    if (registry == NULL || name == NULL) {
        return false;
    }

    GameSession* session = session_get(registry, session_id);
    if (session == NULL) {
        return false;
    }

    if (session->state != SESSION_WAITING) {
        fprintf(stderr, "SessionRegistry: Cannot join session %d - not waiting\n",
                session_id);
        return false;
    }

    if (session->player_count >= session->required_players) {
        fprintf(stderr, "SessionRegistry: Session %d is full\n", session_id);
        return false;
    }

    int slot = find_player_slot(session);
    if (slot < 0) {
        return false;
    }

    session->players[slot].conn_id = conn_id;
    strncpy(session->players[slot].name, name, SESSION_NAME_MAX - 1);
    session->players[slot].name[SESSION_NAME_MAX - 1] = '\0';
    session->players[slot].ready = false;
    session->player_count++;

    printf("SessionRegistry: Player '%s' joined session %d (slot %d, total: %d)\n",
           name, session_id, slot, session->player_count);

    return true;
}
/* }}} */

/* {{{ session_leave */
bool session_leave(SessionRegistry* registry, int session_id, int conn_id) {
    if (registry == NULL) {
        return false;
    }

    GameSession* session = session_get(registry, session_id);
    if (session == NULL) {
        return false;
    }

    /* Check if it's a spectator */
    for (int i = 0; i < SESSION_MAX_SPECTATORS; i++) {
        if (session->spectator_conn_ids[i] == conn_id) {
            session->spectator_conn_ids[i] = -1;
            session->spectator_count--;
            printf("SessionRegistry: Spectator left session %d\n", session_id);
            return true;
        }
    }

    /* Check if it's a player */
    int player_slot = session_get_player_slot(session, conn_id);
    if (player_slot < 0) {
        return false;
    }

    bool was_host = (player_slot == session->host_slot);

    printf("SessionRegistry: Player '%s' left session %d\n",
           session->players[player_slot].name, session_id);

    /* Clear the player slot */
    session->players[player_slot].conn_id = -1;
    session->players[player_slot].name[0] = '\0';
    session->players[player_slot].ready = false;
    session->player_count--;

    /* If host left while waiting, destroy the session */
    if (was_host && session->state == SESSION_WAITING) {
        printf("SessionRegistry: Host left waiting session %d, destroying\n", session_id);
        session_destroy(registry, session_id);
    }

    return true;
}
/* }}} */

/* {{{ session_set_ready */
bool session_set_ready(SessionRegistry* registry, int session_id,
                       int conn_id, bool ready) {
    if (registry == NULL) {
        return false;
    }

    GameSession* session = session_get(registry, session_id);
    if (session == NULL || session->state != SESSION_WAITING) {
        return false;
    }

    int slot = session_get_player_slot(session, conn_id);
    if (slot < 0) {
        return false;
    }

    session->players[slot].ready = ready;

    printf("SessionRegistry: Player '%s' in session %d is %sready\n",
           session->players[slot].name, session_id, ready ? "" : "not ");

    /* Check for auto-start */
    if (session->auto_start && session_can_start(registry, session_id)) {
        session_start(registry, session_id);
    }

    return true;
}
/* }}} */

/* {{{ session_can_start */
bool session_can_start(SessionRegistry* registry, int session_id) {
    if (registry == NULL) {
        return false;
    }

    GameSession* session = session_get(registry, session_id);
    if (session == NULL || session->state != SESSION_WAITING) {
        return false;
    }

    if (session->player_count < session->required_players) {
        return false;
    }

    /* Check all players are ready */
    int ready_count = 0;
    for (int i = 0; i < SESSION_MAX_PLAYERS; i++) {
        if (session->players[i].conn_id != -1) {
            if (!session->players[i].ready) {
                return false;
            }
            ready_count++;
        }
    }

    return ready_count >= session->required_players;
}
/* }}} */

/* {{{ session_start */
bool session_start(SessionRegistry* registry, int session_id) {
    if (registry == NULL) {
        return false;
    }

    GameSession* session = session_get(registry, session_id);
    if (session == NULL) {
        return false;
    }

    if (session->state != SESSION_WAITING) {
        fprintf(stderr, "SessionRegistry: Cannot start session %d - not waiting\n",
                session_id);
        return false;
    }

    if (!session_can_start(registry, session_id)) {
        fprintf(stderr, "SessionRegistry: Cannot start session %d - not ready\n",
                session_id);
        return false;
    }

    /* Create the game */
    session->game = game_create(session->player_count);
    if (session->game == NULL) {
        fprintf(stderr, "SessionRegistry: Failed to create game for session %d\n",
                session_id);
        return false;
    }

    /* Add players to the game */
    int player_index = 0;
    for (int i = 0; i < SESSION_MAX_PLAYERS; i++) {
        if (session->players[i].conn_id != -1) {
            game_add_player(session->game, session->players[i].name);
            session->players[i].player_index = player_index;
            player_index++;
        }
    }

    /* Start the game */
    if (!game_start(session->game)) {
        fprintf(stderr, "SessionRegistry: Failed to start game for session %d\n",
                session_id);
        game_free(session->game);
        session->game = NULL;
        return false;
    }

    session->state = SESSION_PLAYING;

    printf("SessionRegistry: Session %d started with %d players\n",
           session_id, session->player_count);

    return true;
}
/* }}} */

/* {{{ session_end */
void session_end(SessionRegistry* registry, int session_id, int winner_player_index) {
    if (registry == NULL) {
        return;
    }

    GameSession* session = session_get(registry, session_id);
    if (session == NULL) {
        return;
    }

    session->state = SESSION_FINISHED;

    /* Find winner name */
    const char* winner_name = "Unknown";
    for (int i = 0; i < SESSION_MAX_PLAYERS; i++) {
        if (session->players[i].player_index == winner_player_index) {
            winner_name = session->players[i].name;
            break;
        }
    }

    printf("SessionRegistry: Session %d ended. Winner: %s (player %d)\n",
           session_id, winner_name, winner_player_index);
}
/* }}} */

/* ========================================================================== */
/*                              Spectators                                     */
/* ========================================================================== */

/* {{{ session_add_spectator */
bool session_add_spectator(SessionRegistry* registry, int session_id, int conn_id) {
    if (registry == NULL) {
        return false;
    }

    GameSession* session = session_get(registry, session_id);
    if (session == NULL) {
        return false;
    }

    if (!session->allow_spectators) {
        fprintf(stderr, "SessionRegistry: Session %d doesn't allow spectators\n",
                session_id);
        return false;
    }

    int slot = find_spectator_slot(session);
    if (slot < 0) {
        fprintf(stderr, "SessionRegistry: Session %d spectator slots full\n",
                session_id);
        return false;
    }

    session->spectator_conn_ids[slot] = conn_id;
    session->spectator_count++;

    printf("SessionRegistry: Spectator joined session %d (total: %d)\n",
           session_id, session->spectator_count);

    return true;
}
/* }}} */

/* {{{ session_remove_spectator */
bool session_remove_spectator(SessionRegistry* registry, int session_id, int conn_id) {
    if (registry == NULL) {
        return false;
    }

    GameSession* session = session_get(registry, session_id);
    if (session == NULL) {
        return false;
    }

    for (int i = 0; i < SESSION_MAX_SPECTATORS; i++) {
        if (session->spectator_conn_ids[i] == conn_id) {
            session->spectator_conn_ids[i] = -1;
            session->spectator_count--;
            printf("SessionRegistry: Spectator left session %d\n", session_id);
            return true;
        }
    }

    return false;
}
/* }}} */

/* ========================================================================== */
/*                              Session Lookup                                 */
/* ========================================================================== */

/* {{{ session_get */
GameSession* session_get(SessionRegistry* registry, int session_id) {
    if (registry == NULL || session_id < 0) {
        return NULL;
    }

    for (int i = 0; i < SESSION_MAX_SESSIONS; i++) {
        if (registry->sessions[i].id == session_id) {
            return &registry->sessions[i];
        }
    }

    return NULL;
}
/* }}} */

/* {{{ session_find_by_conn */
GameSession* session_find_by_conn(SessionRegistry* registry, int conn_id) {
    if (registry == NULL || conn_id < 0) {
        return NULL;
    }

    for (int i = 0; i < SESSION_MAX_SESSIONS; i++) {
        GameSession* session = &registry->sessions[i];
        if (session->id == SESSION_INVALID_ID) {
            continue;
        }

        /* Check players */
        for (int j = 0; j < SESSION_MAX_PLAYERS; j++) {
            if (session->players[j].conn_id == conn_id) {
                return session;
            }
        }

        /* Check spectators */
        for (int j = 0; j < SESSION_MAX_SPECTATORS; j++) {
            if (session->spectator_conn_ids[j] == conn_id) {
                return session;
            }
        }
    }

    return NULL;
}
/* }}} */

/* {{{ session_get_player_slot */
int session_get_player_slot(GameSession* session, int conn_id) {
    if (session == NULL || conn_id < 0) {
        return -1;
    }

    for (int i = 0; i < SESSION_MAX_PLAYERS; i++) {
        if (session->players[i].conn_id == conn_id) {
            return i;
        }
    }

    return -1;
}
/* }}} */

/* {{{ session_is_host */
bool session_is_host(GameSession* session, int conn_id) {
    if (session == NULL || conn_id < 0) {
        return false;
    }

    return session->players[session->host_slot].conn_id == conn_id;
}
/* }}} */

/* ========================================================================== */
/*                              Lobby Listing                                  */
/* ========================================================================== */

/* {{{ session_list_joinable */
SessionListEntry* session_list_joinable(SessionRegistry* registry, int* count) {
    if (registry == NULL || count == NULL) {
        return NULL;
    }

    *count = 0;

    /* Count joinable sessions first */
    int joinable = 0;
    for (int i = 0; i < SESSION_MAX_SESSIONS; i++) {
        GameSession* session = &registry->sessions[i];
        if (session->id != SESSION_INVALID_ID &&
            session->state == SESSION_WAITING &&
            session->player_count < session->required_players) {
            joinable++;
        }
    }

    if (joinable == 0) {
        return NULL;
    }

    /* Allocate result array */
    SessionListEntry* entries = malloc(sizeof(SessionListEntry) * joinable);
    if (entries == NULL) {
        return NULL;
    }

    /* Fill in entries */
    int idx = 0;
    for (int i = 0; i < SESSION_MAX_SESSIONS && idx < joinable; i++) {
        GameSession* session = &registry->sessions[i];
        if (session->id != SESSION_INVALID_ID &&
            session->state == SESSION_WAITING &&
            session->player_count < session->required_players) {
            entries[idx].id = session->id;
            strncpy(entries[idx].name, session->name, SESSION_NAME_MAX);
            entries[idx].state = session->state;
            entries[idx].player_count = session->player_count;
            entries[idx].required_players = session->required_players;
            entries[idx].allow_spectators = session->allow_spectators;
            idx++;
        }
    }

    *count = joinable;
    return entries;
}
/* }}} */

/* {{{ session_list_spectatable */
SessionListEntry* session_list_spectatable(SessionRegistry* registry, int* count) {
    if (registry == NULL || count == NULL) {
        return NULL;
    }

    *count = 0;

    /* Count spectatable sessions first */
    int spectatable = 0;
    for (int i = 0; i < SESSION_MAX_SESSIONS; i++) {
        GameSession* session = &registry->sessions[i];
        if (session->id != SESSION_INVALID_ID &&
            session->allow_spectators &&
            session->state != SESSION_FINISHED) {
            spectatable++;
        }
    }

    if (spectatable == 0) {
        return NULL;
    }

    /* Allocate result array */
    SessionListEntry* entries = malloc(sizeof(SessionListEntry) * spectatable);
    if (entries == NULL) {
        return NULL;
    }

    /* Fill in entries */
    int idx = 0;
    for (int i = 0; i < SESSION_MAX_SESSIONS && idx < spectatable; i++) {
        GameSession* session = &registry->sessions[i];
        if (session->id != SESSION_INVALID_ID &&
            session->allow_spectators &&
            session->state != SESSION_FINISHED) {
            entries[idx].id = session->id;
            strncpy(entries[idx].name, session->name, SESSION_NAME_MAX);
            entries[idx].state = session->state;
            entries[idx].player_count = session->player_count;
            entries[idx].required_players = session->required_players;
            entries[idx].allow_spectators = session->allow_spectators;
            idx++;
        }
    }

    *count = spectatable;
    return entries;
}
/* }}} */

/* ========================================================================== */
/*                              Statistics                                     */
/* ========================================================================== */

/* {{{ session_count */
int session_count(const SessionRegistry* registry) {
    if (registry == NULL) {
        return 0;
    }
    return registry->session_count;
}
/* }}} */

/* {{{ session_count_by_state */
int session_count_by_state(const SessionRegistry* registry, SessionState state) {
    if (registry == NULL) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < SESSION_MAX_SESSIONS; i++) {
        if (registry->sessions[i].id != SESSION_INVALID_ID &&
            registry->sessions[i].state == state) {
            count++;
        }
    }
    return count;
}
/* }}} */

/* ========================================================================== */
/*                              Utility                                        */
/* ========================================================================== */

/* {{{ session_state_to_string */
const char* session_state_to_string(SessionState state) {
    switch (state) {
        case SESSION_WAITING:  return "waiting";
        case SESSION_PLAYING:  return "playing";
        case SESSION_FINISHED: return "finished";
        default:               return "unknown";
    }
}
/* }}} */
