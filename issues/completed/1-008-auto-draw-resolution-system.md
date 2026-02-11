# 1-008: Auto-Draw Resolution System

## Current Behavior
No auto-draw system exists. Draw effects require manual play like other cards.

## Intended Behavior
A C-based auto-draw system that:
- Scans newly drawn hand for draw effects
- Automatically executes draw effects before main phase
- Marks effects as "spent" until next shuffle
- Chains auto-draws (new draws may have draw effects)
- Generates events for the auto-draw sequence (for narrative/display)
- Prevents tedious play-draw-play loops

## Sub-Issues

This issue has been split into the following sub-issues:

| ID | Description | Status |
|----|-------------|--------|
| 1-008a | Eligibility Detection | **completed** |
| 1-008b | Chain Resolution | **completed** |
| 1-008c | Spent Flag Management | **completed** |
| 1-008d | Event Emission | **completed** |

## Implementation Order

1. **1-008a** first - must detect eligible cards before anything else
2. **1-008c** second - spent flags needed for eligibility
3. **1-008b** third - chain resolution uses eligibility and spent flags
4. **1-008d** last - events wrap the chain resolution

## Related Documents
- docs/02-game-mechanics.md (auto-draw rules)
- docs/04-architecture-c-server.md
- 1-005-turn-loop-structure.md

## Dependencies
- 1-002: Deck Management (shuffle reset)
- 1-005: Turn Loop (integration point)
- 1-007: Card Effect System (effect execution)

## Acceptance Criteria
- [x] Auto-draw functions compile without errors
- [x] Draw effects trigger automatically at turn start
- [x] Spent effects do not trigger again until shuffle
- [x] Chained auto-draws resolve correctly
- [x] Events emitted for narrative integration
- [x] Protocol includes auto-draw sequence for client display

## Implementation Notes (2026-02-11)

### Files Created:
- `src/core/08-auto-draw.h` - Type definitions and function prototypes
- `src/core/08-auto-draw.c` - Implementation of all auto-draw functionality

### Key Components:

1. **Eligibility Detection (1-008a)**
   - `autodraw_has_draw_effect()` checks if CardType has EFFECT_DRAW
   - `autodraw_is_eligible()` checks both draw effect and spent flag
   - `autodraw_find_eligible()` scans hand array for candidates

2. **Spent Flag Management (1-008c)**
   - Uses existing `draw_effect_spent` field in CardInstance
   - `deck_shuffle()` already resets spent flags on shuffle
   - `autodraw_mark_spent()` / `autodraw_is_spent()` helper functions

3. **Chain Resolution (1-008b)**
   - `autodraw_resolve_chain()` iterates until no eligible cards
   - Maximum 20 iterations (safety limit)
   - `autodraw_execute_single()` draws cards and marks spent
   - Returns result code: OK, NO_ELIGIBLE, AWAITING_INPUT, ERROR_MAX_ITER

4. **Event Emission (1-008d)**
   - Four event types: START, TRIGGER, CARD, COMPLETE
   - Listener registration with context for callbacks
   - Events include iteration count and total drawn

### Integration:
Auto-draw resolution called automatically after drawing initial hand:
- `game_submit_draw_order()` calls `autodraw_resolve_chain()`
- `game_skip_draw_order()` calls `autodraw_resolve_chain()`

### Tests Added (192 total passing):
- 22 new tests covering eligibility, spent management, chain resolution, and events
