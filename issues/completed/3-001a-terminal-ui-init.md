# 3-001a: Terminal UI Initialization

## Current Behavior
Terminal UI initialization is implemented with ncurses support.

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
- [x] ncurses initializes without errors
- [x] Color pairs defined for all factions
- [x] All windows created with correct dimensions
- [x] Cleanup properly releases resources

## Implementation Notes (2026-02-10)

### Files Created
- `src/client/01-terminal.h` - Header with TerminalUI struct and TerminalColors enum
- `src/client/01-terminal.c` - Implementation of all terminal functions
- `tests/test-terminal.c` - Visual test program
- `Makefile` - Native build configuration

### Extended Color Pairs
Added additional color pairs beyond the original specification:
- COLOR_PAIR_AUTHORITY (cyan) - For authority display
- COLOR_PAIR_COMBAT (magenta) - For combat values
- COLOR_PAIR_TRADE (yellow) - For trade values
- COLOR_PAIR_ERROR (red) - For error messages
- COLOR_PAIR_SUCCESS (green) - For success messages

### Additional Features
1. SIGWINCH handler for terminal resize detection
2. `needs_resize` flag for safe resize handling in main loop
3. Minimum terminal size handling (60x20) with graceful degradation
4. `terminal_draw_box()` utility for bordered windows
5. `terminal_draw_status()` for status bar rendering
6. `terminal_get_faction_color()` helper function

### Design Decisions
1. Used static global `g_ui_for_resize` for signal handler (necessary for SIGWINCH)
2. nodelay(stdscr, TRUE) for non-blocking input compatible with resize
3. Windows are NULL-checked throughout to handle small terminal gracefully

### Build Verification
```
gcc -Wall -Wextra -pedantic -std=c11 -c src/client/01-terminal.c  # OK
gcc -Wall -Wextra -pedantic -std=c11 -c tests/test-terminal.c     # OK
gcc -o test-terminal *.o -lncurses                                 # OK
```
