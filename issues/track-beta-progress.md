# Track Beta: Infrastructure - Progress

This document tracks the progress of Track Beta (Infrastructure) which consolidates
the original Track B (Server) and Track C (Client) issues.

## Track Overview

Track Beta implements the network layer and client interfaces. It includes:
- Server configuration and HTTP serving
- SSH server for terminal clients
- WebSocket/protocol for browser clients
- Terminal (ncurses) and browser (Canvas) rendering
- Input handling and animation systems

## Current Status: Waiting for BETA Checkpoint

Track Beta has completed all work that can be done without Track Alpha's 1-012
(gamestate serialization). Protocol implementation (2-005) is blocked.

### In Progress

None currently - waiting on Track Alpha (1-012).

### Completed Issues (Server - was Track B)

| Issue | Description | Date |
|-------|-------------|------|
| 2-001 | Configuration system | 2026-02-10 |
| 2-002 | HTTP server (libwebsockets) | 2026-02-10 |
| 2-004a | libssh build integration | 2026-02-10 |
| 2-004b | SSH authentication | 2026-02-10 |
| 2-004c | PTY/terminal handling | 2026-02-10 |
| 2-004d | Session lifecycle | 2026-02-10 |

### Completed Issues (Client - was Track C)

| Issue | Description | Date |
|-------|-------------|------|
| 3-003 | Wasm build configuration | 2026-02-10 |
| 3-001a | Terminal UI initialization | 2026-02-10 |
| 3-004a | Canvas infrastructure | 2026-02-10 |
| 3-001b | Terminal window rendering | 2026-02-10 |
| 3-004b | Card rendering (Canvas) | 2026-02-10 |
| 3-001c | Terminal formatting | 2026-02-10 |
| 3-004c | Game zones (Canvas) | 2026-02-10 |
| 3-001d | Terminal input/resize | 2026-02-10 |
| 3-004d | Status/narrative panels | 2026-02-10 |
| 3-002 | Terminal input system | 2026-02-11 |
| 3-005 | Browser input handler | 2026-02-11 |
| 3-006a | Preferences storage | 2026-02-11 |
| 3-006b | Preferences UI panel | 2026-02-11 |
| 3-006c | File-based export/import | 2026-02-11 |
| 3-007 | Draw order interface | 2026-02-11 |
| 3-008a | Animation core | 2026-02-11 |
| 3-008b | Card movement animations | 2026-02-11 |
| 3-008c | Attack/damage effects | 2026-02-11 |
| 3-009 | Narrative display | 2026-02-11 |

### Pending Issues (Server)

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 2-005* | Protocol implementation | 1-012 (Track Alpha) |
| 2-003 | WebSocket handler | 2-002, 2-005 |
| 2-006 | Connection manager | 2-003, 2-004 |
| 2-007 | Game session management | 2-006 |
| 2-008 | Hidden information handling | 2-007, 1-012 |
| 2-009 | Input validation | 2-005 |
| 2-010 | Phase 2 Demo | All above |

### Pending Issues (Client)

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 3-010 | Phase 3 Demo | All above, 2-010 |

## Checkpoint Status

| Checkpoint | Status | Notes |
|------------|--------|-------|
| ALPHA | COMPLETE | Configuration and rendering ready |
| BETA | BLOCKED | Waiting on 1-012 from Track Alpha |
| GAMMA | BLOCKED | Waiting on BETA |
| DELTA | BLOCKED | Waiting on full integration |

## Statistics

**Server Issues:** 6 complete, 7 pending
**Client Issues:** 19 complete, 1 pending
**Total:** 25 complete, 8 pending

## Deliverables

### Server (Phase 2)
- [x] `src/net/01-config.h/c` - Configuration system
- [x] `src/net/02-http.h/c` - HTTP server
- [x] `src/net/03-ssh.h/c` - SSH server
- [ ] `src/net/04-protocol.h/c` - Message protocol (2-005)
- [ ] `src/net/05-websocket.h/c` - WebSocket handler (2-003)
- [ ] `src/net/06-connection.h/c` - Connection manager (2-006)
- [ ] `src/net/07-session.h/c` - Game sessions (2-007)
- [ ] `issues/completed/demos/phase-2-demo.sh` - Two-player network game

### Client (Phase 3)
- [x] `src/client/terminal.c` - ncurses TUI (rendering complete)
- [x] `assets/web/` - Browser client (Canvas + input complete)
- [ ] Protocol integration (waiting on 2-005)
- [ ] `issues/completed/demos/phase-3-demo.sh` - Dual-client game

## Notes

### 2026-02-11: Rendering Complete
All client rendering and input systems are complete. Both terminal and browser
clients can display mock gamestate data. Animation and narrative systems are
working in demo mode. Only protocol integration remains.

### Critical Path
1. Track Alpha completes 1-012 (serialization)
2. This track implements 2-005 (protocol)
3. WebSocket (2-003) and connections (2-006) follow
4. Phase 2 Demo (2-010) proves network game
5. Phase 3 Demo (3-010) proves dual clients

### Dependencies
- **Blocking:** Track Alpha 1-012 blocks all remaining server work
- **Blocked by this track:** Track Gamma demos (5-010, 6-010) need 3-010
