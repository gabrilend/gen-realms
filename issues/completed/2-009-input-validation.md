# 2-009: Input Validation

## Status: COMPLETE

## Current Behavior
Comprehensive server-side validation of all client actions via the
ValidationResult system. All actions are validated before execution
with clear error messages returned on failure.

## Original Behavior (before fix)
No server-side validation of client actions.

## Intended Behavior
Comprehensive input validation that:
- Validates all client actions server-side
- Prevents illegal moves
- Handles malformed requests
- Returns clear error messages
- Prevents cheating attempts

## Suggested Implementation Steps

1. Create `src/net/08-validation.h` with function declarations
2. Create `src/net/08-validation.c` with implementation
3. Implement action validators:
   ```c
   typedef struct {
       bool valid;
       char* error_message;
   } ValidationResult;

   ValidationResult validate_play_card(Game* game, int player_id,
                                        const char* instance_id);
   ValidationResult validate_buy_card(Game* game, int player_id, int slot);
   ValidationResult validate_attack(Game* game, int player_id,
                                     CombatTarget* target);
   ValidationResult validate_draw_order(Game* game, int player_id,
                                          int* order, int count);
   ```
4. Check turn ownership (is it this player's turn?)
5. Check phase appropriateness (can't buy during draw phase)
6. Check resource sufficiency (enough trade to buy)
7. Check target validity (card in hand, valid slot)
8. Return helpful error messages
9. Write tests for each validation case

## Related Documents
- docs/04-architecture-c-server.md
- 2-005-protocol-implementation.md

## Dependencies
- 2-005: Protocol Implementation
- 1-005: Turn Loop (game state)

## Validation Checks

| Action | Validations |
|--------|-------------|
| play_card | Is turn owner, card in hand, phase is main |
| buy_card | Is turn owner, slot valid, has trade, phase is main |
| attack | Is turn owner, has combat, target valid, outpost rules |
| draw_order | Is turn owner, phase is draw_order, indices valid |
| end_turn | Is turn owner, phase is main |

## Acceptance Criteria
- [x] All actions validated before execution
- [x] Wrong turn rejected
- [x] Wrong phase rejected
- [x] Invalid targets rejected
- [x] Error messages are helpful
- [x] No exploits possible

## Implementation Notes

### Files Created
- `src/net/08-validation.h` - ValidationResult type and function declarations
- `src/net/08-validation.c` - Implementation of all validators
- `tests/test-validation.c` - 21 unit tests

### Key Types
- `ValidationResult` - Contains valid flag, ProtocolError code, and message

### Validators Implemented
- `validate_is_player_turn` - Turn ownership check
- `validate_phase` - Phase appropriateness check
- `validate_game_in_progress` - Game state check
- `validate_play_card` - Card in hand, turn, phase
- `validate_buy_card` - Slot valid, has trade, turn, phase
- `validate_buy_explorer` - Has trade, turn, phase
- `validate_attack_player` - Has combat, valid target, outpost rules
- `validate_attack_base` - Has combat, base exists, outpost rules
- `validate_scrap_hand` - Card in hand, turn, phase
- `validate_scrap_discard` - Card in discard, turn, phase
- `validate_scrap_trade_row` - Slot valid, turn, phase
- `validate_end_turn` - Turn, phase, no pending actions
- `validate_draw_order` - Turn, phase, valid indices
- `validate_action` - Dispatch to specific validators
- `validate_pending_response` - Response matches pending
- `validate_pending_skip` - Pending is optional

### Test Coverage
21 tests covering turn ownership, phase validation, play card, buy card,
attack validation, end turn, action dispatch, and draw order validation.
