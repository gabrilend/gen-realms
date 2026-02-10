# 2-010: Phase 2 Demo

## Current Behavior
Phase 1 demo is local-only, no networking.

## Intended Behavior
A networked demo that:
- Two players connect (one SSH, one placeholder WebSocket)
- Play a complete game over the network
- Demonstrates protocol in action
- Shows hidden information working
- Validates all actions server-side

## Suggested Implementation Steps

1. Create `src/demo/phase-2-demo.c`
2. Create `run-phase2-demo.sh` in project root
3. Initialize server with Phase 1 game engine
4. Start HTTP/WebSocket listener
5. Start SSH listener
6. Create test scenario:
   - Terminal 1: Start server
   - Terminal 2: SSH client connects
   - Terminal 3: Simple WebSocket client (curl or wscat)
7. Play through complete game
8. Log all protocol messages
9. Verify hidden information handling

## Related Documents
- All Phase 2 issues
- 1-013-phase-1-demo.md

## Dependencies
- All previous Phase 2 issues (2-001 through 2-009)
- Phase 1 complete

## Demo Commands

```bash
# Terminal 1: Start server
./symbeline --server --config config/demo.json

# Terminal 2: SSH client
ssh -p 8022 localhost

# Terminal 3: WebSocket test (using websocat)
websocat ws://localhost:8080/ws
```

## Demo Output Example

```
=== SYMBELINE REALMS - Phase 2 Demo ===
Server started on port 8080 (HTTP/WS) and 8022 (SSH)

[10:00:01] SSH connection from 127.0.0.1
[10:00:02] Player "Alice" joined via SSH
[10:00:05] WebSocket connection from 127.0.0.1
[10:00:06] Player "Bob" joined via WebSocket
[10:00:06] Game starting: Alice vs Bob

[10:00:10] Alice: play_card dire_bear
[10:00:10] -> Validated, executing
[10:00:10] -> Broadcasting gamestate to all

[10:00:15] Bob (WS): {"type":"action","action":"buy_card","slot":2}
[10:00:15] -> Validated, executing
[10:00:15] -> Broadcasting gamestate to all
```

## Acceptance Criteria
- [ ] Server starts and listens on both ports
- [ ] SSH client can connect and play
- [ ] WebSocket client can connect and play
- [ ] Actions validate correctly
- [ ] Gamestate syncs between players
- [ ] Hidden info not leaked
- [ ] Complete game playable to victory
