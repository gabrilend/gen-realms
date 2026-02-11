# Track C: Client Rendering - Progress

This document tracks the progress of Track C (Client Rendering) from the parallelized
development roadmap.

## Track Overview

Track C implements terminal and browser client interfaces for Symbeline Realms.
It can be developed with mock gamestate data, with protocol integration after
the GAMMA checkpoint.

## Current Status: Alpha → Beta (Rendering Core)

### Completed Issues

| Issue | Description | Date |
|-------|-------------|------|
| 3-003 | Wasm build configuration | 2026-02-10 |
| 3-001a | Terminal UI initialization | 2026-02-10 |
| 3-004a | Canvas infrastructure | 2026-02-10 |
| 3-001b | Terminal window rendering | 2026-02-10 |
| 3-004b | Card rendering (Canvas) | 2026-02-10 |

### In Progress

None currently.

### Pending (Alpha → Beta: Rendering Core)

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 3-001c | Terminal formatting | 3-001b ✓ |
| 3-004c | Game zones (Canvas) | 3-004b ✓ |

### Pending (Beta → Gamma: Input Systems)

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 3-001d | Terminal input/resize | 3-001c |
| 3-002 | Terminal input system | 3-001d |
| 3-004d | Status/narrative panels | 3-004c |
| 3-005 | Browser input handler | 3-004d |

### Pending (Gamma → Delta: Integration)

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 3-006* | Client style preferences | 3-005 |
| 3-007 | Draw order interface | 3-005, 2-005* |
| 3-008* | Animation system | 3-004c |
| 3-009 | Narrative display | 3-004d |
| 3-010 | Phase 3 Demo | All above, 2-010 |

## Checkpoint Requirements

### For GAMMA Checkpoint
- All input systems working (3-002, 3-005)
- Can render mock gamestate data
- Ready for protocol integration

### For DELTA Checkpoint
- Full client integration with server
- Both terminal and browser clients functional
- Animation and narrative systems working

## Deliverables

Expected deliverables when Track C is complete:

- `src/client/terminal.c` - ncurses TUI client
- `src/client/wasm/` - Browser client (Canvas + WebSocket)
- `assets/web/` - Static HTML/CSS/JS
- `issues/completed/demos/phase-3-demo.sh` - Dual-client game

## Notes

### 2026-02-10: Initial Setup
- Created directory structure for client code
- Implemented Wasm build configuration with Emscripten
- Set up terminal UI with ncurses and faction colors
- Built canvas layout system with demo mode for testing without Wasm
- All Pre-Alpha starting issues (3-003, 3-001a, 3-004a) are now complete

### 2026-02-10: Alpha-Beta Rendering
- Implemented terminal window rendering (3-001b) with full Track A integration
  - Status bar, hand, trade row, bases, narrative windows
  - Faction color coding, effect formatting, scroll support
- Implemented canvas card rendering (3-004b)
  - Full card visuals with faction colors and effects
  - Cost badges, upgrade indicators, defense shields
  - Hover/selection states, face-down card backs
- Demo mode now shows realistic card rendering

### Dependencies on Other Tracks
- Track A (1-001): Card struct needed for card rendering
- Track B (2-005): Protocol needed for draw order interface
- Track B (2-010): Phase 2 Demo needed for Phase 3 Demo
