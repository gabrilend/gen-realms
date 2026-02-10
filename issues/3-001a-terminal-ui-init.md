# 3-001a: Terminal UI Initialization

## Current Behavior
No terminal initialization or layout structure exists.

## Intended Behavior
Initialize ncurses with proper configuration and define the screen layout:
- Initialize ncurses with color support
- Create window structure for UI panels
- Define screen layout zones
- Configure color pairs for factions

## Suggested Implementation Steps

1. Create `src/client/01-terminal.h`:
   ```c
   // {{{ terminal types
   #ifndef TERMINAL_H
   #define TERMINAL_H

   #include <ncurses.h>

   typedef struct {
       WINDOW* status_win;    // Top: authority, d10/d4
       WINDOW* hand_win;      // Left: your hand
       WINDOW* trade_win;     // Right: trade row
       WINDOW* base_win;      // Bottom-left: bases
       WINDOW* narrative_win; // Bottom-right: story
       WINDOW* input_win;     // Bottom: command input
       int term_width;
       int term_height;
   } TerminalUI;
   // }}}
   ```

2. Define color pairs:
   ```c
   // {{{ colors
   typedef enum {
       COLOR_PAIR_DEFAULT = 1,
       COLOR_PAIR_MERCHANT,   // Yellow on black
       COLOR_PAIR_WILDS,      // Green on black
       COLOR_PAIR_KINGDOM,    // Blue on black
       COLOR_PAIR_ARTIFICER,  // Red on black
       COLOR_PAIR_NEUTRAL,    // White on black
       COLOR_PAIR_HIGHLIGHT   // Black on white
   } TerminalColors;
   // }}}
   ```

3. Implement `terminal_init()`:
   ```c
   // {{{ init
   TerminalUI* terminal_init(void) {
       initscr();
       cbreak();
       noecho();
       keypad(stdscr, TRUE);
       curs_set(0);

       if (has_colors()) {
           start_color();
           init_pair(COLOR_PAIR_MERCHANT, COLOR_YELLOW, COLOR_BLACK);
           init_pair(COLOR_PAIR_WILDS, COLOR_GREEN, COLOR_BLACK);
           init_pair(COLOR_PAIR_KINGDOM, COLOR_BLUE, COLOR_BLACK);
           init_pair(COLOR_PAIR_ARTIFICER, COLOR_RED, COLOR_BLACK);
       }

       TerminalUI* ui = malloc(sizeof(TerminalUI));
       getmaxyx(stdscr, ui->term_height, ui->term_width);
       terminal_create_windows(ui);
       return ui;
   }
   // }}}
   ```

4. Implement `terminal_create_windows()`:
   ```c
   // {{{ create windows
   void terminal_create_windows(TerminalUI* ui) {
       int w = ui->term_width;
       int h = ui->term_height;

       // Status bar at top (1 line)
       ui->status_win = newwin(1, w, 0, 0);

       // Hand window (left half, main area)
       ui->hand_win = newwin(h/2 - 1, w/2, 1, 0);

       // Trade row window (right half)
       ui->trade_win = newwin(h/2 - 1, w/2, 1, w/2);

       // Base window (bottom left)
       ui->base_win = newwin(h/2 - 2, w/2, h/2, 0);

       // Narrative window (bottom right)
       ui->narrative_win = newwin(h/2 - 2, w/2, h/2, w/2);

       // Input line at bottom
       ui->input_win = newwin(1, w, h - 1, 0);
   }
   // }}}
   ```

5. Implement `terminal_cleanup()` for proper shutdown

6. Write initialization tests

## Related Documents
- 3-001-terminal-renderer.md (parent issue)
- docs/04-architecture-c-server.md

## Dependencies
- ncurses library

## Acceptance Criteria
- [ ] ncurses initializes without errors
- [ ] Color pairs defined for all factions
- [ ] All windows created with correct dimensions
- [ ] Cleanup properly releases resources
