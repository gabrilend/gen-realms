# 1-006: Basic Combat Resolution

## Current Behavior
No combat system exists. Attack values cannot damage opponents.

## Intended Behavior
A C-based combat system that:
- Accumulates combat power from played cards
- Allows attacking opponent's authority
- Allows attacking opponent's bases (outposts first)
- Tracks damage dealt this turn
- Handles base destruction

## Suggested Implementation Steps

1. Create `src/core/06-combat.h` with type definitions
2. Create `src/core/06-combat.c` with implementation
3. Define combat target types:
   ```c
   typedef enum {
       TARGET_PLAYER,
       TARGET_BASE
   } TargetType;

   typedef struct {
       TargetType type;
       int player_index;       // which player to attack
       CardInstance* base;     // if targeting a base
   } CombatTarget;
   ```
4. Implement `int combat_get_valid_targets(Game* game, CombatTarget* out, int max)`
5. Implement `bool combat_can_attack(Game* game, CombatTarget* target)`
6. Implement `void combat_attack_player(Game* game, int amount)`
7. Implement `void combat_attack_base(Game* game, CardInstance* base, int amount)`
8. Implement `void combat_destroy_base(Game* game, CardInstance* base)`
9. Implement outpost priority check
10. Write tests in `tests/test-combat.c`

## Related Documents
- docs/02-game-mechanics.md
- 1-003-player-state-management.md

## Dependencies
- 1-003: Player State Management (for authority tracking)
- 1-005: Turn Loop Structure (combat occurs in main phase)

## Acceptance Criteria
- [x] Combat functions compile without errors
- [x] Combat power correctly reduces opponent authority
- [x] Cannot attack player while outposts exist
- [x] Base destruction removes base from play and to discard
- [x] Combat power tracked per player per turn
- [ ] Combat actions generate events for narrative hooks (Phase 5)

## Implementation Notes (2026-02-10)
- Created src/core/06-combat.{h,c}
- CombatTarget struct for valid target enumeration
- combat_get_valid_targets() respects outpost priority
- combat_attack_player() checks outposts, deals damage
- combat_attack_base() destroys if damage >= defense
- combat_destroy_base() moves to discard
- Game over detection integrated with game loop
- 13 tests passing
