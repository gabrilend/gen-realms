# 3-001: Terminal Renderer (ncurses)

## Current Behavior
No terminal rendering beyond basic printf output.

## Intended Behavior
A full ncurses-based TUI for SSH clients that:
- Renders game state in organized panels
- Shows hand, trade row, bases, opponent info
- Displays narrative text area
- Uses colors for factions
- Handles terminal resize
- Provides clear visual hierarchy

## Suggested Implementation Steps

1. Create `src/client/01-terminal.h` with type definitions
2. Create `src/client/01-terminal.c` with implementation
3. Initialize ncurses with color support
4. Define screen layout:
   ```c
   typedef struct {
       WINDOW* status_win;    // Top: authority, d10/d4
       WINDOW* hand_win;      // Left: your hand
       WINDOW* trade_win;     // Right: trade row
       WINDOW* base_win;      // Bottom-left: bases
       WINDOW* narrative_win; // Bottom-right: story
       WINDOW* input_win;     // Bottom: command input
   } TerminalUI;
   ```
5. Implement `void terminal_init(void)`
6. Implement `void terminal_render(Game* game, int player_id)`
7. Implement `void terminal_render_hand(WINDOW* win, Player* player)`
8. Implement `void terminal_render_trade_row(WINDOW* win, TradeRow* row)`
9. Implement `void terminal_render_narrative(WINDOW* win, const char* text)`
10. Add faction colors (Merchant=yellow, Wilds=green, Kingdom=blue, Artificer=red)
11. Handle SIGWINCH for resize
12. Write visual tests

## Related Documents
- docs/04-architecture-c-server.md
- 1-013-phase-1-demo.md

## Dependencies
- 1-012: Gamestate Serialization
- ncurses library

## Screen Layout

```
┌─────────────────────────────────────────────────────────────────┐
│ Turn 12 | MAIN | You: Auth 42 d10:7 d4:1 | Opp: Auth 28         │
├─────────────────────────────────┬───────────────────────────────┤
│ YOUR HAND                       │ TRADE ROW                     │
│ [0] Dire Bear +5C         Wilds │ [0] Knight Commander 4g  King │
│ [1] Guild Courier +2T   Merch   │ [1] Battle Golem 3g      Art  │
│ [2] Village Scout +1T   Neutral │ [2] Wolf Scout 1g       Wilds │
│                                 │ [3] Sellsword 3g       Neut   │
│ Trade: 3 Combat: 5              │ [4] Trading Post 3g    Merch  │
├─────────────────────────────────┼───────────────────────────────┤
│ YOUR BASES                      │ STORY                         │
│ [0] Trading Post (4 def)        │ The dire bear emerges from    │
│     Deployed, +2T/turn          │ the depths of the Thornwood.  │
│                                 │ Its roar echoes across the    │
│ OPPONENT BASES                  │ battlefield as it joins the   │
│ [0] Watchtower (3 def, outpost) │ growing pack of Lady Morgaine │
├─────────────────────────────────┴───────────────────────────────┤
│ > Command: _                                                     │
└─────────────────────────────────────────────────────────────────┘
```

## Sub-Issues
This issue has been split into sub-issues for manageable implementation:
- 3-001a: Terminal UI Initialization
- 3-001b: Terminal Window Rendering
- 3-001c: Terminal Formatting and Colors
- 3-001d: Terminal Input and Resize

## Acceptance Criteria
- [ ] ncurses initializes correctly (3-001a)
- [ ] All panels render with correct content (3-001b)
- [ ] Faction colors display (3-001c)
- [ ] Upgrade bonuses shown on cards (3-001c)
- [ ] Resize handled gracefully (3-001d)
- [ ] Scrollable narrative history (3-001b)
