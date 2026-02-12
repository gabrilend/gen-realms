# 2-010: Phase 2 Demo

## Status: COMPLETE

## Current Behavior
Comprehensive demonstration of all Phase 2 networking components, simulating
network communication to show all components working together without requiring
actual network sockets.

## Original Behavior
Phase 1 demo is local-only, no networking.

## Intended Behavior
A demo that:
- Two players connect (one SSH, one WebSocket)
- Demonstrates protocol message flow
- Shows hidden information working
- Validates all actions server-side

## Implementation Notes

### Approach
Rather than requiring full network stack integration, the demo simulates
network communication to demonstrate all Phase 2 components working together.
This allows testing without external dependencies (libwebsockets/libssh) while
still validating the core networking logic.

### Files Created
- `src/demo/phase-2-demo.c` - Main demo implementation
- `src/demo/phase-2-stubs.c` - Stub functions for ws_send/ssh_send
- `scripts/run-phase2-demo.sh` - Launch script

### Files Modified
- `Makefile` - Added demo2 target

### Components Demonstrated

1. **Connection Management** (2-006)
   - SSH and WebSocket connection registration
   - Connection ID assignment
   - Transport-agnostic registry

2. **Session Management** (2-007)
   - Session creation with host
   - Player joining
   - Ready state tracking
   - Session state transitions (WAITING -> PLAYING)

3. **Hidden Information** (2-008)
   - Player sees own hand
   - Opponent hand hidden (only count shown)
   - Spectator sees all hands
   - Verified no information leakage

4. **Input Validation** (2-009)
   - Turn ownership validation
   - Phase appropriateness validation
   - Resource sufficiency validation
   - Target validity validation
   - Clear error messages

5. **Protocol** (2-005)
   - JSON message parsing
   - Gamestate serialization
   - Action message handling
   - Type-safe message dispatch

### Demo Output Sections

```
PHASE 2 DEMO - CONNECTION & SESSION MANAGEMENT
- Simulates SSH/WebSocket connections
- Creates game session, players join and ready up
- Starts game and broadcasts initial state

HIDDEN INFORMATION DEMONSTRATION
- Shows Alice's view (own hand visible, Bob's hidden)
- Shows Bob's view (own hand visible, Alice's hidden)
- Shows spectator view (all hands visible)

INPUT VALIDATION DEMONSTRATION
- Wrong turn rejected
- Invalid slot rejected
- Insufficient trade rejected
- Valid action accepted
- Attack without combat rejected

PROTOCOL MESSAGE FLOW
- Client action message parsing
- Server gamestate broadcast
- End turn message handling

PHASE 2 DEMO SUMMARY
- Component checklist
- Session/connection statistics
- Game state summary
```

## Acceptance Criteria
- [x] Protocol message parsing demonstrated
- [x] Session management demonstrated
- [x] Connection management demonstrated
- [x] Hidden information demonstrated
- [x] Input validation demonstrated
- [x] Color-coded logging for different clients
- [x] All components pass their tests
- [x] Clean exit without memory errors

## Related Documents
- All Phase 2 issues (2-001 through 2-009)
- 1-013-phase-1-demo.md

## Dependencies
- All previous Phase 2 issues (2-001 through 2-009)
- Phase 1 complete
