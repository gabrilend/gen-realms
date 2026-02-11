# 2-004c: PTY and Terminal Handling

## Parent Issue
2-004: SSH Server Integration

## Current Behavior
No PTY handling for terminal clients.

## Intended Behavior
PTY subsystem that:
- Allocates pseudo-terminal on client request
- Handles terminal size negotiation
- Processes terminal escape sequences
- Sends rendered game state as text
- Receives input from terminal

## Suggested Implementation Steps

1. Add PTY state to connection:
   ```c
   // {{{ pty state
   typedef struct {
       int width;
       int height;
       char term_type[32];
       bool allocated;
   } PTYState;

   typedef struct SSHConnection {
       ssh_session session;
       ssh_channel channel;
       SSHAuthState auth;
       PTYState pty;
       // ...
   } SSHConnection;
   // }}}
   ```

2. Implement PTY request callback:
   ```c
   // {{{ pty request callback
   static int pty_request_callback(ssh_session session,
                                    ssh_channel channel,
                                    const char *term,
                                    int cols, int rows,
                                    int py, int px,
                                    void *userdata) {
       SSHConnection* conn = (SSHConnection*)userdata;

       conn->pty.width = cols;
       conn->pty.height = rows;
       strncpy(conn->pty.term_type, term, sizeof(conn->pty.term_type) - 1);
       conn->pty.allocated = true;

       return SSH_OK;
   }
   // }}}
   ```

3. Implement window change callback:
   ```c
   // {{{ window change callback
   static int window_change_callback(ssh_session session,
                                      ssh_channel channel,
                                      int cols, int rows,
                                      int py, int px,
                                      void *userdata) {
       SSHConnection* conn = (SSHConnection*)userdata;

       conn->pty.width = cols;
       conn->pty.height = rows;

       // Trigger re-render with new dimensions
       connection_request_redraw(conn);

       return SSH_OK;
   }
   // }}}
   ```

4. Set up channel callbacks:
   ```c
   // {{{ channel callbacks
   void ssh_setup_channel_callbacks(ssh_channel channel, SSHConnection* conn) {
       struct ssh_channel_callbacks_struct cb = {
           .channel_pty_request_function = pty_request_callback,
           .channel_pty_window_change_function = window_change_callback,
           .channel_data_function = data_callback,
           .channel_shell_request_function = shell_request_callback,
           .userdata = conn
       };
       ssh_callbacks_init(&cb);
       ssh_set_channel_callbacks(channel, &cb);
   }
   // }}}
   ```

5. Implement shell request callback (starts game interface):
   ```c
   // {{{ shell request callback
   static int shell_request_callback(ssh_session session,
                                      ssh_channel channel,
                                      void *userdata) {
       SSHConnection* conn = (SSHConnection*)userdata;

       // Start the game interface
       connection_start_game_interface(conn);

       return SSH_OK;
   }
   // }}}
   ```

6. Implement data send with escape sequences:
   ```c
   // {{{ send with escapes
   void ssh_send_clear_screen(SSHConnection* conn) {
       ssh_channel_write(conn->channel, "\033[2J\033[H", 7);
   }

   void ssh_send_cursor_pos(SSHConnection* conn, int row, int col) {
       char buf[32];
       int len = snprintf(buf, sizeof(buf), "\033[%d;%dH", row, col);
       ssh_channel_write(conn->channel, buf, len);
   }
   // }}}
   ```

7. Handle input data callback

8. Write tests for PTY operations

## Related Documents
- 2-004b-ssh-authentication.md
- 3-001-terminal-renderer.md

## Dependencies
- 2-004b: SSH Authentication

## Terminal Escape Sequences

| Sequence | Description |
|----------|-------------|
| `\033[2J` | Clear screen |
| `\033[H` | Cursor home |
| `\033[{r};{c}H` | Move cursor |
| `\033[{n}m` | Set color/attribute |
| `\033[0m` | Reset attributes |

## Acceptance Criteria
- [x] PTY allocated on request
- [x] Terminal size negotiated
- [x] Window resize handled
- [x] Shell request starts interface
- [x] Escape sequences work
- [x] Input received correctly

## Implementation Notes

**Completed:** 2026-02-10

Implemented in `src/net/03-ssh.c`:
- `pty_request_cb()` - Handles PTY allocation, stores width/height/term_type
- `pty_resize_cb()` - Handles window resize events
- `shell_request_cb()` - Starts game interface with welcome message
- `data_cb()` - Receives input data from terminal

Terminal control functions in `03-ssh.h`:
- `ssh_connection_clear_screen()` - ANSI clear
- `ssh_connection_move_cursor()` - ANSI cursor positioning
- `ssh_connection_set_color()` - ANSI color codes
- `ssh_connection_reset_attributes()` - ANSI reset

PTY state stored in `SSHPtyState` struct with width, height, term_type.
