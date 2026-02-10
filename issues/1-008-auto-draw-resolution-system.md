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

## Suggested Implementation Steps

1. Create `src/core/08-auto-draw.h` with type definitions
2. Create `src/core/08-auto-draw.c` with implementation
3. Add spent tracking to CardInstance:
   ```c
   // Already in CardInstance from 1-001:
   // bool draw_effect_spent;  // reset on shuffle
   ```
4. Implement `int autodraw_find_eligible(CardInstance** hand, int count, CardInstance** out)`
5. Implement `void autodraw_execute_all(Game* game, Player* player)`
6. Implement chain resolution:
   ```c
   void autodraw_resolve_chain(Game* game, Player* player) {
       bool found_new;
       do {
           found_new = false;
           // scan hand for unspent draw effects
           // execute them, mark spent
           // if new cards drawn, set found_new = true
       } while (found_new);
   }
   ```
7. Ensure `deck_shuffle()` resets spent flags on all cards
8. Integrate into game draw phase
9. Add event emission for each auto-draw (for LLM narrative)
10. Write tests in `tests/test-auto-draw.c`

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
