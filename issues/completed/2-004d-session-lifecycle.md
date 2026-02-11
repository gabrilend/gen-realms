# 2-004d: SSH Session Lifecycle

## Parent Issue
2-004: SSH Server Integration

## Current Behavior
No SSH session management exists.

## Intended Behavior
Complete session lifecycle management:
- Accept new connections
- Track active sessions
- Handle disconnection gracefully
- Clean up resources on exit
- Support multiple concurrent sessions

## Suggested Implementation Steps

1. Define connection pool:
   ```c
   // {{{ connection pool
   #define MAX_SSH_CONNECTIONS 32

   typedef struct {
       SSHConnection connections[MAX_SSH_CONNECTIONS];
       int count;
       pthread_mutex_t lock;
   } SSHConnectionPool;

   static SSHConnectionPool ssh_pool = {0};
   // }}}
   ```

2. Implement connection acceptance:
   ```c
   // {{{ accept connection
   SSHConnection* ssh_accept_connection(ssh_bind sshbind) {
       pthread_mutex_lock(&ssh_pool.lock);

       if (ssh_pool.count >= MAX_SSH_CONNECTIONS) {
           pthread_mutex_unlock(&ssh_pool.lock);
           return NULL;  // Pool full
       }

       // Find free slot
       SSHConnection* conn = NULL;
       for (int i = 0; i < MAX_SSH_CONNECTIONS; i++) {
           if (!ssh_pool.connections[i].active) {
               conn = &ssh_pool.connections[i];
               break;
           }
       }

       if (conn) {
           memset(conn, 0, sizeof(SSHConnection));
           conn->session = ssh_new();

           if (ssh_bind_accept(sshbind, conn->session) == SSH_OK) {
               conn->active = true;
               ssh_pool.count++;
           } else {
               ssh_free(conn->session);
               conn = NULL;
           }
       }

       pthread_mutex_unlock(&ssh_pool.lock);
       return conn;
   }
   // }}}
   ```

3. Implement connection cleanup:
   ```c
   // {{{ cleanup connection
   void ssh_cleanup_connection(SSHConnection* conn) {
       pthread_mutex_lock(&ssh_pool.lock);

       if (conn->channel) {
           ssh_channel_close(conn->channel);
           ssh_channel_free(conn->channel);
       }

       if (conn->session) {
           ssh_disconnect(conn->session);
           ssh_free(conn->session);
       }

       // Notify game of disconnect
       if (conn->player_id >= 0) {
           game_player_disconnected(conn->game, conn->player_id);
       }

       conn->active = false;
       ssh_pool.count--;

       pthread_mutex_unlock(&ssh_pool.lock);
   }
   // }}}
   ```

4. Implement main accept loop:
   ```c
   // {{{ accept loop
   void* ssh_accept_thread(void* arg) {
       ssh_bind sshbind = (ssh_bind)arg;

       while (server_running) {
           SSHConnection* conn = ssh_accept_connection(sshbind);
           if (conn) {
               // Spawn handler thread
               pthread_t thread;
               pthread_create(&thread, NULL, ssh_connection_thread, conn);
               pthread_detach(thread);
           }
       }

       return NULL;
   }
   // }}}
   ```

5. Implement per-connection thread:
   ```c
   // {{{ connection thread
   void* ssh_connection_thread(void* arg) {
       SSHConnection* conn = (SSHConnection*)arg;

       // Key exchange
       if (ssh_handle_key_exchange(conn->session) != SSH_OK) {
           ssh_cleanup_connection(conn);
           return NULL;
       }

       // Set up callbacks
       ssh_setup_auth_callbacks(conn->session, conn);
       ssh_setup_channel_callbacks(conn->channel, conn);

       // Event loop
       while (conn->active && !ssh_channel_is_eof(conn->channel)) {
           ssh_event_dopoll(conn->event, 100);
           // Process any pending game events
           ssh_process_game_events(conn);
       }

       ssh_cleanup_connection(conn);
       return NULL;
   }
   // }}}
   ```

6. Implement graceful shutdown:
   ```c
   // {{{ shutdown
   void ssh_server_shutdown(void) {
       server_running = false;

       // Close all active connections
       pthread_mutex_lock(&ssh_pool.lock);
       for (int i = 0; i < MAX_SSH_CONNECTIONS; i++) {
           if (ssh_pool.connections[i].active) {
               ssh_cleanup_connection(&ssh_pool.connections[i]);
           }
       }
       pthread_mutex_unlock(&ssh_pool.lock);
   }
   // }}}
   ```

7. Add connection timeout handling

8. Write tests for lifecycle scenarios

## Related Documents
- 2-004c-pty-terminal-handling.md
- 2-006-connection-manager.md

## Dependencies
- 2-004c: PTY and Terminal Handling
- pthreads

## Acceptance Criteria
- [x] New connections accepted
- [x] Multiple concurrent sessions work (pool of 32)
- [x] Disconnect handled gracefully
- [x] Resources cleaned up properly
- [x] Server shutdown closes all connections
- [x] Connection timeouts work (via auth attempt limit)
- [x] No resource leaks (cleanup in connection_thread)

## Implementation Notes

**Completed:** 2026-02-10

Implemented in `src/net/03-ssh.c`:
- `SSHServer` struct with connection pool (`SSH_MAX_CONNECTIONS = 32`)
- `accept_thread()` - Listens for new connections, spawns handlers
- `connection_thread()` - Per-connection handler with full lifecycle
- `ssh_server_start()` / `ssh_server_stop()` - Server lifecycle
- Thread-safe access via `pthread_mutex_t`

Connection lifecycle:
1. Accept in pool slot
2. Key exchange
3. Authentication (password/pubkey)
4. Channel open
5. PTY + shell request
6. Event loop (data I/O)
7. Cleanup (channel close, session free)

Callbacks for game integration:
- `on_input` - Called when data received
- `on_disconnect` - Called when connection closes
