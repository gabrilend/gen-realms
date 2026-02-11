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
| 2-007 | Game Session Management | pending |
| 2-008 | Hidden Information Handling | pending |
| 2-009 | Input Validation | pending |
| 2-010 | Phase 2 Demo | pending |

## Completed: 14/18

## Technology Stack
- libwebsockets for HTTP/WebSocket server
- libssh (compiled from source) for SSH server
- JSON protocol over all transports
- Unified connection manager for transport-agnostic game logic

## Recent Progress

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
