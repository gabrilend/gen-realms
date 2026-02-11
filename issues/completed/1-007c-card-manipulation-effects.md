# 1-007c: Card Manipulation Effects

## Parent Issue
1-007: Card Effect System

## Status
**COMPLETED** (2026-02-11)

## Current Behavior
Card manipulation effects implemented via pending action system:
- Draw cards from deck to hand (immediate, no pending)
- Force opponent to discard (creates pending action for opponent choice)
- Scrap cards from trade row (creates pending action)
- Scrap cards from hand or discard pile (creates pending action)
- Put card from discard on top of deck (creates pending action)

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
- [x] Draw effects pull cards from deck to hand
- [x] Opponent discard creates pending choice
- [x] Scrap trade row creates pending choice
- [x] Scrap hand/discard creates pending choice
- [x] Choices resolve correctly when input received

## Implementation Log

### 2026-02-11: Implemented
- Added `PendingActionType` enum and `PendingAction` struct to 05-game.h
- Added pending action queue to Game struct (max 8 pending actions)
- Implemented pending action queue functions (push/pop/get/clear)
- Implemented resolution functions for each pending action type:
  - `game_resolve_discard()` - discards card from opponent's hand
  - `game_resolve_scrap_trade_row()` - scraps card from trade row
  - `game_resolve_scrap_hand()` - scraps card from player's hand
  - `game_resolve_scrap_discard()` - scraps card from player's discard
  - `game_resolve_top_deck()` - puts discard card on top of deck
  - `game_skip_pending_action()` - skips optional actions
- Updated effect handlers in 07-effects.c:
  - `handle_draw()` - already worked, no changes needed
  - `handle_discard()` - creates PENDING_DISCARD for opponent
  - `handle_scrap_trade_row()` - creates PENDING_SCRAP_TRADE_ROW
  - `handle_scrap_hand()` - creates PENDING_SCRAP_HAND_DISCARD
  - `handle_top_deck()` - creates PENDING_TOP_DECK
- Added 42 tests for card manipulation effects (234 total tests passing)
