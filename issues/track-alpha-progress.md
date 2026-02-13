# Track Alpha: Core Engine - Progress

This document tracks the progress of Track Alpha (Core Engine) which consolidates
the original Track A (Game Logic) issues.

## Track Overview

Track Alpha implements the core game engine in pure C. This is the foundation
that all other tracks depend on. It includes card structures, deck management,
player state, game mechanics, and serialization.

## Current Status: COMPLETE

Track Alpha is fully complete. All 22 issues finished, including the Phase 1 demo.

### In Progress

| Issue | Description | Status | Developer |
|-------|-------------|--------|-----------|
| - | All issues complete | - | - |

### Completed Issues

| Issue | Description | Date |
|-------|-------------|------|
| 1-001 | Card data structure | 2026-02-10 |
| 1-002 | Deck management system | 2026-02-10 |
| 1-003 | Player state management | 2026-02-10 |
| 1-004 | Trade row implementation | 2026-02-10 |
| 1-005 | Turn loop structure | 2026-02-10 |
| 1-006 | Basic combat resolution | 2026-02-10 |
| 1-007 | Card effect parser (all sub-issues) | 2026-02-10 |
| 1-008 | Auto-draw resolution system | 2026-02-11 |
| 1-009 | Deck flow tracker (d10/d4) | 2026-02-10 |
| 1-010 | Base card type | 2026-02-11 |
| 1-011 | Spawning mechanics | 2026-02-11 |
| 1-012 | Gamestate serialization | 2026-02-11 |
| 1-013 | Phase 1 Demo | 2026-02-12 |

### Pending Issues

None - Track Alpha complete.

## Checkpoint Status

| Checkpoint | Status | Notes |
|------------|--------|-------|
| ALPHA | COMPLETE | Card/Player/Game structs defined |
| BETA | COMPLETE | Serialization (1-012) finished 2026-02-11 |
| GAMMA | COMPLETE | All mechanics implemented |
| DELTA | COMPLETE | Phase 1 demo playable (1-013) |

## Deliverables

- [x] `src/core/01-card.h/c` - Card data structures
- [x] `src/core/02-deck.h/c` - Deck management
- [x] `src/core/03-player.h/c` - Player state
- [x] `src/core/04-trade-row.h/c` - Trade row
- [x] `src/core/05-game.h/c` - Game state and turn loop
- [x] `src/core/06-combat.h/c` - Combat resolution
- [x] `src/core/07-effects.h/c` - Effect system
- [x] `src/core/08-auto-draw.h/c` - Auto-draw system (1-008)
- [x] `src/core/09-serialize.h/c` - JSON serialization (1-012)
- [x] `tests/test-core.c` - Core unit tests (292 tests)
- [x] `tests/test-serialize.c` - Serialization tests (113 tests)
- [x] `src/demo/phase-1-demo.c` - CLI game loop (~1300 lines)
- [x] `run-phase1-demo.sh` - Demo launch script

## Notes

### 2026-02-11: BETA Checkpoint Complete
- 1-008 (auto-draw) and 1-012 (serialization) both complete
- BETA checkpoint achieved - Track Beta can now proceed with protocol work
- Total tests: 305 passing (192 core + 113 serialize)
- JSON serialization supports player-specific views (hidden opponent hands)

### 2026-02-12: TRACK ALPHA COMPLETE
- 1-013 Phase 1 Demo finished with full feature implementation
- Demo showcases all Phase 1 mechanics: cards, combat, bases, spawning, upgrades
- ANSI color UI with box-drawing characters
- ~1300 lines demonstrating all mechanics working together
- Total tests: 405 passing (292 core + 113 serialize)

### Dependencies on This Track
- **Track Beta (2-005)**: UNBLOCKED - 1-012 complete, can define protocol messages
- **Track Beta (2-008)**: UNBLOCKED - 1-012 provides hidden info serialization
- **Track Gamma (5-006)**: Needs game mechanics for trade row logic
- **All Tracks**: Phase 1 demo provides validation that core engine works
