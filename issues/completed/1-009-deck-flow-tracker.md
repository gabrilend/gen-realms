# 1-009: Deck Flow Tracker (d10/d4)

## Current Behavior
No deck flow tracking exists. Buying and scrapping cards has no cumulative effect.

## Intended Behavior
C functions extending the Player module that:
- Maintain d10 counter (starts at 5)
- Increment d10 when buying a card
- Decrement d10 when scrapping a card
- On overflow (9->0): increment d4 (more cards drawn)
- On underflow (0->9): decrement d4 (fewer cards drawn)
- d4 affects cards drawn at turn start
- Minimum hand size of 1 card regardless of d4

## Suggested Implementation Steps

1. Add to `src/core/03-player.c` (extend player module)
2. Implement `void player_increment_d10(Player* p)`:
   ```c
   p->d10++;
   if (p->d10 > 9) {
       p->d10 = 0;
       p->d4++;
   }
   ```
3. Implement `void player_decrement_d10(Player* p)`:
   ```c
   p->d10--;
   if (p->d10 < 0) {
       p->d10 = 9;
       p->d4--;
   }
   ```
4. Implement `int player_get_hand_size(Player* p)`:
   ```c
   int size = 5 + p->d4;
   return size < 1 ? 1 : size;
   ```
5. Call increment from `trade_row_buy()`
6. Call decrement from scrap effect execution
7. Use `player_get_hand_size()` in draw phase
8. Write tests in `tests/test-player.c` for overflow/underflow

## Related Documents
- docs/02-game-mechanics.md (d10/d4 rules)
- docs/04-architecture-c-server.md
- 1-003-player-state-management.md

## Dependencies
- 1-003: Player State Management (d10/d4 stored in Player struct)
- 1-004: Trade Row (buy triggers increment)

## Acceptance Criteria
- [x] d10 increments on card purchase
- [x] d10 decrements on card scrap
- [x] Overflow correctly grants +1 draw (d10: 9->0, d4++)
- [x] Underflow correctly reduces draws by 1 (d10: 0->9, d4--)
- [x] Hand size never goes below 1
- [ ] d10/d4 included in gamestate protocol for display (1-012)

## Implementation Notes (2026-02-10)
- player_d10_increment() implemented in 03-player.c
- player_d10_decrement() implemented in 03-player.c
- Called from trade_row_buy() and trade_row_buy_explorer()
- Called from effect handlers (EFFECT_D10_UP, EFFECT_D10_DOWN)
- player_get_hand_size() returns base (5) + d4 with minimum 1
- 8 tests passing for d10/d4 overflow/underflow behavior
