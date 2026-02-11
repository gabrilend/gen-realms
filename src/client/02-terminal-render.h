/* 02-terminal-render.h - Terminal rendering function declarations
 *
 * Provides rendering functions for each terminal UI window, displaying
 * game state information using the structures defined in the core modules.
 */

#ifndef TERMINAL_RENDER_H
#define TERMINAL_RENDER_H

#include "01-terminal.h"
#include "../core/05-game.h"

/* {{{ Main render function */

/* Render all windows for a given game state and player perspective */
void terminal_render(TerminalUI* ui, Game* game, int player_id);

/* }}} */

/* {{{ Individual window renderers */

/* Render status bar with player stats and turn info */
void terminal_render_status(TerminalUI* ui, Game* game, int player_id);

/* Render player's hand with card list */
void terminal_render_hand(TerminalUI* ui, Player* player);

/* Render trade row with purchasable cards */
void terminal_render_trade_row(TerminalUI* ui, TradeRow* row, int player_trade);

/* Render bases for both players */
void terminal_render_bases(TerminalUI* ui, Player* player, Player* opponent);

/* Render played cards for current turn */
void terminal_render_played(TerminalUI* ui, Player* player);

/* Render narrative text with scrolling */
void terminal_render_narrative(TerminalUI* ui, const char** lines, int line_count,
                               int scroll_offset);

/* Render input prompt */
void terminal_render_input(TerminalUI* ui, const char* prompt);

/* }}} */

/* {{{ Utility renderers */

/* Render a single card in a window at given position */
void terminal_render_card(WINDOW* win, CardInstance* card, int y, int x,
                          int index, bool selected);

/* Render a compact card list (for small windows) */
void terminal_render_card_list(WINDOW* win, CardInstance** cards, int count,
                               int start_y, int max_lines);

/* Render effect summary string */
void terminal_render_effects(WINDOW* win, CardInstance* card, int y, int x);

/* }}} */

/* {{{ Narrative buffer management */

/* Maximum lines in narrative buffer */
#define NARRATIVE_MAX_LINES 100
#define NARRATIVE_LINE_MAX 256

typedef struct {
    char lines[NARRATIVE_MAX_LINES][NARRATIVE_LINE_MAX];
    int line_count;
    int scroll_offset;
} NarrativeBuffer;

NarrativeBuffer* narrative_buffer_create(void);
void narrative_buffer_free(NarrativeBuffer* buf);
void narrative_buffer_add(NarrativeBuffer* buf, const char* text);
void narrative_buffer_clear(NarrativeBuffer* buf);
void narrative_buffer_scroll_up(NarrativeBuffer* buf, int lines);
void narrative_buffer_scroll_down(NarrativeBuffer* buf, int lines);

/* }}} */

#endif /* TERMINAL_RENDER_H */
