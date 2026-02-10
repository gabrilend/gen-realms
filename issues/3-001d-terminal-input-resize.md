# 3-001d: Terminal Input and Resize

## Current Behavior
No input handling or terminal resize support.

## Intended Behavior
Handle user input and terminal resize events:
- Process command input at bottom bar
- Handle SIGWINCH for terminal resize
- Rebuild windows on size change
- Input history support
- Command parsing

## Suggested Implementation Steps

1. Install SIGWINCH handler:
   ```c
   // {{{ sigwinch
   static volatile sig_atomic_t resize_pending = 0;

   void handle_sigwinch(int sig) {
       (void)sig;
       resize_pending = 1;
   }

   void terminal_setup_signals(void) {
       struct sigaction sa;
       sa.sa_handler = handle_sigwinch;
       sigemptyset(&sa.sa_mask);
       sa.sa_flags = 0;
       sigaction(SIGWINCH, &sa, NULL);
   }
   // }}}
   ```

2. Implement resize handling:
   ```c
   // {{{ resize
   void terminal_handle_resize(TerminalUI* ui) {
       if (!resize_pending) return;
       resize_pending = 0;

       endwin();
       refresh();
       getmaxyx(stdscr, ui->term_height, ui->term_width);

       // Delete old windows
       delwin(ui->status_win);
       delwin(ui->hand_win);
       delwin(ui->trade_win);
       delwin(ui->base_win);
       delwin(ui->narrative_win);
       delwin(ui->input_win);

       // Recreate with new dimensions
       terminal_create_windows(ui);
   }
   // }}}
   ```

3. Implement input reading:
   ```c
   // {{{ read input
   char* terminal_read_command(TerminalUI* ui) {
       static char buffer[256];
       werase(ui->input_win);
       mvwprintw(ui->input_win, 0, 0, "> Command: ");
       wrefresh(ui->input_win);

       echo();
       curs_set(1);
       wgetnstr(ui->input_win, buffer, sizeof(buffer) - 1);
       curs_set(0);
       noecho();

       return buffer;
   }
   // }}}
   ```

4. Implement command parsing:
   ```c
   // {{{ parse command
   typedef struct {
       char action;      // 'p'lay, 'b'uy, 'a'ttack, 'e'nd
       int target;       // Card/base index
       int secondary;    // Secondary target (for attacks)
   } Command;

   Command parse_command(const char* input) {
       Command cmd = {0};
       if (sscanf(input, "%c %d %d", &cmd.action, &cmd.target, &cmd.secondary) >= 1) {
           return cmd;
       }
       cmd.action = '?';  // Unknown
       return cmd;
   }
   // }}}
   ```

5. Implement input history (up/down arrows)

6. Add help command display

7. Write input handling tests

## Related Documents
- 3-001a-terminal-ui-init.md
- 3-001-terminal-renderer.md (parent issue)

## Dependencies
- 3-001a: Terminal UI Initialization
- 3-001b: Terminal Window Rendering

## Command Format

| Command | Format | Example |
|---------|--------|---------|
| Play card | `p <index>` | `p 2` |
| Buy card | `b <index>` | `b 0` |
| Attack player | `a` | `a` |
| Attack base | `a <base_idx>` | `a 1` |
| End turn | `e` | `e` |
| Help | `h` or `?` | `h` |

## Acceptance Criteria
- [ ] Commands parsed correctly
- [ ] Resize rebuilds all windows
- [ ] Input line clears after command
- [ ] Invalid input shows error
- [ ] Help displays command list
