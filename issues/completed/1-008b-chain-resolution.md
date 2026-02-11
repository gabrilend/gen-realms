# 1-008b: Auto-Draw Chain Resolution

## Parent Issue
1-008: Auto-Draw Resolution System

## Current Behavior
No chain resolution for auto-draws.

## Intended Behavior
Chain resolution system that:
- Executes auto-draw effects in order
- Handles newly drawn cards that also have auto-draw
- Prevents infinite loops
- Respects draw order choice mechanic
- Completes all chains before main phase

## Suggested Implementation Steps

1. Define chain state in `src/core/08-auto-draw.c`:
   ```c
   // {{{ chain state
   typedef struct {
       int iterations;
       int max_iterations;
       int cards_drawn;
       bool awaiting_draw_order;
   } AutoDrawChainState;

   #define MAX_CHAIN_ITERATIONS 20
   // }}}
   ```

2. Implement `autodraw_resolve_chain()`:
   ```c
   // {{{ resolve chain
   AutoDrawResult autodraw_resolve_chain(Game* game, Player* player) {
       AutoDrawChainState state = {
           .iterations = 0,
           .max_iterations = MAX_CHAIN_ITERATIONS,
           .cards_drawn = 0,
           .awaiting_draw_order = false
       };

       bool found_new;
       do {
           found_new = false;
           state.iterations++;

           // Safety check
           if (state.iterations > state.max_iterations) {
               return AUTODRAW_ERROR_MAX_ITERATIONS;
           }

           // Find eligible cards
           AutoDrawCandidate candidates[16];
           int count = autodraw_find_eligible(
               player->hand, player->hand_count,
               candidates, 16);

           // Execute each eligible card's draw effect
           for (int i = 0; i < count; i++) {
               int drawn = autodraw_execute_single(
                   game, player, &candidates[i]);

               if (drawn > 0) {
                   found_new = true;
                   state.cards_drawn += drawn;
               }
           }

       } while (found_new);

       return AUTODRAW_OK;
   }
   // }}}
   ```

3. Implement `autodraw_execute_single()`:
   ```c
   // {{{ execute single
   int autodraw_execute_single(Game* game, Player* player,
                                AutoDrawCandidate* candidate) {
       // Mark as spent first (prevents re-trigger)
       candidate->card->draw_effect_spent = true;

       // Execute the draw effect
       int draw_count = candidate->draw_effect->value;
       int actually_drawn = 0;

       for (int i = 0; i < draw_count; i++) {
           CardInstance* drawn = deck_draw(player->deck);
           if (drawn) {
               hand_add(player->hand, drawn);
               actually_drawn++;

               // Emit event for each card drawn
               game_emit_event(game, EVENT_AUTO_DRAW, drawn);
           }
       }

       return actually_drawn;
   }
   // }}}
   ```

4. Handle draw order integration:
   ```c
   // {{{ with draw order
   // If draw order choice is enabled, pause chain for player input
   if (game->config->draw_order_choice && draw_count > 1) {
       // Request draw order from player
       game_request_draw_order(game, player, draw_count);
       return AUTODRAW_AWAITING_INPUT;
   }
   // }}}
   ```

5. Add iteration tracking for debugging

6. Write tests for chain scenarios

## Related Documents
- 1-008a-eligibility-detection.md
- 3-007-draw-order-interface.md
- docs/02-game-mechanics.md

## Dependencies
- 1-008a: Eligibility Detection
- 1-002: Deck Management (draw function)

## Chain Example

```
Turn start: Player draws 5 cards
Hand contains: Guild Courier (+2T, Draw 1)

Chain iteration 1:
- Find eligible: Guild Courier
- Execute: Draw 1 card → Merchant Scout
- Mark Guild Courier as spent

Chain iteration 2:
- Find eligible: Merchant Scout has no draw effect
- No more eligible cards

Chain complete. Main phase begins.
```

## Acceptance Criteria
- [ ] Chain resolves all eligible cards
- [ ] Newly drawn cards checked for eligibility
- [ ] Max iterations prevents infinite loops
- [ ] Draw order choice respected
- [ ] Chain completes before main phase
