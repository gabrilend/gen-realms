# 2-004: SSH Server Integration

## Current Behavior
No SSH capability exists. Terminal clients cannot connect.

## Intended Behavior
An embedded SSH server using libssh that:
- Listens on configured SSH port
- Authenticates connecting users
- Provides PTY for terminal interface
- Sends gamestate as text
- Receives input commands
- Compiled from source during build

## Suggested Implementation Steps

1. Add libssh to build system (compile from source)
2. Create `src/net/04-ssh.h` with type definitions
3. Create `src/net/04-ssh.c` with implementation
4. Define SSH session state:
   ```c
   typedef struct {
       ssh_session session;
       ssh_channel channel;
       int player_id;
       int game_id;
       char input_buffer[256];
   } SSHConnection;
   ```
5. Implement `void ssh_server_init(int port)`
6. Implement authentication callback (password or key-based)
7. Implement PTY request handling
8. Implement `void ssh_send_gamestate(SSHConnection* conn, Game* game)`
9. Implement input polling and command parsing
10. Handle disconnect cleanup
11. Write integration tests

## Related Documents
- docs/04-architecture-c-server.md
- 2-001-configuration-system.md

## Dependencies
- 2-001: Configuration System
- libssh (compiled from source)

## Build Notes

```makefile
# In Makefile
LIBSSH_VERSION = 0.10.6
deps/libssh:
    wget https://www.libssh.org/files/0.10/libssh-$(LIBSSH_VERSION).tar.xz
    tar xf libssh-$(LIBSSH_VERSION).tar.xz
    cd libssh-$(LIBSSH_VERSION) && mkdir build && cd build && \
        cmake .. && make
```

## Acceptance Criteria
- [ ] SSH server accepts connections
- [ ] Authentication works
- [ ] PTY established for terminal
- [ ] Gamestate renders as text
- [ ] Commands parse correctly
- [ ] Multiple concurrent SSH sessions work
