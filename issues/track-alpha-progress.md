# Track Alpha: Core Engine - Progress

This document tracks the progress of Track Alpha (Core Engine) which consolidates
the original Track A (Game Logic) issues.

## Track Overview

Track Alpha implements the core game engine in pure C. This is the foundation
that all other tracks depend on. It includes card structures, deck management,
player state, game mechanics, and serialization.

## Current Status: Beta → Gamma (Advanced Mechanics)

### In Progress

| Issue | Description | Status | Developer |
|-------|-------------|--------|-----------|
| 1-008 | Auto-draw resolution system | in_progress | Active |
| 1-012 | Gamestate serialization | in_progress | Active |

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
| 1-009 | Deck flow tracker (d10/d4) | 2026-02-10 |
| 1-010 | Base card type | 2026-02-11 |
| 1-011 | Spawning mechanics | 2026-02-11 |

### Pending Issues

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 1-008a | Eligibility detection | 1-007 |
| 1-008b | Chain resolution | 1-008a, 1-008c |
| 1-008c | Spent flag management | 1-002 |
| 1-008d | Event emission | 1-008b |
| 1-013 | Phase 1 Demo | 1-012 |

## Checkpoint Status

| Checkpoint | Status | Notes |
|------------|--------|-------|
| ALPHA | COMPLETE | Card/Player/Game structs defined |
| BETA | IN PROGRESS | Needs 1-012 (serialization) |
| GAMMA | BLOCKED | Waiting on BETA |
| DELTA | BLOCKED | Waiting on GAMMA |

## Deliverables

- [x] `src/core/01-card.h/c` - Card data structures
- [x] `src/core/02-deck.h/c` - Deck management
- [x] `src/core/03-player.h/c` - Player state
- [x] `src/core/04-trade-row.h/c` - Trade row
- [x] `src/core/05-turn.h/c` - Turn loop
- [x] `src/core/06-combat.h/c` - Combat resolution
- [x] `src/core/07-effects.h/c` - Effect system
- [x] `src/core/08-deck-tracker.h/c` - d10/d4 tracking
- [ ] `src/core/09-serialize.h/c` - JSON serialization (1-012)
- [ ] `src/core/10-auto-draw.h/c` - Auto-draw system (1-008)
- [ ] `tests/test-core.c` - Complete unit tests
- [ ] `issues/completed/demos/phase-1-demo.sh` - CLI game loop

## Notes

### 2026-02-11: Current Sprint
- 1-008 (auto-draw) and 1-012 (serialization) in parallel development
- Both are required for BETA checkpoint
- Once complete, Track Beta can begin protocol work

### Dependencies on This Track
- **Track Beta (2-005)**: Needs 1-012 for protocol message format
- **Track Beta (2-008)**: Needs 1-012 for hidden information handling
- **Track Gamma (5-006)**: Needs game mechanics for trade row logic
