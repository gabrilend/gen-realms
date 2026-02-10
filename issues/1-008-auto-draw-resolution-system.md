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
| 1-008a | Eligibility Detection | pending |
| 1-008b | Chain Resolution | pending |
| 1-008c | Spent Flag Management | pending |
| 1-008d | Event Emission | pending |

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
- [ ] Auto-draw functions compile without errors
- [ ] Draw effects trigger automatically at turn start
- [ ] Spent effects do not trigger again until shuffle
- [ ] Chained auto-draws resolve correctly
- [ ] Events emitted for narrative integration
- [ ] Protocol includes auto-draw sequence for client display
