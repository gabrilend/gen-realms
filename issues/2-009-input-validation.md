# 2-009: Input Validation

## Current Behavior
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
- [ ] All actions validated before execution
- [ ] Wrong turn rejected
- [ ] Wrong phase rejected
- [ ] Invalid targets rejected
- [ ] Error messages are helpful
- [ ] No exploits possible
