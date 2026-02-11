# 1-007d: Special Effects

## Parent Issue
1-007: Card Effect System

## Status: COMPLETED

## Current Behavior
Special effect handlers implemented for complex mechanics:
- **Copy ship**: Creates PENDING_COPY_SHIP action, player selects a ship in play, executes that ship's effects
- **Destroy base**: Creates PENDING_DESTROY_BASE action, player selects opponent's base, base is freed
- **Acquire free**: Sets EffectContext.next_ship_free flag with max cost limit
- **Acquire top**: Sets EffectContext.next_ship_to_top flag for deck placement

## Implementation Summary

### Files Modified
- `src/core/01-card.h`: Added EFFECT_ACQUIRE_TOP to EffectType enum
- `src/core/05-game.h`: Added function prototypes for copy/destroy and game_buy_card/game_buy_explorer
- `src/core/05-game.c`: Implemented pending action request/resolve functions and purchase wrappers
- `src/core/07-effects.c`: Added handle_acquire_top, updated handle_destroy_base and handle_copy_ship handlers
- `tests/test-core.c`: Added test_special_effects() with 26 tests

### Key Functions Implemented
```c
/* Pending action requests */
void game_request_copy_ship(Game* game, int player_id);
void game_request_destroy_base(Game* game, int player_id);

/* Pending action resolution */
bool game_resolve_copy_ship(Game* game, const char* card_instance_id);
bool game_resolve_destroy_base(Game* game, const char* card_instance_id);

/* Purchase wrappers with effect context support */
CardInstance* game_buy_card(Game* game, int slot);
CardInstance* game_buy_explorer(Game* game);
```

### Effect Context Integration
The EffectContext struct (defined in 07-effects.c) tracks turn-scoped flags:
- `next_ship_free`: Next purchase costs 0 (up to `free_ship_max_cost`)
- `next_ship_to_top`: Next purchase goes to draw pile top instead of discard

The game_buy_card/game_buy_explorer functions check these flags and reset them after use.

## Original Intended Behavior
Effect handlers for special mechanics:
- Copy ship (play a ship as if it were another)
- Destroy base (attack and remove opponent's base)
- Next ship costs 0 (free purchase)
- Next ship goes to top of deck (instead of discard)

## Acceptance Criteria
- [x] Copy ship executes target ship's effects
- [x] Destroy base removes opponent's base
- [x] Next-ship-free flag reduces cost to 0
- [x] Next-ship-top flag puts purchase on deck
- [x] Flags reset after use

## Test Results
All 26 special effects tests pass:
- Copy ship creates pending, type correct, optional, resolved, gained trade/combat
- Destroy base creates pending, type correct, resolved, base removed
- Acquire free flag set, max cost set, buy succeeds, trade unchanged, flag reset
- Acquire top flag set, buy succeeds, draw pile increased, discard unchanged, card on top, flag reset
- Copy ship can be skipped

## Related Documents
- 1-007a-effect-dispatch-infrastructure.md
- 1-004-trade-row-implementation.md
- 1-010-base-card-type.md

## Dependencies
- 1-007a: Effect Dispatch Infrastructure
- 1-004: Trade Row (purchase modification)
- 1-010: Base Card Type (base destruction)
