# 3-001b: Terminal Window Rendering

## Current Behavior
Terminal window rendering functions are implemented using Track A core structures.

## Intended Behavior
Implement rendering functions for each UI panel:
- Hand window shows player's cards
- Trade row window shows available purchases
- Base window shows deployed bases
- Narrative window shows story text
- Status bar shows game state

## Suggested Implementation Steps

1. Implement `terminal_render_status()`:
   ```c
   // {{{ render status
   void terminal_render_status(TerminalUI* ui, Game* game, int player_id) {
       werase(ui->status_win);
       Player* p = &game->players[player_id];
       Player* opp = &game->players[1 - player_id];

       mvwprintw(ui->status_win, 0, 0,
           "Turn %d | %s | You: Auth %d d10:%d d4:%d | Opp: Auth %d",
           game->turn, game->phase == MAIN ? "MAIN" : "DISCARD",
           p->authority, p->d10, p->d4,
           opp->authority);

       wrefresh(ui->status_win);
   }
   // }}}
   ```

2. Implement `terminal_render_hand()`:
   ```c
   // {{{ render hand
   void terminal_render_hand(TerminalUI* ui, Player* player) {
       werase(ui->hand_win);
       box(ui->hand_win, 0, 0);
       mvwprintw(ui->hand_win, 0, 2, " YOUR HAND ");

       for (int i = 0; i < player->hand_count; i++) {
           Card* card = player->hand[i];
           int color = faction_to_color_pair(card->type->faction);
           wattron(ui->hand_win, COLOR_PAIR(color));
           mvwprintw(ui->hand_win, i + 1, 1, "[%d] %s",
               i, card->type->name);
           wattroff(ui->hand_win, COLOR_PAIR(color));
       }

       mvwprintw(ui->hand_win, getmaxy(ui->hand_win) - 2, 1,
           "Trade: %d Combat: %d", player->trade, player->combat);

       wrefresh(ui->hand_win);
   }
   // }}}
   ```

3. Implement `terminal_render_trade_row()`:
   ```c
   // {{{ render trade row
   void terminal_render_trade_row(TerminalUI* ui, TradeRow* row) {
       werase(ui->trade_win);
       box(ui->trade_win, 0, 0);
       mvwprintw(ui->trade_win, 0, 2, " TRADE ROW ");

       for (int i = 0; i < row->count; i++) {
           Card* card = row->cards[i];
           int color = faction_to_color_pair(card->type->faction);
           wattron(ui->trade_win, COLOR_PAIR(color));
           mvwprintw(ui->trade_win, i + 1, 1, "[%d] %-20s %dg",
               i, card->type->name, card->type->cost);
           wattroff(ui->trade_win, COLOR_PAIR(color));
       }

       wrefresh(ui->trade_win);
   }
   // }}}
   ```

4. Implement `terminal_render_bases()`:
   ```c
   // {{{ render bases
   void terminal_render_bases(TerminalUI* ui, Player* player, Player* opponent) {
       werase(ui->base_win);
       box(ui->base_win, 0, 0);

       mvwprintw(ui->base_win, 0, 2, " YOUR BASES ");
       int y = 1;
       for (int i = 0; i < player->base_count; i++) {
           Base* base = &player->bases[i];
           mvwprintw(ui->base_win, y++, 1, "[%d] %s (%d def)",
               i, base->card->type->name, base->defense);
       }

       int mid = getmaxy(ui->base_win) / 2;
       mvwprintw(ui->base_win, mid, 2, " OPPONENT BASES ");
       y = mid + 1;
       for (int i = 0; i < opponent->base_count; i++) {
           Base* base = &opponent->bases[i];
           mvwprintw(ui->base_win, y++, 1, "[%d] %s (%d def)",
               i, base->card->type->name, base->defense);
       }

       wrefresh(ui->base_win);
   }
   // }}}
   ```

5. Implement `terminal_render_narrative()` with scrolling support

6. Implement `terminal_render()` to call all sub-renderers

7. Write rendering tests

## Related Documents
- 3-001a-terminal-ui-init.md
- 3-001-terminal-renderer.md (parent issue)

## Dependencies
- 3-001a: Terminal UI Initialization

## Acceptance Criteria
- [x] Hand window shows all cards with faction colors
- [x] Trade row shows cards with costs
- [x] Base window shows both players' bases
- [x] Status bar shows all game state info
- [x] Narrative window scrolls properly

## Implementation Notes (2026-02-10)

### Files Created
- `src/client/02-terminal-render.h` - Header with all render function declarations
- `src/client/02-terminal-render.c` - Implementation of all rendering functions

### Rendering Functions Implemented
```c
terminal_render()            // Master render calling all sub-renderers
terminal_render_status()     // Turn info, authority, d10/d4, trade/combat
terminal_render_hand()       // Player's hand with faction colors
terminal_render_trade_row()  // 5 slots + Explorer, with costs and affordability
terminal_render_bases()      // Both players' bases with defense values
terminal_render_narrative()  // Scrollable narrative text
terminal_render_input()      // Input prompt line
terminal_render_card()       // Single card with index and bonuses
terminal_render_card_list()  // Compact list for small windows
terminal_render_effects()    // Effect summary string
```

### Additional Features
1. Narrative buffer management (NarrativeBuffer struct) with scrolling
2. Effect formatting for display (+2T, +3C, Draw 2, etc.)
3. Affordability highlighting (dim cards player can't buy)
4. Upgrade bonus display (+xC +xT on upgraded cards)
5. Outpost indicator on base cards
6. Scroll indicators (^ and v) for narrative

### Integration with Track A
Uses core structures from:
- `01-card.h` (CardInstance, CardType, Faction, Effect)
- `02-deck.h` (Deck with hand, played, bases)
- `03-player.h` (Player with authority, trade, combat, d10/d4)
- `04-trade-row.h` (TradeRow with slots)
- `05-game.h` (Game, GamePhase)
