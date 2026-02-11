# Track B: Server Infrastructure - Progress

This file tracks progress on Track B (Server Infrastructure) of the Symbeline Realms development.

## Track Overview

Track B implements the network layer enabling multi-client connections. It includes:
- Configuration system (Pre-Alpha)
- HTTP server with libwebsockets (Alpha → Beta)
- SSH server integration (Alpha → Beta)
- Protocol implementation (Beta → Gamma)
- Connection and session management (Gamma → Delta)

## Current Status

**Checkpoint Progress:** BETA complete! Protocol layer (2-005) implemented with 129 tests passing.

## Completed Issues

### 2-001: Configuration System
- **Status:** COMPLETE
- **Completed:** 2026-02-10
- **Files Created:**
  - `src/net/01-config.h` - Configuration structure and API definitions
  - `src/net/01-config.c` - JSON configuration parser implementation
  - `libs/cJSON.c` and `libs/cJSON.h` - cJSON library for JSON parsing
  - `config/server.json.example` - Example configuration file
  - `tests/test-config.c` - Unit tests (7 tests, all passing)
- **Features Implemented:**
  - JSON-based configuration loading
  - Default values for all settings
  - Validation for ports, timeouts, and game rules
  - Helper functions for endpoint URL formatting
  - Support for LLM and ComfyUI external service configuration

### 2-002: HTTP Server
- **Status:** COMPLETE
- **Completed:** 2026-02-10
- **Files Created:**
  - `src/net/02-http.h` - HTTP server API and types
  - `src/net/02-http.c` - libwebsockets-based HTTP server
  - `tests/test-http.c` - MIME type and utility tests
- **Features Implemented:**
  - Static file serving from assets/web/
  - MIME type detection for HTML, JS, CSS, Wasm, images, fonts
  - /api/config REST endpoint for service discovery
  - Security headers (X-Frame-Options, X-Content-Type-Options)
  - Directory traversal prevention
  - Automatic index.html for root path
- **Dependencies:** Requires `libwebsockets-devel` package

### 2-004: SSH Server Integration (all sub-issues complete)
- **Status:** COMPLETE
- **Completed:** 2026-02-10
- **Files Created:**
  - `src/net/03-ssh.h` - SSH server API and types
  - `src/net/03-ssh.c` - libssh-based SSH server implementation
  - `tests/test-ssh.c` - SSH server lifecycle tests
- **Sub-issues Completed:**
  - 2-004a: libssh Build Integration (using system package)
  - 2-004b: SSH Authentication (password + public key)
  - 2-004c: PTY and Terminal Handling
  - 2-004d: SSH Session Lifecycle (connection pool, threading)
- **Features Implemented:**
  - SSH server with connection pool (32 max)
  - Password and public key authentication
  - RSA host key generation
  - PTY allocation and terminal size negotiation
  - ANSI escape sequence utilities
  - Threaded connection handling
  - Welcome message and echo mode (development)
- **Dependencies:** Requires `libssh-devel` package

### 2-005: Protocol Implementation (all sub-issues complete)
- **Status:** COMPLETE
- **Completed:** 2026-02-11
- **Files Created:**
  - `src/net/04-protocol.h` - Protocol definitions (14 message types, 21 error codes)
  - `src/net/04-protocol.c` - Full protocol implementation (~500 lines)
  - `tests/test-protocol.c` - 129 comprehensive tests
- **Sub-issues Completed:**
  - 2-005a: Message Type Definitions (MessageType, ProtocolError, ChoiceType enums)
  - 2-005b: Client to Server Handlers (join, action, draw_order, end_turn, leave, chat)
  - 2-005c: Server to Client Generation (gamestate, narrative, error, player_joined/left, etc.)
  - 2-005d: Validation and Error Handling (parsing, validation helpers, error messages)
- **Features Implemented:**
  - Complete message type system with JSON serialization
  - Dispatch table routing for client message handlers
  - Server message generation functions
  - Validation helpers for JSON fields
  - 21 protocol error codes with human-readable messages
  - Integration with 1-012 gamestate serialization

### 2-003: WebSocket Handler
- **Status:** COMPLETE
- **Completed:** 2026-02-11
- **Files Created:**
  - `src/net/05-websocket.h` - WebSocket API and structures
  - `src/net/05-websocket.c` - Full implementation (~450 lines)
  - `tests/test-websocket.c` - Unit tests
- **Features Implemented:**
  - WSContext for connection pool management (64 max connections)
  - WSConnection per-connection state with player/game association
  - Protocol callback handling ESTABLISHED, RECEIVE, WRITEABLE, CLOSED
  - Integration with 04-protocol for message parsing/dispatch
  - HTTP server modified to include WebSocket protocol
  - ws_send() and ws_broadcast() for message delivery
  - ws_broadcast_gamestate() for player-specific filtered state
  - Player join/leave notifications
- **Dependencies:** Requires `libwebsockets-devel` package

## In Progress Issues

(None currently)

## Pending Issues

### Pre-Alpha → Alpha
- ~~2-002: HTTP server (libwebsockets)~~ - COMPLETE
- ~~2-004*: SSH server integration~~ - COMPLETE (all sub-issues)

### Alpha → Beta
- ~~2-005*: Protocol implementation~~ - COMPLETE
- ~~2-003: WebSocket handler~~ - COMPLETE
- 2-006: Connection manager - Depends on 2-003, 2-004 (UNBLOCKED)

### Beta → Gamma
- 2-007: Game session management - Depends on 2-006
- 2-008: Hidden information handling - Depends on 2-007, 1-012
- 2-009: Input validation - Depends on 2-005
- 2-010: Phase 2 Demo - Depends on all above

## Dependencies on Other Tracks

- **Track A (Game Logic):**
  - 2-005 (Protocol) blocked until 1-012 (Gamestate serialization) is complete
  - 2-008 (Hidden info) depends on 1-012 for gamestate format

## Notes

- cJSON library added to `libs/` for JSON parsing (MIT license)
- Configuration supports external LLM and ComfyUI service endpoints
- Game rule overrides allow customizing starting values for testing
