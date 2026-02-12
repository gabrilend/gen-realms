# Phase 2 Progress: Networking Layer

## Goal
Implement the server-side networking infrastructure supporting both SSH (terminal) and WebSocket (browser) clients with a unified protocol.

## Status: In Progress

## Issues

| ID | Description | Status |
|----|-------------|--------|
| 2-001 | Configuration System | COMPLETE |
| 2-002 | HTTP Server | COMPLETE |
| 2-003 | WebSocket Handler | COMPLETE |
| 2-004 | SSH Server Integration | COMPLETE |
| 2-004a | ↳ libssh Build Integration | COMPLETE |
| 2-004b | ↳ SSH Authentication | COMPLETE |
| 2-004c | ↳ PTY and Terminal Handling | COMPLETE |
| 2-004d | ↳ Session Lifecycle | COMPLETE |
| 2-005 | Protocol Implementation | COMPLETE |
| 2-005a | ↳ Message Type Definitions | COMPLETE |
| 2-005b | ↳ Client to Server Handlers | COMPLETE |
| 2-005c | ↳ Server to Client Generation | COMPLETE |
| 2-005d | ↳ Validation and Error Handling | COMPLETE |
| 2-006 | Connection Manager | COMPLETE |
| 2-007 | Game Session Management | COMPLETE |
| 2-008 | Hidden Information Handling | COMPLETE |
| 2-009 | Input Validation | pending |
| 2-010 | Phase 2 Demo | pending |

## Completed: 16/18

## Technology Stack
- libwebsockets for HTTP/WebSocket server
- libssh (compiled from source) for SSH server
- JSON protocol over all transports
- Unified connection manager for transport-agnostic game logic

## Recent Progress

### 2-008: Hidden Information Handling (COMPLETE)
Implemented perspective-aware game state serialization:
- ViewPerspective enum (VIEW_SELF, VIEW_OPPONENT, VIEW_SPECTATOR)
- serialize_player_for_view() for perspective-based player data
- serialize_game_for_spectator() for full visibility spectator mode
- Updated serialize_player_public to include d10/d4 and discard pile
- 10 unit tests validating hidden information handling
- Verified no information leakage through JSON string search

Files modified:
- src/core/09-serialize.h - Added ViewPerspective and new function declarations
- src/core/09-serialize.c - Implementation of perspective handling

Files created:
- tests/test-hidden-info.c - 10 unit tests (all pass)

### 2-007: Game Session Management (COMPLETE)
Implemented session management for multiplayer game coordination:
- SessionRegistry tracks multiple concurrent game sessions
- WAITING -> PLAYING -> FINISHED lifecycle states
- Players can create, join, leave, and ready-up in sessions
- Host authority (host leaving WAITING session destroys it)
- Spectator support with configurable allowance
- Lobby listing functions for joinable/spectatable sessions
- Integration with Game module for starting games

Files created:
- src/net/07-sessions.h - Session types and API
- src/net/07-sessions.c - Full implementation
- tests/test-sessions.c - 23 unit tests (all pass)

### 2-006: Connection Manager (COMPLETE)
Implemented a unified connection registry that abstracts over both WebSocket and SSH connections. Key features:
- ConnectionRegistry with auto-incrementing IDs
- Registration/unregistration of both transport types
- Player and game assignment tracking
- Unified send functions (conn_send, conn_send_to_player)
- Broadcasting to game participants with optional exclusion
- Connection count statistics by type and game

Files created:
- src/net/06-connections.h - API definitions
- src/net/06-connections.c - Implementation
- tests/test-connections.c - 19 unit tests (all pass)

## Notes
Phase 2 enables multiplayer by implementing the server's networking layer. The server maintains authoritative game state and sends filtered updates to each client, hiding opponent hand contents until cards are played.

The connection manager (2-006) provides a clean abstraction layer that allows game session logic (2-007) to work with connections without needing to know whether they're WebSocket or SSH.
