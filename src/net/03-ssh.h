/*
 * 03-ssh.h - SSH Server for Terminal Clients
 *
 * Provides SSH server functionality using libssh for terminal-based game clients.
 * Handles authentication, PTY allocation, and session management for players
 * connecting via SSH to play in their terminal.
 *
 * Dependencies: libssh (install via: xbps-install libssh-devel)
 */

#ifndef SYMBELINE_SSH_H
#define SYMBELINE_SSH_H

#include <stdbool.h>
#include "01-config.h"

/* Forward declarations to avoid libssh header dependency */
struct ssh_bind_struct;
typedef struct ssh_bind_struct* ssh_bind;
struct ssh_session_struct;
typedef struct ssh_session_struct* ssh_session;
struct ssh_channel_struct;
typedef struct ssh_channel_struct* ssh_channel;
struct ssh_event_struct;
typedef struct ssh_event_struct* ssh_event;

/* {{{ Constants */

#define SSH_MAX_CONNECTIONS 32
#define SSH_USERNAME_MAX 64
#define SSH_TERM_TYPE_MAX 32
#define SSH_HOST_KEY_PATH "config/ssh_host_key"

/* }}} */

/* {{{ Authentication Structures */

/* Authentication methods supported */
typedef enum {
    SSH_AUTH_METHOD_PASSWORD,
    SSH_AUTH_METHOD_PUBKEY,
    SSH_AUTH_METHOD_NONE
} SSHAuthMethod;

/* Authentication state for a connection */
typedef struct {
    char username[SSH_USERNAME_MAX];
    SSHAuthMethod method;
    bool authenticated;
    int failed_attempts;
} SSHAuthState;

/* }}} */

/* {{{ PTY Structures */

/* Pseudo-terminal state */
typedef struct {
    int width;
    int height;
    char term_type[SSH_TERM_TYPE_MAX];
    bool allocated;
} SSHPtyState;

/* }}} */

/* {{{ Connection Structures */

/* Single SSH connection */
typedef struct SSHConnection {
    /* SSH handles */
    ssh_session session;
    ssh_channel channel;
    ssh_event event;

    /* Connection state */
    bool active;
    int connection_id;

    /* Authentication */
    SSHAuthState auth;

    /* Terminal */
    SSHPtyState pty;

    /* Game state */
    int player_id;          /* -1 if not in game */
    void* game;             /* Pointer to game session */

    /* Callbacks for game events */
    void (*on_input)(struct SSHConnection* conn, const char* data, size_t len);
    void (*on_disconnect)(struct SSHConnection* conn);
} SSHConnection;

/* SSH server instance */
typedef struct {
    ssh_bind bind;
    const ServerConfig* config;
    char* host_key_path;
    bool running;

    /* Connection pool */
    SSHConnection connections[SSH_MAX_CONNECTIONS];
    int connection_count;
} SSHServer;

/* }}} */

/* {{{ SSH Server API */

/*
 * ssh_server_create
 * Creates and initializes an SSH server instance.
 * Uses port from config->ssh_port.
 * Returns NULL on failure.
 */
SSHServer* ssh_server_create(const ServerConfig* config);

/*
 * ssh_server_start
 * Starts the SSH server (non-blocking).
 * Spawns accept thread for incoming connections.
 * Returns true on success, false on failure.
 */
bool ssh_server_start(SSHServer* server);

/*
 * ssh_server_stop
 * Stops the SSH server gracefully.
 * Closes all active connections.
 */
void ssh_server_stop(SSHServer* server);

/*
 * ssh_server_destroy
 * Frees all resources associated with the SSH server.
 * Safe to call with NULL.
 */
void ssh_server_destroy(SSHServer* server);

/*
 * ssh_server_is_running
 * Returns true if server is currently running.
 */
bool ssh_server_is_running(const SSHServer* server);

/*
 * ssh_server_get_connection_count
 * Returns the number of active connections.
 */
int ssh_server_get_connection_count(const SSHServer* server);

/* }}} */

/* {{{ Connection API */

/*
 * ssh_connection_send
 * Sends data to the connected terminal.
 * Returns number of bytes sent, or -1 on error.
 */
int ssh_connection_send(SSHConnection* conn, const char* data, size_t len);

/*
 * ssh_connection_send_string
 * Sends a null-terminated string to the connected terminal.
 * Returns number of bytes sent, or -1 on error.
 */
int ssh_connection_send_string(SSHConnection* conn, const char* str);

/*
 * ssh_connection_clear_screen
 * Sends ANSI escape sequence to clear the terminal.
 */
void ssh_connection_clear_screen(SSHConnection* conn);

/*
 * ssh_connection_move_cursor
 * Moves cursor to specified row and column (1-indexed).
 */
void ssh_connection_move_cursor(SSHConnection* conn, int row, int col);

/*
 * ssh_connection_set_color
 * Sets terminal color using ANSI codes.
 * fg: foreground color (30-37, 90-97)
 * bg: background color (40-47, 100-107)
 * Use -1 for no change.
 */
void ssh_connection_set_color(SSHConnection* conn, int fg, int bg);

/*
 * ssh_connection_reset_attributes
 * Resets terminal attributes to default.
 */
void ssh_connection_reset_attributes(SSHConnection* conn);

/*
 * ssh_connection_close
 * Closes a specific connection gracefully.
 */
void ssh_connection_close(SSHConnection* conn);

/* }}} */

/* {{{ Host Key Management */

/*
 * ssh_ensure_host_key
 * Generates SSH host key if it doesn't exist.
 * Returns true on success (key exists or was generated).
 */
bool ssh_ensure_host_key(const char* key_path);

/* }}} */

#endif /* SYMBELINE_SSH_H */
