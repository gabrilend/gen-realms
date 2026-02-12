# Track Alpha-2: Core Engine Completion - Progress

This document tracks the progress of Track Alpha-2, the second parallel development
phase for the Core Engine track. This track focuses on completing the remaining
core engine work and producing the Phase 1 demo.

## Track Overview

Track Alpha-2 completes the core C game engine by producing a playable demo
that validates all implemented mechanics. With serialization (1-012) complete,
the engine is fully functional and needs demonstration.

## Current Status: Demo Production

### In Progress

| Issue | Description | Status | Developer |
|-------|-------------|--------|-----------|
| - | - | - | - |

### Completed Issues (Previous Phase)

| Issue | Description | Date |
|-------|-------------|------|
| 1-001 | Card data structure | 2026-02-10 |
| 1-002 | Deck management system | 2026-02-10 |
| 1-003 | Player state management | 2026-02-10 |
| 1-004 | Trade row implementation | 2026-02-10 |
| 1-005 | Turn loop structure | 2026-02-10 |
| 1-006 | Basic combat resolution | 2026-02-10 |
| 1-007* | Card effect parser (6 sub-issues) | 2026-02-10 |
| 1-008* | Auto-draw resolution (4 sub-issues) | 2026-02-11 |
| 1-009 | Deck flow tracker (d10/d4) | 2026-02-10 |
| 1-010 | Base card type | 2026-02-11 |
| 1-011 | Spawning mechanics | 2026-02-11 |
| 1-012 | Gamestate serialization | 2026-02-11 |

### Pending Issues

| Issue | Description | Dependencies | Priority |
|-------|-------------|--------------|----------|
| 1-013 | Phase 1 Demo | 1-012 (done) | HIGH |

## Issue Details

### 1-013: Phase 1 Demo

**Status:** Ready to implement
**Dependencies:** All satisfied (1-001 through 1-012 complete)

**Deliverables:**
- `src/demo/phase-1-demo.c` - CLI game loop
- `run-phase1-demo.sh` - Demo launch script
- Console-based game with two human players
- Validates: turn loop, effects, combat, bases, serialization

**Acceptance Criteria:**
- Two players can play complete game to victory
- All card types work (ships, bases, spawners)
- Effects resolve correctly with ally bonuses
- Auto-draw resolution works
- Game state displays properly in terminal

## Checkpoint Status

| Checkpoint | Status | Notes |
|------------|--------|-------|
| ALPHA | COMPLETE | Structures defined |
| BETA | COMPLETE | Serialization working |
| GAMMA | AVAILABLE | Ready for demo |
| DELTA | BLOCKED | Needs Phase 2 |

## Statistics

**Total Issues:** 13 (12 complete, 1 pending)
**Unit Tests:** 305 passing (192 core + 113 serialize)
**Engine Status:** Feature complete, demo pending

## Deliverables

- [x] `src/core/01-card.h/c` - Card data structures
- [x] `src/core/02-deck.h/c` - Deck management
- [x] `src/core/03-player.h/c` - Player state
- [x] `src/core/04-trade-row.h/c` - Trade row
- [x] `src/core/05-game.h/c` - Game state and turn loop
- [x] `src/core/06-combat.h/c` - Combat resolution
- [x] `src/core/07-effects.h/c` - Effect system
- [x] `src/core/08-auto-draw.h/c` - Auto-draw system
- [x] `src/core/09-serialize.h/c` - JSON serialization
- [x] `tests/test-core.c` - Core unit tests (192 tests)
- [x] `tests/test-serialize.c` - Serialization tests (113 tests)
- [ ] `src/demo/phase-1-demo.c` - CLI game loop (1-013)
- [ ] `run-phase1-demo.sh` - Demo script (1-013)

## Dependencies Provided

Track Alpha-2 provides these to other tracks:

**To Track Beta-2:**
- 1-012 provides gamestate format for 2-005 (protocol)
- Complete game engine for 2-010 (Phase 2 demo)

**To Track Gamma-2:**
- Game engine for 4-009 (balance validator)

## Notes

### 2026-02-12: Track Alpha-2 Planning

Track Alpha-2 has only one remaining issue (1-013) since the core engine
is feature-complete. The Phase 1 demo will validate all mechanics work
together in a playable game.

This track has the shortest remaining work and can be completed quickly
to unblock other tracks that depend on the demo.
