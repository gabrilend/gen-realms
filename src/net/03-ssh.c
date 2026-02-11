/*
 * 03-ssh.c - SSH Server Implementation
 *
 * Uses libssh to provide SSH server functionality for terminal clients.
 * Handles authentication (password and public key), PTY allocation,
 * and session lifecycle management with threading for concurrent connections.
 */

/* Enable POSIX functions */
#define _POSIX_C_SOURCE 200809L

#include "03-ssh.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

/* {{{ Static State
 * Global server state for callback access.
 * Thread-safe access via mutex.
 */
static pthread_mutex_t server_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile bool server_running = false;
/* }}} */

/* {{{ ssh_ensure_host_key
 * Generates RSA host key if it doesn't exist.
 * Key is stored at the specified path.
 */
bool ssh_ensure_host_key(const char* key_path) {
    if (key_path == NULL) {
        return false;
    }

    /* Check if key already exists */
    if (access(key_path, F_OK) == 0) {
        printf("SSH: Using existing host key: %s\n", key_path);
        return true;
    }

    printf("SSH: Generating new host key: %s\n", key_path);

    /* Generate new RSA key */
    ssh_key key = NULL;
    int rc = ssh_pki_generate(SSH_KEYTYPE_RSA, 2048, &key);
    if (rc != SSH_OK) {
        fprintf(stderr, "SSH: Failed to generate host key\n");
        return false;
    }

    /* Export private key to file */
    rc = ssh_pki_export_privkey_file(key, NULL, NULL, NULL, key_path);
    ssh_key_free(key);

    if (rc != SSH_OK) {
        fprintf(stderr, "SSH: Failed to save host key to %s\n", key_path);
        return false;
    }

    /* Set restrictive permissions on key file */
    chmod(key_path, 0600);

    printf("SSH: Host key generated successfully\n");
    return true;
}
/* }}} */

/* {{{ Password Authentication Callback
 * Called when client attempts password authentication.
 * For development, accepts any non-empty password.
 */
static int auth_password_cb(ssh_session session, const char* user,
                           const char* password, void* userdata) {
    SSHConnection* conn = (SSHConnection*)userdata;
    (void)session;

    if (conn == NULL) {
        return SSH_AUTH_DENIED;
    }

    /* Rate limit check */
    if (conn->auth.failed_attempts >= 3) {
        printf("SSH: Too many failed attempts for user '%s'\n", user);
        return SSH_AUTH_DENIED;
    }

    /* Accept any non-empty password (development mode) */
    /* TODO: Implement proper authentication against user database */
    if (password != NULL && strlen(password) > 0) {
        strncpy(conn->auth.username, user, SSH_USERNAME_MAX - 1);
        conn->auth.username[SSH_USERNAME_MAX - 1] = '\0';
        conn->auth.method = SSH_AUTH_METHOD_PASSWORD;
        conn->auth.authenticated = true;
        printf("SSH: User '%s' authenticated (password)\n", user);
        return SSH_AUTH_SUCCESS;
    }

    conn->auth.failed_attempts++;
    printf("SSH: Auth failed for user '%s' (attempt %d)\n",
           user, conn->auth.failed_attempts);
    return SSH_AUTH_DENIED;
}
/* }}} */

/* {{{ Public Key Authentication Callback
 * Called when client attempts public key authentication.
 * Accepts any valid signature (development mode).
 */
static int auth_pubkey_cb(ssh_session session, const char* user,
                         struct ssh_key_struct* pubkey,
                         char signature_state, void* userdata) {
    SSHConnection* conn = (SSHConnection*)userdata;
    (void)session;
    (void)pubkey;

    if (conn == NULL) {
        return SSH_AUTH_DENIED;
    }

    /* Client is probing if key would be accepted */
    if (signature_state == SSH_PUBLICKEY_STATE_NONE) {
        return SSH_AUTH_SUCCESS;
    }

    /* Client provided valid signature */
    if (signature_state == SSH_PUBLICKEY_STATE_VALID) {
        strncpy(conn->auth.username, user, SSH_USERNAME_MAX - 1);
        conn->auth.username[SSH_USERNAME_MAX - 1] = '\0';
        conn->auth.method = SSH_AUTH_METHOD_PUBKEY;
        conn->auth.authenticated = true;
        printf("SSH: User '%s' authenticated (public key)\n", user);
        return SSH_AUTH_SUCCESS;
    }

    return SSH_AUTH_DENIED;
}
/* }}} */

/* {{{ PTY Request Callback
 * Called when client requests a pseudo-terminal.
 */
static int pty_request_cb(ssh_session session, ssh_channel channel,
                         const char* term, int cols, int rows,
                         int py, int px, void* userdata) {
    SSHConnection* conn = (SSHConnection*)userdata;
    (void)session;
    (void)channel;
    (void)py;
    (void)px;

    if (conn == NULL) {
        return SSH_ERROR;
    }

    conn->pty.width = cols;
    conn->pty.height = rows;
    if (term != NULL) {
        strncpy(conn->pty.term_type, term, SSH_TERM_TYPE_MAX - 1);
        conn->pty.term_type[SSH_TERM_TYPE_MAX - 1] = '\0';
    }
    conn->pty.allocated = true;

    printf("SSH: PTY allocated (%dx%d, term=%s)\n", cols, rows, term);
    return SSH_OK;
}
/* }}} */

/* {{{ Window Change Callback
 * Called when client terminal is resized.
 */
static int pty_resize_cb(ssh_session session, ssh_channel channel,
                        int cols, int rows, int py, int px,
                        void* userdata) {
    SSHConnection* conn = (SSHConnection*)userdata;
    (void)session;
    (void)channel;
    (void)py;
    (void)px;

    if (conn == NULL) {
        return SSH_ERROR;
    }

    conn->pty.width = cols;
    conn->pty.height = rows;

    printf("SSH: Terminal resized to %dx%d\n", cols, rows);

    /* TODO: Trigger game interface redraw */

    return SSH_OK;
}
/* }}} */

/* {{{ Shell Request Callback
 * Called when client requests a shell.
 * This is where we start the game interface.
 */
static int shell_request_cb(ssh_session session, ssh_channel channel,
                           void* userdata) {
    SSHConnection* conn = (SSHConnection*)userdata;
    (void)session;
    (void)channel;

    if (conn == NULL) {
        return SSH_ERROR;
    }

    printf("SSH: Shell requested for user '%s'\n", conn->auth.username);

    /* Send welcome message */
    const char* welcome =
        "\033[2J\033[H"  /* Clear screen, home cursor */
        "╔════════════════════════════════════════╗\r\n"
        "║      Welcome to Symbeline Realms       ║\r\n"
        "║           Terminal Edition             ║\r\n"
        "╚════════════════════════════════════════╝\r\n"
        "\r\n"
        "Logged in as: ";

    ssh_channel_write(conn->channel, welcome, strlen(welcome));
    ssh_channel_write(conn->channel, conn->auth.username,
                      strlen(conn->auth.username));
    ssh_channel_write(conn->channel, "\r\n\r\n", 4);

    /* TODO: Start game interface loop */

    return SSH_OK;
}
/* }}} */

/* {{{ Data Callback
 * Called when data is received from the client.
 */
static int data_cb(ssh_session session, ssh_channel channel,
                  void* data, uint32_t len, int is_stderr,
                  void* userdata) {
    SSHConnection* conn = (SSHConnection*)userdata;
    (void)session;
    (void)channel;
    (void)is_stderr;

    if (conn == NULL || data == NULL || len == 0) {
        return 0;
    }

    /* Call input handler if registered */
    if (conn->on_input != NULL) {
        conn->on_input(conn, (const char*)data, len);
    } else {
        /* Echo input for now (development) */
        ssh_channel_write(conn->channel, data, len);

        /* Check for quit command */
        if (len >= 4 && strncmp((char*)data, "quit", 4) == 0) {
            ssh_connection_send_string(conn, "\r\nGoodbye!\r\n");
            return -1;  /* Signal disconnect */
        }
    }

    return len;
}
/* }}} */

/* {{{ Channel Close Callback */
static void channel_close_cb(ssh_session session, ssh_channel channel,
                            void* userdata) {
    SSHConnection* conn = (SSHConnection*)userdata;
    (void)session;
    (void)channel;

    if (conn != NULL) {
        printf("SSH: Channel closed for user '%s'\n", conn->auth.username);
        conn->active = false;
    }
}
/* }}} */

/* {{{ Connection Thread
 * Handles a single SSH connection.
 */
static void* connection_thread(void* arg) {
    SSHConnection* conn = (SSHConnection*)arg;

    if (conn == NULL || conn->session == NULL) {
        return NULL;
    }

    printf("SSH: New connection (id=%d)\n", conn->connection_id);

    /* Perform key exchange */
    if (ssh_handle_key_exchange(conn->session) != SSH_OK) {
        fprintf(stderr, "SSH: Key exchange failed: %s\n",
                ssh_get_error(conn->session));
        goto cleanup;
    }

    /* Set up authentication callbacks */
    struct ssh_server_callbacks_struct server_cb = {
        .auth_password_function = auth_password_cb,
        .auth_pubkey_function = auth_pubkey_cb,
        .userdata = conn
    };
    ssh_callbacks_init(&server_cb);
    ssh_set_server_callbacks(conn->session, &server_cb);

    /* Wait for authentication */
    ssh_set_auth_methods(conn->session,
                         SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY);

    ssh_message msg;
    int auth_attempts = 0;

    while (!conn->auth.authenticated && auth_attempts < 10) {
        msg = ssh_message_get(conn->session);
        if (msg == NULL) {
            break;
        }

        if (ssh_message_type(msg) == SSH_REQUEST_AUTH) {
            switch (ssh_message_subtype(msg)) {
                case SSH_AUTH_METHOD_PASSWORD:
                    if (auth_password_cb(conn->session,
                            ssh_message_auth_user(msg),
                            ssh_message_auth_password(msg),
                            conn) == SSH_AUTH_SUCCESS) {
                        ssh_message_auth_reply_success(msg, 0);
                    } else {
                        ssh_message_reply_default(msg);
                    }
                    break;

                case SSH_AUTH_METHOD_PUBLICKEY:
                    /* Let libssh handle pubkey auth */
                    ssh_message_reply_default(msg);
                    break;

                case SSH_AUTH_METHOD_NONE:
                    ssh_message_auth_set_methods(msg,
                        SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY);
                    ssh_message_reply_default(msg);
                    break;

                default:
                    ssh_message_reply_default(msg);
                    break;
            }
        } else {
            ssh_message_reply_default(msg);
        }

        ssh_message_free(msg);
        auth_attempts++;
    }

    if (!conn->auth.authenticated) {
        printf("SSH: Authentication failed (id=%d)\n", conn->connection_id);
        goto cleanup;
    }

    /* Wait for channel open request */
    conn->channel = NULL;
    while (conn->channel == NULL && conn->active) {
        msg = ssh_message_get(conn->session);
        if (msg == NULL) {
            break;
        }

        if (ssh_message_type(msg) == SSH_REQUEST_CHANNEL_OPEN &&
            ssh_message_subtype(msg) == SSH_CHANNEL_SESSION) {
            conn->channel = ssh_message_channel_request_open_reply_accept(msg);
        } else {
            ssh_message_reply_default(msg);
        }

        ssh_message_free(msg);
    }

    if (conn->channel == NULL) {
        printf("SSH: No channel opened (id=%d)\n", conn->connection_id);
        goto cleanup;
    }

    /* Set up channel callbacks */
    struct ssh_channel_callbacks_struct channel_cb = {
        .channel_pty_request_function = pty_request_cb,
        .channel_pty_window_change_function = pty_resize_cb,
        .channel_shell_request_function = shell_request_cb,
        .channel_data_function = data_cb,
        .channel_close_function = channel_close_cb,
        .userdata = conn
    };
    ssh_callbacks_init(&channel_cb);
    ssh_set_channel_callbacks(conn->channel, &channel_cb);

    /* Handle channel requests (PTY, shell) */
    while (conn->active && !ssh_channel_is_eof(conn->channel)) {
        msg = ssh_message_get(conn->session);
        if (msg == NULL) {
            continue;
        }

        if (ssh_message_type(msg) == SSH_REQUEST_CHANNEL) {
            switch (ssh_message_subtype(msg)) {
                case SSH_CHANNEL_REQUEST_PTY:
                    pty_request_cb(conn->session, conn->channel,
                                   ssh_message_channel_request_pty_term(msg),
                                   ssh_message_channel_request_pty_width(msg),
                                   ssh_message_channel_request_pty_height(msg),
                                   ssh_message_channel_request_pty_pxwidth(msg),
                                   ssh_message_channel_request_pty_pxheight(msg),
                                   conn);
                    ssh_message_channel_request_reply_success(msg);
                    break;

                case SSH_CHANNEL_REQUEST_SHELL:
                    shell_request_cb(conn->session, conn->channel, conn);
                    ssh_message_channel_request_reply_success(msg);
                    break;

                case SSH_CHANNEL_REQUEST_WINDOW_CHANGE:
                    pty_resize_cb(conn->session, conn->channel,
                                  ssh_message_channel_request_pty_width(msg),
                                  ssh_message_channel_request_pty_height(msg),
                                  0, 0, conn);
                    break;

                default:
                    ssh_message_reply_default(msg);
                    break;
            }
        } else {
            ssh_message_reply_default(msg);
        }

        ssh_message_free(msg);
    }

cleanup:
    printf("SSH: Connection closed (id=%d, user=%s)\n",
           conn->connection_id, conn->auth.username);

    /* Call disconnect callback if registered */
    if (conn->on_disconnect != NULL) {
        conn->on_disconnect(conn);
    }

    /* Cleanup */
    if (conn->channel != NULL) {
        ssh_channel_send_eof(conn->channel);
        ssh_channel_close(conn->channel);
        ssh_channel_free(conn->channel);
        conn->channel = NULL;
    }

    if (conn->session != NULL) {
        ssh_disconnect(conn->session);
        ssh_free(conn->session);
        conn->session = NULL;
    }

    conn->active = false;

    return NULL;
}
/* }}} */

/* {{{ Accept Thread
 * Listens for and accepts new SSH connections.
 */
static void* accept_thread(void* arg) {
    SSHServer* server = (SSHServer*)arg;

    while (server_running && server->running) {
        /* Find free connection slot */
        SSHConnection* conn = NULL;
        pthread_mutex_lock(&server_lock);

        for (int i = 0; i < SSH_MAX_CONNECTIONS; i++) {
            if (!server->connections[i].active) {
                conn = &server->connections[i];
                memset(conn, 0, sizeof(SSHConnection));
                conn->connection_id = i;
                conn->player_id = -1;
                conn->active = true;
                server->connection_count++;
                break;
            }
        }

        pthread_mutex_unlock(&server_lock);

        if (conn == NULL) {
            /* Pool full, wait and retry */
            usleep(100000);  /* 100ms */
            continue;
        }

        /* Create new session */
        conn->session = ssh_new();
        if (conn->session == NULL) {
            conn->active = false;
            server->connection_count--;
            continue;
        }

        /* Accept connection */
        int rc = ssh_bind_accept(server->bind, conn->session);
        if (rc != SSH_OK) {
            ssh_free(conn->session);
            conn->session = NULL;
            conn->active = false;

            pthread_mutex_lock(&server_lock);
            server->connection_count--;
            pthread_mutex_unlock(&server_lock);

            continue;
        }

        /* Spawn connection handler thread */
        pthread_t thread;
        if (pthread_create(&thread, NULL, connection_thread, conn) != 0) {
            ssh_free(conn->session);
            conn->session = NULL;
            conn->active = false;

            pthread_mutex_lock(&server_lock);
            server->connection_count--;
            pthread_mutex_unlock(&server_lock);

            continue;
        }

        pthread_detach(thread);
    }

    return NULL;
}
/* }}} */

/* {{{ ssh_server_create */
SSHServer* ssh_server_create(const ServerConfig* config) {
    if (config == NULL) {
        return NULL;
    }

    SSHServer* server = calloc(1, sizeof(SSHServer));
    if (server == NULL) {
        return NULL;
    }

    server->config = config;
    server->running = false;
    server->connection_count = 0;
    server->host_key_path = strdup(SSH_HOST_KEY_PATH);

    if (server->host_key_path == NULL) {
        free(server);
        return NULL;
    }

    return server;
}
/* }}} */

/* {{{ ssh_server_start */
bool ssh_server_start(SSHServer* server) {
    if (server == NULL || server->running) {
        return false;
    }

    /* Ensure host key exists */
    if (!ssh_ensure_host_key(server->host_key_path)) {
        fprintf(stderr, "SSH: Failed to ensure host key\n");
        return false;
    }

    /* Create bind */
    server->bind = ssh_bind_new();
    if (server->bind == NULL) {
        fprintf(stderr, "SSH: Failed to create bind\n");
        return false;
    }

    /* Configure bind */
    ssh_bind_options_set(server->bind, SSH_BIND_OPTIONS_BINDPORT,
                         &server->config->ssh_port);
    ssh_bind_options_set(server->bind, SSH_BIND_OPTIONS_HOSTKEY,
                         server->host_key_path);

    /* Listen for connections */
    if (ssh_bind_listen(server->bind) < 0) {
        fprintf(stderr, "SSH: Failed to listen: %s\n",
                ssh_get_error(server->bind));
        ssh_bind_free(server->bind);
        server->bind = NULL;
        return false;
    }

    server->running = true;
    server_running = true;

    /* Start accept thread */
    pthread_t thread;
    if (pthread_create(&thread, NULL, accept_thread, server) != 0) {
        fprintf(stderr, "SSH: Failed to create accept thread\n");
        ssh_bind_free(server->bind);
        server->bind = NULL;
        server->running = false;
        server_running = false;
        return false;
    }

    pthread_detach(thread);

    printf("SSH server started on port %d\n", server->config->ssh_port);
    return true;
}
/* }}} */

/* {{{ ssh_server_stop */
void ssh_server_stop(SSHServer* server) {
    if (server == NULL || !server->running) {
        return;
    }

    server->running = false;
    server_running = false;

    /* Close all active connections */
    pthread_mutex_lock(&server_lock);
    for (int i = 0; i < SSH_MAX_CONNECTIONS; i++) {
        if (server->connections[i].active) {
            ssh_connection_close(&server->connections[i]);
        }
    }
    pthread_mutex_unlock(&server_lock);

    /* Free bind */
    if (server->bind != NULL) {
        ssh_bind_free(server->bind);
        server->bind = NULL;
    }

    printf("SSH server stopped\n");
}
/* }}} */

/* {{{ ssh_server_destroy */
void ssh_server_destroy(SSHServer* server) {
    if (server == NULL) {
        return;
    }

    if (server->running) {
        ssh_server_stop(server);
    }

    free(server->host_key_path);
    free(server);
}
/* }}} */

/* {{{ ssh_server_is_running */
bool ssh_server_is_running(const SSHServer* server) {
    if (server == NULL) {
        return false;
    }
    return server->running;
}
/* }}} */

/* {{{ ssh_server_get_connection_count */
int ssh_server_get_connection_count(const SSHServer* server) {
    if (server == NULL) {
        return 0;
    }
    return server->connection_count;
}
/* }}} */

/* {{{ ssh_connection_send */
int ssh_connection_send(SSHConnection* conn, const char* data, size_t len) {
    if (conn == NULL || conn->channel == NULL || data == NULL) {
        return -1;
    }

    return ssh_channel_write(conn->channel, data, len);
}
/* }}} */

/* {{{ ssh_connection_send_string */
int ssh_connection_send_string(SSHConnection* conn, const char* str) {
    if (str == NULL) {
        return -1;
    }
    return ssh_connection_send(conn, str, strlen(str));
}
/* }}} */

/* {{{ ssh_connection_clear_screen */
void ssh_connection_clear_screen(SSHConnection* conn) {
    ssh_connection_send(conn, "\033[2J\033[H", 7);
}
/* }}} */

/* {{{ ssh_connection_move_cursor */
void ssh_connection_move_cursor(SSHConnection* conn, int row, int col) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[%d;%dH", row, col);
    if (len > 0) {
        ssh_connection_send(conn, buf, len);
    }
}
/* }}} */

/* {{{ ssh_connection_set_color */
void ssh_connection_set_color(SSHConnection* conn, int fg, int bg) {
    char buf[32];
    int len;

    if (fg >= 0 && bg >= 0) {
        len = snprintf(buf, sizeof(buf), "\033[%d;%dm", fg, bg);
    } else if (fg >= 0) {
        len = snprintf(buf, sizeof(buf), "\033[%dm", fg);
    } else if (bg >= 0) {
        len = snprintf(buf, sizeof(buf), "\033[%dm", bg);
    } else {
        return;
    }

    if (len > 0) {
        ssh_connection_send(conn, buf, len);
    }
}
/* }}} */

/* {{{ ssh_connection_reset_attributes */
void ssh_connection_reset_attributes(SSHConnection* conn) {
    ssh_connection_send(conn, "\033[0m", 4);
}
/* }}} */

/* {{{ ssh_connection_close */
void ssh_connection_close(SSHConnection* conn) {
    if (conn == NULL) {
        return;
    }

    conn->active = false;

    if (conn->channel != NULL) {
        ssh_channel_send_eof(conn->channel);
        ssh_channel_close(conn->channel);
    }
}
/* }}} */
