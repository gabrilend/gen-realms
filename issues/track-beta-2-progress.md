# Track Beta-2: Network Protocol & Integration - Progress

This document tracks the progress of Track Beta-2, the second parallel development
phase for Infrastructure. This track focuses on implementing the network protocol,
WebSocket handling, and completing the Phase 2/3 demos.

## Track Overview

Track Beta-2 implements the critical networking path:
- Protocol message types and validation
- WebSocket server and connection management
- Game session management
- Hidden information handling
- Full client integration demos
- WASM JavaScript elimination (optional parallel work)

## Current Status: Protocol Implementation

### In Progress

| Issue | Description | Status | Developer |
|-------|-------------|--------|-----------|
| 2-010 | Phase 2 Demo | IN PROGRESS | Active |

### Completed Issues (Previous Phase)

| Issue | Description | Date |
|-------|-------------|------|
| 2-001 | Configuration system | 2026-02-10 |
| 2-002 | HTTP server (libwebsockets) | 2026-02-10 |
| 2-004a | libssh build integration | 2026-02-10 |
| 2-004b | SSH authentication | 2026-02-10 |
| 2-004c | PTY/terminal handling | 2026-02-10 |
| 2-004d | Session lifecycle | 2026-02-10 |
| 3-001a | Terminal UI initialization | 2026-02-10 |
| 3-001b | Terminal window rendering | 2026-02-10 |
| 3-001c | Terminal formatting | 2026-02-10 |
| 3-001d | Terminal input/resize | 2026-02-10 |
| 3-002 | Terminal input system | 2026-02-11 |
| 3-003 | WASM build configuration | 2026-02-10 |
| 3-004a | Canvas infrastructure | 2026-02-10 |
| 3-004b | Card rendering (Canvas) | 2026-02-10 |
| 3-004c | Game zones (Canvas) | 2026-02-10 |
| 3-004d | Status/narrative panels | 2026-02-10 |
| 3-005 | Browser input handler | 2026-02-11 |
| 3-006a | Preferences storage | 2026-02-11 |
| 3-006b | Preferences UI panel | 2026-02-11 |
| 3-006c | File-based export/import | 2026-02-11 |
| 3-007 | Draw order interface | 2026-02-11 |
| 3-008a | Animation core | 2026-02-11 |
| 3-008b | Card movement animations | 2026-02-11 |
| 3-008c | Attack/damage effects | 2026-02-11 |
| 3-009 | Narrative display | 2026-02-11 |

### Pending Issues (Critical Path)

| Issue | Description | Dependencies | Priority |
|-------|-------------|--------------|----------|
| 2-005* | Protocol implementation | 1-012 (done) | CRITICAL |
| 2-003 | WebSocket handler | 2-002 (done), 2-005 | HIGH |
| 2-006 | Connection manager | 2-003, 2-004 (done) | HIGH |
| 2-007 | Game session management | 2-006 | HIGH |
| 2-008 | Hidden information handling | 2-007, 1-012 (done) | HIGH |
| 2-009 | Input validation | 2-005 | HIGH |
| 2-010 | Phase 2 Demo | All above | HIGH |
| 3-010 | Phase 3 Demo | 2-010 | MEDIUM |

### Pending Issues (Parallel Work)

| Issue | Description | Dependencies | Priority |
|-------|-------------|--------------|----------|
| 3-011* | WASM JS elimination (10 sub-issues) | 3-003 (done) | MEDIUM |

## Critical Path Analysis

```
1-012 (DONE)
    │
    └──► 2-005 (Protocol) ──► 2-003 (WebSocket) ──► 2-006 (Connections)
              │                                           │
              └──► 2-009 (Validation)                     │
                                                          ▼
                                              2-007 (Sessions) ──► 2-008 (Hidden Info)
                                                          │
                                                          ▼
                                                      2-010 (Demo)
                                                          │
                                                          ▼
                                                      3-010 (Demo)
```

## Issue Details

### 2-005: Protocol Implementation (UNBLOCKED)

**Status:** Ready to implement
**Dependencies:** 1-012 complete (provides gamestate format)
**Sub-issues:** 2-005a through 2-005d

**Deliverables:**
- `src/net/04-protocol.h/c` - Message types and handlers
- Client→Server: join, action, chat, preferences
- Server→Client: gamestate, narrative, error
- JSON-based with cJSON

### 2-003: WebSocket Handler

**Status:** Blocked on 2-005
**Dependencies:** 2-002 (HTTP server done), 2-005 (protocol)

**Deliverables:**
- WebSocket upgrade handling
- Message framing and dispatch
- Per-client message queues

### 2-006: Connection Manager

**Status:** Blocked on 2-003
**Dependencies:** 2-003 (WebSocket), 2-004 (SSH done)

**Deliverables:**
- Unified client tracking (SSH + WebSocket)
- Join/leave handling
- Broadcast functionality

### 2-007: Game Session Management

**Status:** Blocked on 2-006

**Deliverables:**
- Game creation and joining
- Player assignment
- Spectator support
- Game lifecycle management

### 2-008: Hidden Information Handling

**Status:** Blocked on 2-007

**Deliverables:**
- Per-player gamestate views
- Hidden opponent hand
- Face-down deck counts only

### 2-009: Input Validation

**Status:** Blocked on 2-005

**Deliverables:**
- Server-side action validation
- Illegal move rejection
- Cheat prevention

### 2-010: Phase 2 Demo (IN PROGRESS)

**Status:** IN PROGRESS
**Dependencies:** All above issues

**Deliverables:**
- Two-player networked game
- SSH + WebSocket clients
- Protocol logging
- Hidden info verification

### 3-010: Phase 3 Demo

**Status:** Blocked on 2-010

**Deliverables:**
- Full ncurses terminal experience
- Full browser Canvas experience
- Both clients playing simultaneously
- All UI features working

### 3-011: WASM JS Elimination (Optional Parallel)

**Status:** CORE COMPLETE (final cleanup pending)
**Sub-issues:** 3-011a through 3-011j (10 sub-issues)

This large refactoring effort can proceed in parallel as it doesn't
affect the critical protocol path. It replaces 22 JavaScript files
with pure WebAssembly C code.

**Sub-issue Status (2026-02-12):**
- 3-011a: Core canvas infrastructure - COMPLETE
- 3-011b: Theme and layout constants - COMPLETE
- 3-011c: Input handling - COMPLETE
- 3-011d: Card and zone rendering - COMPLETE
- 3-011e: Panel rendering - COMPLETE
- 3-011f: Animation system - COMPLETE
- 3-011g: WebSocket communication - COMPLETE
- 3-011h: Preferences storage - COMPLETE
- 3-011i: AI integration modules - COMPLETE
- 3-011j: Final integration - COMPLETE (cleanup tasks pending)

**Files Created:**
- canvas.h/c - Canvas initialization and render loop
- draw2d.h/c - 2D drawing primitives
- theme.h/c - Colors, layout constants
- input.h/c - Mouse and keyboard handling
- card-renderer.h/c - Card rendering with faction colors
- zone-renderer.h/c - Hand, trade row, bases, play area
- panel-renderer.h/c - Status bar and narrative panel
- animation.h/c - Animation system with easing
- websocket.h/c - WebSocket via EM_ASM
- preferences.h/c - localStorage persistence
- ai-hooks.h/c - AI integration for narratives
- game-client.h/c - Main integration module

**Remaining cleanup:**
- Update index.html to minimal shell
- Update Makefile.wasm with new sources
- Test full integration
- Remove old JS files from assets/web/

## Checkpoint Status

| Checkpoint | Status | Notes |
|------------|--------|-------|
| ALPHA | COMPLETE | Config and rendering done |
| BETA | UNBLOCKED | 1-012 complete, can start 2-005 |
| GAMMA | BLOCKED | Waiting on 2-005 |
| DELTA | BLOCKED | Waiting on GAMMA |

## Statistics

**Server Issues:** 6 complete, 7 pending
**Client Issues:** 19 complete, 2 pending (3-010, 3-011)
**Total:** 25 complete, 9 pending

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
- [x] `assets/web/*.js` - Browser client (Canvas + input complete)
- [ ] Protocol integration (waiting on 2-005)
- [ ] `issues/completed/demos/phase-3-demo.sh` - Dual-client game

## Dependencies Provided

Track Beta-2 provides these to other tracks:

**To Track Gamma-2:**
- 3-010 (Phase 3 Demo) required for:
  - 5-010 (Phase 5 Demo)
  - 6-010 (Phase 6 Demo)
  - 4-010 (Phase 4 Demo)

## Notes

### 2026-02-12: Track Beta-2 Planning

Track Beta-2 is the critical path for the entire project. The protocol
implementation (2-005) is now unblocked since 1-012 is complete. Once
the protocol is done, the remaining networking issues can proceed
quickly in sequence.

The 3-011 WASM JavaScript elimination is large but optional parallel
work that can be assigned to a separate developer if available.

### Recommended Order

1. **2-005** - Protocol (unblocked, critical)
2. **2-003** - WebSocket (depends on 2-005)
3. **2-009** - Validation (depends on 2-005, can parallel with 2-003)
4. **2-006** - Connections (depends on 2-003)
5. **2-007** - Sessions (depends on 2-006)
6. **2-008** - Hidden info (depends on 2-007)
7. **2-010** - Phase 2 Demo (depends on all)
8. **3-010** - Phase 3 Demo (depends on 2-010)
9. **3-011** - WASM JS (parallel, lowest priority)
