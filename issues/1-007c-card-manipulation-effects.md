# 1-007c: Card Manipulation Effects

## Parent Issue
1-007: Card Effect System

## Current Behavior
No card manipulation effects exist.

## Intended Behavior
Effect handlers for manipulating cards:
- Draw cards from deck to hand
- Force opponent to discard
- Scrap cards from trade row
- Scrap cards from hand or discard pile

## Suggested Implementation Steps

1. Implement `handle_draw_cards()`:
   ```c
   // {{{ handle_draw_cards
   static void handle_draw_cards(Game* game, Player* player, Effect* effect, CardInstance* source) {
       for (int i = 0; i < effect->value; i++) {
           CardInstance* card = deck_draw(player->deck);
           if (card) {
               hand_add(player->hand, card);
           }
       }
       // Note: auto-draw effects handled separately in 1-008
   }
   // }}}
   ```

2. Implement `handle_opponent_discard()`:
   ```c
   // {{{ handle_opponent_discard
   static void handle_opponent_discard(Game* game, Player* player, Effect* effect, CardInstance* source) {
       Player* opponent = game_get_opponent(game, player);
       // Opponent chooses which cards to discard
       // This requires input - mark as pending choice
       game_request_discard(game, opponent, effect->value);
   }
   // }}}
   ```

3. Implement `handle_scrap_trade_row()`:
   ```c
   // {{{ handle_scrap_trade_row
   static void handle_scrap_trade_row(Game* game, Player* player, Effect* effect, CardInstance* source) {
       // Player chooses card from trade row to scrap
       // This requires input - mark as pending choice
       game_request_scrap_trade_row(game, player, effect->value);
   }
   // }}}
   ```

4. Implement `handle_scrap_hand_discard()`:
   ```c
   // {{{ handle_scrap_hand_discard
   static void handle_scrap_hand_discard(Game* game, Player* player, Effect* effect, CardInstance* source) {
       // Player chooses card from hand or discard to scrap
       game_request_scrap_hand_discard(game, player, effect->value);
   }
   // }}}
   ```

5. Define pending choice structures for deferred input

6. Write tests in `tests/test-effects-cards.c`

## Related Documents
- 1-007a-effect-dispatch-infrastructure.md
- 1-002-deck-management-system.md

## Dependencies
- 1-007a: Effect Dispatch Infrastructure
- 1-002: Deck Management (draw, hand operations)
- 1-005: Turn Loop (choice handling)

## Acceptance Criteria
- [ ] Draw effects pull cards from deck to hand
- [ ] Opponent discard creates pending choice
- [ ] Scrap trade row creates pending choice
- [ ] Scrap hand/discard creates pending choice
- [ ] Choices resolve correctly when input received
