# 1-007e: Upgrade and Spawn Effects

## Parent Issue
1-007: Card Effect System

## Status: COMPLETED

## Current Behavior
Upgrade and spawn effects are fully implemented:

- **EFFECT_UPGRADE_ATTACK**: Creates pending action, player chooses card, applies attack_bonus
- **EFFECT_UPGRADE_TRADE**: Creates pending action, player chooses card, applies trade_bonus
- **EFFECT_UPGRADE_AUTH**: Creates pending action, player chooses card, applies authority_bonus
- **EFFECT_SPAWN**: Looks up card type by ID, creates instances in player's discard pile

## Implementation Summary

### Upgrade Effects
Upgrade effects use the pending action system:
1. Handler calls `game_request_upgrade(game, player_id, upgrade_type, upgrade_value)`
2. Creates `PENDING_UPGRADE` action (optional, can be skipped)
3. Player resolves with `game_resolve_upgrade(game, card_instance_id)`
4. Target card found in hand, discard, or played area
5. `card_instance_apply_upgrade()` applies the bonus permanently

### Spawn Effects
Spawn effects create units in the discard pile:
1. Effect's `target_card_id` specifies which unit type to spawn
2. Handler calls `game_find_card_type(game, target_card_id)`
3. Creates `effect->value` instances (default 1)
4. Each instance added to player's discard via `deck_add_to_discard()`

### Key Functions Added
- `game_request_upgrade()` - Creates pending upgrade action
- `game_resolve_upgrade()` - Resolves upgrade by applying bonus to chosen card
- `handle_upgrade_attack()` - Handler for EFFECT_UPGRADE_ATTACK
- `handle_upgrade_trade()` - Handler for EFFECT_UPGRADE_TRADE
- `handle_upgrade_auth()` - Handler for EFFECT_UPGRADE_AUTH
- `handle_spawn()` - Handler for EFFECT_SPAWN

### Files Modified
- `src/core/05-game.h` - Added upgrade function prototypes
- `src/core/05-game.c` - Implemented game_request_upgrade, game_resolve_upgrade
- `src/core/07-effects.c` - Added upgrade and spawn handlers to dispatch table
- `tests/test-core.c` - Added test_upgrade_spawn_effects() with 28 new tests

## Test Coverage (28 tests)
- Upgrade attack creates pending action
- Upgrade pending type, optional flag, stored values
- Upgrade resolved and bonus applied
- Upgrade trade effect
- Upgrade auth effect
- Upgrade can be skipped
- Spawn creates unit in discard
- Spawn creates correct type with unique ID
- Spawn multiple units (value > 1)
- Unknown spawn type ignored gracefully
- Upgrade works on cards in discard pile

## Related Documents
- docs/02-game-mechanics.md (upgrade system)
- 1-007a-effect-dispatch-infrastructure.md
- 1-010-base-card-type.md
- 1-011-spawning-mechanics.md

## Dependencies
- 1-007a: Effect Dispatch Infrastructure (completed)
- 1-001: Card Data Structure (CardInstance bonuses)
- 1-010: Base Card Type (completed)
- 1-011: Spawning Mechanics (completed)

## Acceptance Criteria
- [x] Upgrade effects modify CardInstance permanently
- [x] Upgrade bonuses persist across shuffles (stored in instance)
- [x] needs_regen flag set on upgraded cards (via card_instance_apply_upgrade)
- [x] Spawn effects create units in discard pile
- [x] Spawned units are proper CardInstances with unique IDs
- [x] Unknown spawn types handled gracefully

## Notes
- Spawn creates permanent units in discard (not temporary)
- Upgrade effects are optional (can be skipped)
- Upgrades can target cards in hand, discard, or played area
