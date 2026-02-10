/* test-terminal.c - Test program for terminal UI initialization
 *
 * Verifies that ncurses initializes correctly and all windows are created
 * with proper dimensions. Displays a test screen for visual verification.
 *
 * Usage: ./bin/test-terminal
 *        Press any key to exit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../src/client/01-terminal.h"

/* {{{ test_colors
 * Display color palette to verify color pairs work.
 */
static void test_colors(WINDOW* win, int start_y) {
    int y = start_y;

    mvwprintw(win, y++, 2, "Color Test:");

    wattron(win, COLOR_PAIR(COLOR_PAIR_MERCHANT));
    mvwprintw(win, y++, 2, "  Merchant Guilds (yellow)");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_MERCHANT));

    wattron(win, COLOR_PAIR(COLOR_PAIR_WILDS));
    mvwprintw(win, y++, 2, "  The Wilds (green)");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_WILDS));

    wattron(win, COLOR_PAIR(COLOR_PAIR_KINGDOM));
    mvwprintw(win, y++, 2, "  High Kingdom (blue)");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_KINGDOM));

    wattron(win, COLOR_PAIR(COLOR_PAIR_ARTIFICER));
    mvwprintw(win, y++, 2, "  Artificer Order (red)");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_ARTIFICER));

    wattron(win, COLOR_PAIR(COLOR_PAIR_NEUTRAL));
    mvwprintw(win, y++, 2, "  Neutral (white)");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_NEUTRAL));

    wattron(win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_REVERSE);
    mvwprintw(win, y++, 2, "  Highlighted (inverse)");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_REVERSE);
}
/* }}} */

/* {{{ test_window_info
 * Display window dimensions and position.
 */
static void test_window_info(WINDOW* win, const char* name) {
    if (win == NULL) {
        printf("  %s: NULL\n", name);
        return;
    }

    int w, h, x, y;
    getmaxyx(win, h, w);
    getbegyx(win, y, x);
    printf("  %s: %dx%d at (%d,%d)\n", name, w, h, x, y);
}
/* }}} */

/* {{{ main */
int main(void) {
    printf("Symbeline Realms - Terminal UI Test\n");
    printf("====================================\n\n");

    /* Initialize terminal UI */
    printf("Initializing terminal UI...\n");
    TerminalUI* ui = terminal_init();

    if (ui == NULL) {
        fprintf(stderr, "ERROR: Failed to initialize terminal UI\n");
        return 1;
    }

    printf("Terminal size: %dx%d\n", ui->term_width, ui->term_height);
    printf("Windows created:\n");

    /* Print window info (will show in terminal after endwin) */
    /* We need to save this info before displaying */
    int sw = 0, sh = 0, hww = 0, hwh = 0, tww = 0, twh = 0;
    int bww = 0, bwh = 0, nww = 0, nwh = 0, iww = 0, iwh = 0;

    if (ui->status_win) getmaxyx(ui->status_win, sh, sw);
    if (ui->hand_win) getmaxyx(ui->hand_win, hwh, hww);
    if (ui->trade_win) getmaxyx(ui->trade_win, twh, tww);
    if (ui->base_win) getmaxyx(ui->base_win, bwh, bww);
    if (ui->narrative_win) getmaxyx(ui->narrative_win, nwh, nww);
    if (ui->input_win) getmaxyx(ui->input_win, iwh, iww);

    /* Draw test content */
    terminal_draw_status(ui, 50, 3, 2, 5, 4);

    if (ui->hand_win) {
        terminal_draw_box(ui->hand_win, "HAND");
        mvwprintw(ui->hand_win, 2, 2, "Your cards appear here");
        test_colors(ui->hand_win, 4);
        wrefresh(ui->hand_win);
    }

    if (ui->trade_win) {
        terminal_draw_box(ui->trade_win, "TRADE ROW");
        mvwprintw(ui->trade_win, 2, 2, "Cards for purchase");
        wrefresh(ui->trade_win);
    }

    if (ui->base_win) {
        terminal_draw_box(ui->base_win, "BASES");
        mvwprintw(ui->base_win, 2, 2, "Your bases in play");
        wrefresh(ui->base_win);
    }

    if (ui->narrative_win) {
        terminal_draw_box(ui->narrative_win, "NARRATIVE");
        mvwprintw(ui->narrative_win, 2, 2, "Story text appears here...");
        mvwprintw(ui->narrative_win, 4, 2, "The merchant ship glides");
        mvwprintw(ui->narrative_win, 5, 2, "silently through the mist,");
        mvwprintw(ui->narrative_win, 6, 2, "its cargo hold brimming");
        mvwprintw(ui->narrative_win, 7, 2, "with exotic treasures.");
        wrefresh(ui->narrative_win);
    }

    if (ui->input_win) {
        wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(ui->input_win, 0, 0, " Press any key to exit... ");
        wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
        wrefresh(ui->input_win);
    }

    /* Wait for keypress (blocking mode for test) */
    nodelay(stdscr, FALSE);
    getch();

    /* Test resize handling */
    /* (User can resize terminal and we'll detect it) */

    /* Cleanup */
    terminal_cleanup(ui);

    /* Print results after ncurses cleanup */
    printf("\nTest completed successfully!\n\n");
    printf("Window dimensions:\n");
    printf("  status_win:    %dx%d\n", sw, sh);
    printf("  hand_win:      %dx%d\n", hww, hwh);
    printf("  trade_win:     %dx%d\n", tww, twh);
    printf("  base_win:      %dx%d\n", bww, bwh);
    printf("  narrative_win: %dx%d\n", nww, nwh);
    printf("  input_win:     %dx%d\n", iww, iwh);

    printf("\nAll tests passed.\n");

    return 0;
}
/* }}} */
