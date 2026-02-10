/* 01-terminal.h - Terminal UI definitions for Symbeline Realms
 *
 * Defines the ncurses-based terminal interface for playing the game in a
 * console environment. This provides a lightweight alternative to the
 * browser client for SSH/terminal access.
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include <ncurses.h>

/* {{{ terminal_colors
 * Color pairs for different factions and UI elements.
 * These map to ncurses color pairs initialized in terminal_init().
 */
typedef enum {
    COLOR_PAIR_DEFAULT = 1,    /* Gray on black - default text */
    COLOR_PAIR_MERCHANT,       /* Yellow on black - Merchant Guilds faction */
    COLOR_PAIR_WILDS,          /* Green on black - The Wilds faction */
    COLOR_PAIR_KINGDOM,        /* Blue on black - High Kingdom faction */
    COLOR_PAIR_ARTIFICER,      /* Red on black - Artificer Order faction */
    COLOR_PAIR_NEUTRAL,        /* White on black - Neutral cards */
    COLOR_PAIR_HIGHLIGHT,      /* Black on white - Selected items */
    COLOR_PAIR_AUTHORITY,      /* Cyan on black - Authority display */
    COLOR_PAIR_COMBAT,         /* Magenta on black - Combat value */
    COLOR_PAIR_TRADE,          /* Yellow on black - Trade value */
    COLOR_PAIR_ERROR,          /* Red on black - Error messages */
    COLOR_PAIR_SUCCESS         /* Green on black - Success messages */
} TerminalColors;
/* }}} */

/* {{{ terminal_ui
 * Main terminal UI structure containing all ncurses windows.
 * Windows are arranged in a split-screen layout:
 *
 *   +------------------+------------------+
 *   |     STATUS BAR (authority, d10/d4)  |
 *   +------------------+------------------+
 *   |                  |                  |
 *   |    HAND          |    TRADE ROW     |
 *   |    (your cards)  |    (buy cards)   |
 *   |                  |                  |
 *   +------------------+------------------+
 *   |                  |                  |
 *   |    BASES         |    NARRATIVE     |
 *   |    (in play)     |    (story text)  |
 *   |                  |                  |
 *   +------------------+------------------+
 *   |           INPUT LINE                |
 *   +-------------------------------------+
 */
typedef struct {
    WINDOW* status_win;        /* Top: authority, d10/d4, resources */
    WINDOW* hand_win;          /* Left upper: your hand cards */
    WINDOW* trade_win;         /* Right upper: trade row cards */
    WINDOW* base_win;          /* Left lower: bases in play */
    WINDOW* narrative_win;     /* Right lower: story/narrative text */
    WINDOW* input_win;         /* Bottom: command input line */
    int term_width;            /* Terminal width in columns */
    int term_height;           /* Terminal height in rows */
    int needs_resize;          /* Flag set when SIGWINCH received */
} TerminalUI;
/* }}} */

/* {{{ Function declarations */

/* Lifecycle functions */
TerminalUI* terminal_init(void);
void terminal_cleanup(TerminalUI* ui);

/* Window management */
void terminal_create_windows(TerminalUI* ui);
void terminal_destroy_windows(TerminalUI* ui);
void terminal_handle_resize(TerminalUI* ui);

/* Rendering functions */
void terminal_refresh_all(TerminalUI* ui);
void terminal_draw_box(WINDOW* win, const char* title);
void terminal_clear_window(WINDOW* win);

/* Status bar */
void terminal_draw_status(TerminalUI* ui, int authority, int d10, int d4,
                          int combat, int trade);

/* Color utilities */
int terminal_get_faction_color(const char* faction);

/* }}} */

#endif /* TERMINAL_H */
