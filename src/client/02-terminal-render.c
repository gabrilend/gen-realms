/* 02-terminal-render.c - Terminal rendering implementation
 *
 * Implements rendering functions for each terminal UI window. Each renderer
 * clears its window, draws borders/titles, and renders the appropriate
 * game state information using faction-appropriate colors.
 */

#include "02-terminal-render.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* {{{ Helper: get color pair for faction
 * Maps Faction enum to ncurses color pair.
 */
static int faction_color_pair(Faction faction) {
    switch (faction) {
        case FACTION_MERCHANT:  return COLOR_PAIR_MERCHANT;
        case FACTION_WILDS:     return COLOR_PAIR_WILDS;
        case FACTION_KINGDOM:   return COLOR_PAIR_KINGDOM;
        case FACTION_ARTIFICER: return COLOR_PAIR_ARTIFICER;
        default:                return COLOR_PAIR_NEUTRAL;
    }
}
/* }}} */

/* {{{ Helper: format effect for display
 * Creates a short string representation of an effect.
 */
static void format_effect(Effect* effect, char* buf, size_t buf_size) {
    switch (effect->type) {
        case EFFECT_TRADE:
            snprintf(buf, buf_size, "+%dT", effect->value);
            break;
        case EFFECT_COMBAT:
            snprintf(buf, buf_size, "+%dC", effect->value);
            break;
        case EFFECT_AUTHORITY:
            snprintf(buf, buf_size, "+%dA", effect->value);
            break;
        case EFFECT_DRAW:
            snprintf(buf, buf_size, "Draw %d", effect->value);
            break;
        case EFFECT_DISCARD:
            snprintf(buf, buf_size, "Opp --%d", effect->value);
            break;
        case EFFECT_DESTROY_BASE:
            snprintf(buf, buf_size, "Destroy");
            break;
        case EFFECT_D10_UP:
            snprintf(buf, buf_size, "D10+%d", effect->value);
            break;
        case EFFECT_D10_DOWN:
            snprintf(buf, buf_size, "D10-%d", effect->value);
            break;
        default:
            snprintf(buf, buf_size, "(%s)", effect_type_to_string(effect->type));
            break;
    }
}
/* }}} */

/* {{{ terminal_render
 * Main render function that calls all sub-renderers.
 */
void terminal_render(TerminalUI* ui, Game* game, int player_id) {
    if (ui == NULL || game == NULL) return;

    Player* player = game->players[player_id];
    Player* opponent = game->players[1 - player_id];

    terminal_render_status(ui, game, player_id);
    terminal_render_hand(ui, player);
    terminal_render_trade_row(ui, game->trade_row, player->trade);
    terminal_render_bases(ui, player, opponent);

    /* Narrative is handled separately with scroll state */
}
/* }}} */

/* {{{ terminal_render_status
 * Renders the status bar with turn info and player stats.
 * Format: Turn X | PHASE | You: Auth XX D10:X D4:X | Trade:X Combat:X | Opp: Auth XX
 */
void terminal_render_status(TerminalUI* ui, Game* game, int player_id) {
    if (ui == NULL || ui->status_win == NULL || game == NULL) return;

    Player* player = game->players[player_id];
    Player* opponent = game->players[1 - player_id];

    werase(ui->status_win);

    int x = 1;

    /* Turn number */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(ui->status_win, 0, x, "Turn %d", game->turn_number);
    x += 8;

    /* Phase */
    const char* phase_str = game_phase_to_string(game->phase);
    mvwprintw(ui->status_win, 0, x, "| %s", phase_str);
    x += strlen(phase_str) + 3;
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    /* Player authority */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_AUTHORITY));
    mvwprintw(ui->status_win, 0, x, "| You: %d", player->authority);
    x += 12;
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_AUTHORITY));

    /* D10/D4 */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(ui->status_win, 0, x, "D10:%d D4:%d", player->d10, player->d4);
    x += 14;
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    /* Current trade */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_TRADE));
    mvwprintw(ui->status_win, 0, x, "| T:%d", player->trade);
    x += 7;
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_TRADE));

    /* Current combat */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_COMBAT));
    mvwprintw(ui->status_win, 0, x, "C:%d", player->combat);
    x += 6;
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_COMBAT));

    /* Opponent authority */
    wattron(ui->status_win, COLOR_PAIR(COLOR_PAIR_ERROR));
    mvwprintw(ui->status_win, 0, x, "| Opp: %d", opponent->authority);
    wattroff(ui->status_win, COLOR_PAIR(COLOR_PAIR_ERROR));

    wrefresh(ui->status_win);
}
/* }}} */

/* {{{ terminal_render_hand
 * Renders the player's hand with numbered card list.
 */
void terminal_render_hand(TerminalUI* ui, Player* player) {
    if (ui == NULL || ui->hand_win == NULL || player == NULL) return;

    werase(ui->hand_win);
    box(ui->hand_win, 0, 0);

    /* Title */
    mvwprintw(ui->hand_win, 0, 2, " YOUR HAND (%d) ", player->deck->hand_count);

    int max_y = getmaxy(ui->hand_win) - 3;

    /* Render each card in hand */
    for (int i = 0; i < player->deck->hand_count && i < max_y; i++) {
        CardInstance* card = player->deck->hand[i];
        terminal_render_card(ui->hand_win, card, i + 1, 1, i, false);
    }

    /* Show if there are more cards than fit */
    if (player->deck->hand_count > max_y) {
        wattron(ui->hand_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(ui->hand_win, max_y + 1, 1, "... +%d more",
                  player->deck->hand_count - max_y);
        wattroff(ui->hand_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    }

    /* Show played cards count */
    int played = player->deck->played_count;
    if (played > 0) {
        wattron(ui->hand_win, COLOR_PAIR(COLOR_PAIR_SUCCESS));
        mvwprintw(ui->hand_win, getmaxy(ui->hand_win) - 2, 1,
                  "Played: %d cards", played);
        wattroff(ui->hand_win, COLOR_PAIR(COLOR_PAIR_SUCCESS));
    }

    wrefresh(ui->hand_win);
}
/* }}} */

/* {{{ terminal_render_trade_row
 * Renders the trade row with purchasable cards.
 */
void terminal_render_trade_row(TerminalUI* ui, TradeRow* row, int player_trade) {
    if (ui == NULL || ui->trade_win == NULL || row == NULL) return;

    werase(ui->trade_win);
    box(ui->trade_win, 0, 0);

    /* Title */
    mvwprintw(ui->trade_win, 0, 2, " TRADE ROW ");

    int max_y = getmaxy(ui->trade_win) - 3;

    /* Render each slot */
    for (int i = 0; i < TRADE_ROW_SLOTS && i < max_y; i++) {
        CardInstance* card = row->slots[i];

        if (card == NULL) {
            wattron(ui->trade_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            mvwprintw(ui->trade_win, i + 1, 1, "[%d] (empty)", i);
            wattroff(ui->trade_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            continue;
        }

        int cost = card->type->cost;
        bool can_afford = (player_trade >= cost);

        int color = faction_color_pair(card->type->faction);
        if (!can_afford) {
            color = COLOR_PAIR_DEFAULT;  /* Dim if can't afford */
        }

        wattron(ui->trade_win, COLOR_PAIR(color));
        mvwprintw(ui->trade_win, i + 1, 1, "[%d] %-18s %2dg",
                  i, card->type->name, cost);
        wattroff(ui->trade_win, COLOR_PAIR(color));

        /* Show effect summary */
        char effect_buf[32];
        if (card->type->effect_count > 0) {
            format_effect(&card->type->effects[0], effect_buf, sizeof(effect_buf));
            mvwprintw(ui->trade_win, i + 1, 28, "%s", effect_buf);
        }
    }

    /* Explorer always available */
    int exp_y = TRADE_ROW_SLOTS + 1;
    if (exp_y < max_y) {
        bool can_afford_exp = (player_trade >= EXPLORER_COST);
        int exp_color = can_afford_exp ? COLOR_PAIR_NEUTRAL : COLOR_PAIR_DEFAULT;

        wattron(ui->trade_win, COLOR_PAIR(exp_color));
        mvwprintw(ui->trade_win, exp_y, 1, "[E] Explorer            %dg  +2T",
                  EXPLORER_COST);
        wattroff(ui->trade_win, COLOR_PAIR(exp_color));
    }

    /* Deck count */
    wattron(ui->trade_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(ui->trade_win, getmaxy(ui->trade_win) - 2, 1,
              "Deck: %d remaining", trade_row_deck_remaining(row));
    wattroff(ui->trade_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    wrefresh(ui->trade_win);
}
/* }}} */

/* {{{ terminal_render_bases
 * Renders bases for both players.
 */
void terminal_render_bases(TerminalUI* ui, Player* player, Player* opponent) {
    if (ui == NULL || ui->base_win == NULL) return;

    werase(ui->base_win);
    box(ui->base_win, 0, 0);

    int h = getmaxy(ui->base_win);
    int mid = h / 2;

    /* Your bases */
    mvwprintw(ui->base_win, 0, 2, " YOUR BASES (%d) ", player->deck->base_count);

    for (int i = 0; i < player->deck->base_count && i + 1 < mid; i++) {
        CardInstance* base = player->deck->bases[i];
        int color = faction_color_pair(base->type->faction);

        wattron(ui->base_win, COLOR_PAIR(color));
        mvwprintw(ui->base_win, i + 1, 1, "[%d] %s (%d def%s)",
                  i, base->type->name, base->type->defense,
                  base->type->is_outpost ? " OUT" : "");
        wattroff(ui->base_win, COLOR_PAIR(color));
    }

    /* Divider */
    wattron(ui->base_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwhline(ui->base_win, mid, 1, '-', getmaxx(ui->base_win) - 2);
    wattroff(ui->base_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    /* Opponent bases */
    wattron(ui->base_win, COLOR_PAIR(COLOR_PAIR_ERROR));
    mvwprintw(ui->base_win, mid, 2, " OPPONENT BASES (%d) ", opponent->deck->base_count);
    wattroff(ui->base_win, COLOR_PAIR(COLOR_PAIR_ERROR));

    for (int i = 0; i < opponent->deck->base_count && mid + i + 1 < h - 1; i++) {
        CardInstance* base = opponent->deck->bases[i];
        int color = faction_color_pair(base->type->faction);

        wattron(ui->base_win, COLOR_PAIR(color));
        mvwprintw(ui->base_win, mid + i + 1, 1, "[%d] %s (%d def%s)",
                  i, base->type->name, base->type->defense,
                  base->type->is_outpost ? " OUT" : "");
        wattroff(ui->base_win, COLOR_PAIR(color));
    }

    wrefresh(ui->base_win);
}
/* }}} */

/* {{{ terminal_render_played
 * Renders played cards for current turn (useful for showing what's been done).
 */
void terminal_render_played(TerminalUI* ui, Player* player) {
    if (ui == NULL || ui->hand_win == NULL || player == NULL) return;

    /* This could be shown in a separate area or as part of hand window */
    /* For now, played cards are shown in hand window footer */
}
/* }}} */

/* {{{ terminal_render_narrative
 * Renders narrative text with scrolling support.
 */
void terminal_render_narrative(TerminalUI* ui, const char** lines, int line_count,
                               int scroll_offset) {
    if (ui == NULL || ui->narrative_win == NULL) return;

    werase(ui->narrative_win);
    box(ui->narrative_win, 0, 0);
    mvwprintw(ui->narrative_win, 0, 2, " NARRATIVE ");

    int win_h = getmaxy(ui->narrative_win) - 2;
    int win_w = getmaxx(ui->narrative_win) - 2;

    /* Adjust scroll to valid range */
    int max_scroll = line_count - win_h;
    if (max_scroll < 0) max_scroll = 0;
    if (scroll_offset > max_scroll) scroll_offset = max_scroll;
    if (scroll_offset < 0) scroll_offset = 0;

    /* Render visible lines */
    wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    for (int i = 0; i < win_h && scroll_offset + i < line_count; i++) {
        const char* line = lines[scroll_offset + i];
        mvwaddnstr(ui->narrative_win, i + 1, 1, line, win_w);
    }
    wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    /* Scroll indicators */
    if (scroll_offset > 0) {
        mvwprintw(ui->narrative_win, 1, win_w - 1, "^");
    }
    if (scroll_offset < max_scroll) {
        mvwprintw(ui->narrative_win, win_h, win_w - 1, "v");
    }

    wrefresh(ui->narrative_win);
}
/* }}} */

/* {{{ terminal_render_input
 * Renders input prompt at bottom of screen.
 */
void terminal_render_input(TerminalUI* ui, const char* prompt) {
    if (ui == NULL || ui->input_win == NULL) return;

    werase(ui->input_win);

    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_REVERSE);
    mvwprintw(ui->input_win, 0, 0, " %s", prompt ? prompt : ">");
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_REVERSE);

    wrefresh(ui->input_win);
}
/* }}} */

/* {{{ terminal_render_card
 * Renders a single card line with index and faction color.
 */
void terminal_render_card(WINDOW* win, CardInstance* card, int y, int x,
                          int index, bool selected) {
    if (win == NULL || card == NULL) return;

    int color = faction_color_pair(card->type->faction);

    if (selected) {
        wattron(win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_REVERSE);
    } else {
        wattron(win, COLOR_PAIR(color));
    }

    /* Format: [N] CardName (+bonuses) */
    char bonus_str[32] = "";
    if (card->attack_bonus > 0 || card->trade_bonus > 0) {
        snprintf(bonus_str, sizeof(bonus_str), " (+%dC +%dT)",
                 card->attack_bonus, card->trade_bonus);
    }

    mvwprintw(win, y, x, "[%d] %s%s", index, card->type->name, bonus_str);

    if (selected) {
        wattroff(win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_REVERSE);
    } else {
        wattroff(win, COLOR_PAIR(color));
    }
}
/* }}} */

/* {{{ terminal_render_card_list
 * Renders a compact card list for small windows.
 */
void terminal_render_card_list(WINDOW* win, CardInstance** cards, int count,
                               int start_y, int max_lines) {
    if (win == NULL || cards == NULL) return;

    for (int i = 0; i < count && i < max_lines; i++) {
        terminal_render_card(win, cards[i], start_y + i, 1, i, false);
    }

    if (count > max_lines) {
        wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
        mvwprintw(win, start_y + max_lines, 1, "... +%d more", count - max_lines);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    }
}
/* }}} */

/* {{{ terminal_render_effects
 * Renders effect summary for a card.
 */
void terminal_render_effects(WINDOW* win, CardInstance* card, int y, int x) {
    if (win == NULL || card == NULL) return;

    char buf[64] = "";
    char effect_str[16];
    int offset = 0;

    for (int i = 0; i < card->type->effect_count && offset < 60; i++) {
        format_effect(&card->type->effects[i], effect_str, sizeof(effect_str));
        int len = strlen(effect_str);
        if (offset + len + 1 < 60) {
            if (offset > 0) buf[offset++] = ' ';
            strcpy(buf + offset, effect_str);
            offset += len;
        }
    }

    wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(win, y, x, "%s", buf);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
}
/* }}} */

/* {{{ Narrative buffer functions */

NarrativeBuffer* narrative_buffer_create(void) {
    NarrativeBuffer* buf = malloc(sizeof(NarrativeBuffer));
    if (buf == NULL) return NULL;

    buf->line_count = 0;
    buf->scroll_offset = 0;
    return buf;
}

void narrative_buffer_free(NarrativeBuffer* buf) {
    if (buf != NULL) {
        free(buf);
    }
}

void narrative_buffer_add(NarrativeBuffer* buf, const char* text) {
    if (buf == NULL || text == NULL) return;

    if (buf->line_count >= NARRATIVE_MAX_LINES) {
        /* Shift lines up, dropping oldest */
        memmove(buf->lines[0], buf->lines[1],
                (NARRATIVE_MAX_LINES - 1) * NARRATIVE_LINE_MAX);
        buf->line_count = NARRATIVE_MAX_LINES - 1;
    }

    strncpy(buf->lines[buf->line_count], text, NARRATIVE_LINE_MAX - 1);
    buf->lines[buf->line_count][NARRATIVE_LINE_MAX - 1] = '\0';
    buf->line_count++;
}

void narrative_buffer_clear(NarrativeBuffer* buf) {
    if (buf == NULL) return;
    buf->line_count = 0;
    buf->scroll_offset = 0;
}

void narrative_buffer_scroll_up(NarrativeBuffer* buf, int lines) {
    if (buf == NULL) return;
    buf->scroll_offset -= lines;
    if (buf->scroll_offset < 0) buf->scroll_offset = 0;
}

void narrative_buffer_scroll_down(NarrativeBuffer* buf, int lines) {
    if (buf == NULL) return;
    buf->scroll_offset += lines;
    /* Upper bound checked in render function */
}

/* }}} */
