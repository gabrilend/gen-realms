# 3-001c: Terminal Formatting and Colors

## Current Behavior
No formatted card display or faction colors.

## Intended Behavior
Rich text formatting for terminal display:
- Faction colors applied to cards
- Card stats formatted clearly
- Ally bonuses highlighted
- Upgrade indicators
- Box drawing characters for panels

## Suggested Implementation Steps

1. Implement `faction_to_color_pair()`:
   ```c
   // {{{ faction colors
   int faction_to_color_pair(Faction faction) {
       switch (faction) {
           case FACTION_MERCHANT:  return COLOR_PAIR_MERCHANT;
           case FACTION_WILDS:     return COLOR_PAIR_WILDS;
           case FACTION_KINGDOM:   return COLOR_PAIR_KINGDOM;
           case FACTION_ARTIFICER: return COLOR_PAIR_ARTIFICER;
           default:                return COLOR_PAIR_NEUTRAL;
       }
   }
   // }}}
   ```

2. Implement `format_card_line()`:
   ```c
   // {{{ format card
   void format_card_line(char* buf, size_t size, Card* card, bool show_ally) {
       CardType* t = card->type;
       char effects[64] = "";

       if (t->trade > 0) strcat(effects, "+%dT ");
       if (t->combat > 0) strcat(effects, "+%dC ");
       if (t->authority > 0) strcat(effects, "+%dA ");

       snprintf(buf, size, "%-16s %s", t->name, effects);

       if (show_ally && card->ally_active) {
           strncat(buf, " [ALLY]", size - strlen(buf) - 1);
       }
   }
   // }}}
   ```

3. Implement `format_faction_tag()`:
   ```c
   // {{{ faction tag
   const char* format_faction_tag(Faction faction) {
       switch (faction) {
           case FACTION_MERCHANT:  return "Merch";
           case FACTION_WILDS:     return "Wilds";
           case FACTION_KINGDOM:   return "King";
           case FACTION_ARTIFICER: return "Art";
           default:                return "Neut";
       }
   }
   // }}}
   ```

4. Implement highlight for selectable items:
   ```c
   // {{{ highlight
   void terminal_highlight_card(WINDOW* win, int y, int x, bool selected) {
       if (selected) {
           mvwchgat(win, y, x, -1, A_REVERSE, 0, NULL);
       }
   }
   // }}}
   ```

5. Add box drawing alternatives for non-UTF8 terminals

6. Implement upgrade badge display (+, ++, etc.)

7. Write formatting tests

## Related Documents
- 3-001b-terminal-window-rendering.md
- 4-003-faction-style-guides.md

## Dependencies
- 3-001a: Terminal UI Initialization
- 3-001b: Terminal Window Rendering

## Faction Color Scheme

| Faction | Color | ANSI Code |
|---------|-------|-----------|
| Merchant | Yellow | 3 |
| Wilds | Green | 2 |
| Kingdom | Blue | 4 |
| Artificer | Red | 1 |
| Neutral | White | 7 |

## Acceptance Criteria
- [ ] Faction colors display correctly
- [ ] Card effects formatted consistently
- [ ] Ally bonuses highlighted
- [ ] Selection highlighting works
- [ ] Graceful fallback for no-color terminals
