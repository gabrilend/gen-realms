/* 01-terminal.c - Terminal UI implementation for Symbeline Realms
 *
 * Implements the ncurses-based terminal interface. This file handles
 * initialization, window management, and basic rendering utilities.
 * Game-specific rendering is handled by higher-level modules.
 */

#include "01-terminal.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* {{{ Static variables */
static TerminalUI* g_ui_for_resize = NULL;
/* }}} */

/* {{{ signal_handler_winch
 * SIGWINCH handler to set resize flag when terminal is resized.
 * The actual resize is handled in the main loop to avoid ncurses issues.
 */
static void signal_handler_winch(int sig) {
    (void)sig;  /* unused */
    if (g_ui_for_resize != NULL) {
        g_ui_for_resize->needs_resize = 1;
    }
}
/* }}} */

/* {{{ terminal_init_colors
 * Initialize all color pairs used by the UI.
 * Called during terminal_init() if the terminal supports colors.
 */
static void terminal_init_colors(void) {
    /* Default: gray on black */
    init_pair(COLOR_PAIR_DEFAULT, COLOR_WHITE, COLOR_BLACK);

    /* Faction colors */
    init_pair(COLOR_PAIR_MERCHANT, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_PAIR_WILDS, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_PAIR_KINGDOM, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_PAIR_ARTIFICER, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_PAIR_NEUTRAL, COLOR_WHITE, COLOR_BLACK);

    /* UI colors */
    init_pair(COLOR_PAIR_HIGHLIGHT, COLOR_BLACK, COLOR_WHITE);
    init_pair(COLOR_PAIR_AUTHORITY, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_PAIR_COMBAT, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_PAIR_TRADE, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_PAIR_ERROR, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_PAIR_SUCCESS, COLOR_GREEN, COLOR_BLACK);
}
/* }}} */

/* {{{ terminal_init
 * Initialize ncurses and create the terminal UI structure.
 * Returns NULL on failure.
 */
TerminalUI* terminal_init(void) {
    /* Initialize ncurses */
    if (initscr() == NULL) {
        return NULL;
    }

    /* Configure ncurses settings */
    cbreak();              /* Disable line buffering */
    noecho();              /* Don't echo typed characters */
    keypad(stdscr, TRUE);  /* Enable function keys */
    curs_set(0);           /* Hide cursor */
    nodelay(stdscr, TRUE); /* Non-blocking input (for resize handling) */

    /* Initialize colors if available */
    if (has_colors()) {
        start_color();
        use_default_colors();
        terminal_init_colors();
    }

    /* Allocate UI structure */
    TerminalUI* ui = (TerminalUI*)malloc(sizeof(TerminalUI));
    if (ui == NULL) {
        endwin();
        return NULL;
    }
    memset(ui, 0, sizeof(TerminalUI));

    /* Get terminal dimensions */
    getmaxyx(stdscr, ui->term_height, ui->term_width);

    /* Create windows */
    terminal_create_windows(ui);

    /* Set up resize handler */
    g_ui_for_resize = ui;
    signal(SIGWINCH, signal_handler_winch);

    return ui;
}
/* }}} */

/* {{{ terminal_cleanup
 * Clean up ncurses and free the UI structure.
 */
void terminal_cleanup(TerminalUI* ui) {
    if (ui == NULL) return;

    /* Restore signal handler */
    signal(SIGWINCH, SIG_DFL);
    g_ui_for_resize = NULL;

    /* Destroy windows */
    terminal_destroy_windows(ui);

    /* End ncurses */
    endwin();

    /* Free structure */
    free(ui);
}
/* }}} */

/* {{{ terminal_create_windows
 * Create all ncurses windows based on current terminal size.
 * Window layout adapts to terminal dimensions.
 */
void terminal_create_windows(TerminalUI* ui) {
    int w = ui->term_width;
    int h = ui->term_height;

    /* Minimum size check */
    if (w < 60 || h < 20) {
        /* Terminal too small - create minimal windows */
        ui->status_win = newwin(1, w, 0, 0);
        ui->hand_win = newwin(h - 2, w, 1, 0);
        ui->trade_win = NULL;
        ui->base_win = NULL;
        ui->narrative_win = NULL;
        ui->input_win = newwin(1, w, h - 1, 0);
        return;
    }

    /* Calculate layout dimensions */
    int half_w = w / 2;
    int status_h = 1;
    int input_h = 1;
    int main_h = (h - status_h - input_h) / 2;
    int lower_h = h - status_h - input_h - main_h;

    /* Status bar at top (full width, 1 line) */
    ui->status_win = newwin(status_h, w, 0, 0);

    /* Hand window (left half, upper main area) */
    ui->hand_win = newwin(main_h, half_w, status_h, 0);

    /* Trade row window (right half, upper main area) */
    ui->trade_win = newwin(main_h, w - half_w, status_h, half_w);

    /* Base window (left half, lower main area) */
    ui->base_win = newwin(lower_h, half_w, status_h + main_h, 0);

    /* Narrative window (right half, lower main area) */
    ui->narrative_win = newwin(lower_h, w - half_w, status_h + main_h, half_w);

    /* Input line at bottom (full width, 1 line) */
    ui->input_win = newwin(input_h, w, h - 1, 0);
}
/* }}} */

/* {{{ terminal_destroy_windows
 * Destroy all ncurses windows.
 */
void terminal_destroy_windows(TerminalUI* ui) {
    if (ui->status_win) delwin(ui->status_win);
    if (ui->hand_win) delwin(ui->hand_win);
    if (ui->trade_win) delwin(ui->trade_win);
    if (ui->base_win) delwin(ui->base_win);
    if (ui->narrative_win) delwin(ui->narrative_win);
    if (ui->input_win) delwin(ui->input_win);

    ui->status_win = NULL;
    ui->hand_win = NULL;
    ui->trade_win = NULL;
    ui->base_win = NULL;
    ui->narrative_win = NULL;
    ui->input_win = NULL;
}
/* }}} */

/* {{{ terminal_handle_resize
 * Handle terminal resize by recreating all windows.
 * Call this when needs_resize flag is set.
 */
void terminal_handle_resize(TerminalUI* ui) {
    if (ui == NULL) return;

    ui->needs_resize = 0;

    /* Get new dimensions */
    endwin();
    refresh();
    getmaxyx(stdscr, ui->term_height, ui->term_width);

    /* Recreate windows */
    terminal_destroy_windows(ui);
    terminal_create_windows(ui);

    /* Force full redraw */
    clear();
    refresh();
    terminal_refresh_all(ui);
}
/* }}} */

/* {{{ terminal_refresh_all
 * Refresh all windows to display their contents.
 */
void terminal_refresh_all(TerminalUI* ui) {
    if (ui->status_win) wrefresh(ui->status_win);
    if (ui->hand_win) wrefresh(ui->hand_win);
    if (ui->trade_win) wrefresh(ui->trade_win);
    if (ui->base_win) wrefresh(ui->base_win);
    if (ui->narrative_win) wrefresh(ui->narrative_win);
    if (ui->input_win) wrefresh(ui->input_win);
}
/* }}} */

/* {{{ terminal_draw_box
 * Draw a border around a window with an optional title.
 */
void terminal_draw_box(WINDOW* win, const char* title) {
    if (win == NULL) return;

    box(win, 0, 0);

    if (title != NULL && strlen(title) > 0) {
        int w = getmaxx(win);
        int title_len = strlen(title);
        int title_x = (w - title_len - 2) / 2;  /* Center title */
        if (title_x < 1) title_x = 1;

        mvwprintw(win, 0, title_x, " %s ", title);
    }
}
/* }}} */

/* {{{ terminal_clear_window
 * Clear a window's contents (inside the border).
 */
void terminal_clear_window(WINDOW* win) {
    if (win == NULL) return;
    werase(win);
}
/* }}} */

/* {{{ terminal_draw_status
 * Draw the status bar with player stats.
 *
 * Format: [Authority: 50] [D10: 3/D4: 2] [Combat: 5] [Trade: 3]
 */
void terminal_draw_status(TerminalUI* ui, int authority, int d10, int d4,
                          int combat, int trade) {
    if (ui == NULL || ui->status_win == NULL) return;

    werase(ui->status_win);

    int x = 1;

    /* Authority (cyan) */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_AUTHORITY));
    mvwprintw(ui->status_win, 0, x, "Authority: %d", authority);
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_AUTHORITY));
    x += 16;

    /* D10/D4 (default) */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(ui->status_win, 0, x, "| D10: %d D4: %d", d10, d4);
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    x += 18;

    /* Combat (magenta) */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_COMBAT));
    mvwprintw(ui->status_win, 0, x, "| Combat: %d", combat);
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_COMBAT));
    x += 14;

    /* Trade (yellow) */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_TRADE));
    mvwprintw(ui->status_win, 0, x, "| Trade: %d", trade);
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_TRADE));

    wrefresh(ui->status_win);
}
/* }}} */

/* {{{ terminal_get_faction_color
 * Get the color pair for a faction name.
 */
int terminal_get_faction_color(const char* faction) {
    if (faction == NULL) return COLOR_PAIR_NEUTRAL;

    if (strcmp(faction, "merchant") == 0) return COLOR_PAIR_MERCHANT;
    if (strcmp(faction, "wilds") == 0) return COLOR_PAIR_WILDS;
    if (strcmp(faction, "kingdom") == 0) return COLOR_PAIR_KINGDOM;
    if (strcmp(faction, "artificer") == 0) return COLOR_PAIR_ARTIFICER;

    return COLOR_PAIR_NEUTRAL;
}
/* }}} */
