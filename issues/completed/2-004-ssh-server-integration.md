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

## Sub-Issues

This issue has been split into the following sub-issues:

| ID | Description | Status |
|----|-------------|--------|
| 2-004a | libssh Build Integration | pending |
| 2-004b | SSH Authentication | pending |
| 2-004c | PTY and Terminal Handling | pending |
| 2-004d | Session Lifecycle | pending |

## Implementation Order

1. **2-004a** first - library must be built before anything else
2. **2-004b** second - authentication needed before session
3. **2-004c** third - PTY handling for terminal display
4. **2-004d** last - session management ties it together

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
