/*
 * panel-renderer.c - Panel Rendering Implementation for WASM Client
 *
 * Renders the status bar with player stats and action buttons,
 * and the narrative panel with scrolling LLM-generated text.
 */

#include "panel-renderer.h"
#include "draw2d.h"
#include "theme.h"
#include <stdio.h>
#include <string.h>

/* Button dimensions */
#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT 28
#define BUTTON_SPACING 8

/* Stat block dimensions */
#define STAT_BLOCK_WIDTH 200
#define STAT_ICON_SIZE 16

/* {{{ render_stat_value
 * Render a single stat with icon and value.
 */
static void render_stat_value(int x, int y, const char* icon, int value,
                              uint32_t color, bool show_sign) {
    /* Icon/label */
    draw_text_ex(x, y, icon, color, FONT_SIZE_SMALL,
                 TEXT_ALIGN_LEFT, TEXT_BASELINE_MIDDLE);

    /* Value */
    char buf[16];
    if (show_sign && value > 0) {
        snprintf(buf, sizeof(buf), "+%d", value);
    } else {
        snprintf(buf, sizeof(buf), "%d", value);
    }

    int icon_w = draw_measure_text(icon, FONT_SIZE_SMALL);
    draw_text_ex(x + icon_w + 4, y, buf, THEME_TEXT_PRIMARY, FONT_SIZE_NORMAL,
                 TEXT_ALIGN_LEFT, TEXT_BASELINE_MIDDLE);
}
/* }}} */

/* {{{ render_button
 * Render a single action button.
 */
static void render_button(int x, int y, int w, int h, const ActionButton* btn) {
    uint32_t bg_color, border_color, text_color;

    if (!btn->enabled) {
        bg_color = THEME_BG_DARK;
        border_color = THEME_BORDER_DIM;
        text_color = THEME_TEXT_DIM;
    } else if (btn->pressed) {
        bg_color = THEME_BG_HIGHLIGHT;
        border_color = THEME_TEXT_PRIMARY;
        text_color = THEME_TEXT_PRIMARY;
    } else if (btn->hovered) {
        bg_color = THEME_BG_CARD;
        border_color = THEME_BORDER_BRIGHT;
        text_color = THEME_TEXT_PRIMARY;
    } else {
        bg_color = THEME_BG_PANEL;
        border_color = THEME_BORDER_NORMAL;
        text_color = THEME_TEXT_SECONDARY;
    }

    /* Button background */
    draw_rounded_rect(x, y, w, h, 4, bg_color);
    draw_rounded_rect_outline(x, y, w, h, 4, border_color, btn->hovered ? 2 : 1);

    /* Button label */
    draw_text_ex(x + w/2, y + h/2, btn->label, text_color, FONT_SIZE_NORMAL,
                 TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);

    /* Hotkey hint (bottom right) */
    if (btn->hotkey && btn->enabled) {
        draw_text_ex(x + w - 4, y + h - 2, btn->hotkey, THEME_TEXT_DIM,
                     FONT_SIZE_SMALL, TEXT_ALIGN_RIGHT, TEXT_BASELINE_BOTTOM);
    }
}
/* }}} */

/* {{{ panel_render_player_stats */
void panel_render_player_stats(int x, int y, int w, const PlayerStats* stats, bool is_left) {
    if (!stats) return;

    int text_y = y + STATUS_BAR_HEIGHT / 2;

    /* Player name with current turn indicator */
    uint32_t name_color = stats->is_current_player ? THEME_STATUS_SUCCESS : THEME_TEXT_SECONDARY;

    if (is_left) {
        /* Left-aligned (player) */
        int name_x = x + PADDING_SMALL;

        /* Turn indicator */
        if (stats->is_current_player) {
            draw_circle(name_x + 4, text_y, 4, THEME_STATUS_SUCCESS);
            name_x += 12;
        }

        draw_text_ex(name_x, text_y, stats->name, name_color, FONT_SIZE_NORMAL,
                     TEXT_ALIGN_LEFT, TEXT_BASELINE_MIDDLE);

        /* Stats row below name */
        int stat_x = name_x;
        int stat_y = text_y + 12;

        render_stat_value(stat_x, stat_y, "♥", stats->authority, THEME_VALUE_AUTHORITY, false);
        stat_x += 50;
        render_stat_value(stat_x, stat_y, "$", stats->trade, THEME_VALUE_TRADE, false);
        stat_x += 45;
        render_stat_value(stat_x, stat_y, "⚔", stats->combat, THEME_VALUE_COMBAT, false);
    } else {
        /* Right-aligned (opponent) */
        int name_x = x + w - PADDING_SMALL;

        draw_text_ex(name_x, text_y, stats->name, name_color, FONT_SIZE_NORMAL,
                     TEXT_ALIGN_RIGHT, TEXT_BASELINE_MIDDLE);

        /* Turn indicator */
        if (stats->is_current_player) {
            int name_w = draw_measure_text(stats->name, FONT_SIZE_NORMAL);
            draw_circle(name_x - name_w - 8, text_y, 4, THEME_STATUS_SUCCESS);
        }

        /* Stats row below name */
        int stat_x = name_x - 50;
        int stat_y = text_y + 12;

        render_stat_value(stat_x, stat_y, "⚔", stats->combat, THEME_VALUE_COMBAT, false);
        stat_x -= 45;
        render_stat_value(stat_x, stat_y, "$", stats->trade, THEME_VALUE_TRADE, false);
        stat_x -= 50;
        render_stat_value(stat_x, stat_y, "♥", stats->authority, THEME_VALUE_AUTHORITY, false);
    }

    /* Deck/discard counts - compact format */
    int deck_y = y + 8;
    char deck_str[32];
    snprintf(deck_str, sizeof(deck_str), "D:%d H:%d X:%d",
             stats->deck_count, stats->hand_count, stats->discard_count);

    if (is_left) {
        draw_text_ex(x + PADDING_SMALL, deck_y, deck_str, THEME_TEXT_DIM,
                     FONT_SIZE_SMALL, TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);
    } else {
        draw_text_ex(x + w - PADDING_SMALL, deck_y, deck_str, THEME_TEXT_DIM,
                     FONT_SIZE_SMALL, TEXT_ALIGN_RIGHT, TEXT_BASELINE_TOP);
    }
}
/* }}} */

/* {{{ panel_render_action_buttons */
void panel_render_action_buttons(int x, int y, int w, int h,
                                 ActionButton* buttons, int count) {
    if (!buttons || count <= 0) return;

    /* Calculate total buttons width */
    int total_width = count * BUTTON_WIDTH + (count - 1) * BUTTON_SPACING;

    /* Center buttons */
    int start_x = x + (w - total_width) / 2;
    int btn_y = y + (h - BUTTON_HEIGHT) / 2;

    for (int i = 0; i < count; i++) {
        int btn_x = start_x + i * (BUTTON_WIDTH + BUTTON_SPACING);
        render_button(btn_x, btn_y, BUTTON_WIDTH, BUTTON_HEIGHT, &buttons[i]);
    }
}
/* }}} */

/* {{{ panel_render_status_bar */
void panel_render_status_bar(const Zone* zone, const StatusBarData* data) {
    if (!zone || !data) return;

    /* Background */
    draw_rect(zone->x, zone->y, zone->w, zone->h, THEME_BG_PANEL);
    draw_line(zone->x, zone->y + zone->h, zone->x + zone->w, zone->y + zone->h,
              THEME_BORDER_NORMAL, 1);

    /* Player stats (left side) */
    panel_render_player_stats(zone->x, zone->y, STAT_BLOCK_WIDTH, &data->player, true);

    /* Opponent stats (right side) */
    panel_render_player_stats(zone->x + zone->w - STAT_BLOCK_WIDTH, zone->y,
                              STAT_BLOCK_WIDTH, &data->opponent, false);

    /* Turn info (center top) */
    int center_x = zone->x + zone->w / 2;
    int turn_y = zone->y + 8;

    char turn_str[32];
    snprintf(turn_str, sizeof(turn_str), "Turn %d", data->turn.turn_number);
    draw_text_ex(center_x, turn_y, turn_str, THEME_TEXT_DIM, FONT_SIZE_SMALL,
                 TEXT_ALIGN_CENTER, TEXT_BASELINE_TOP);

    /* Phase indicator */
    if (data->turn.phase_name) {
        uint32_t phase_color = data->turn.is_player_turn ? THEME_STATUS_SUCCESS : THEME_STATUS_WARNING;
        draw_text_ex(center_x, turn_y + 14, data->turn.phase_name, phase_color,
                     FONT_SIZE_NORMAL, TEXT_ALIGN_CENTER, TEXT_BASELINE_TOP);
    }

    /* Action buttons (center bottom) */
    int btn_area_x = zone->x + STAT_BLOCK_WIDTH;
    int btn_area_w = zone->w - STAT_BLOCK_WIDTH * 2;
    int btn_area_y = zone->y + zone->h / 2;
    int btn_area_h = zone->h / 2;

    panel_render_action_buttons(btn_area_x, btn_area_y, btn_area_w, btn_area_h,
                                (ActionButton*)data->buttons, data->button_count);
}
/* }}} */

/* {{{ panel_render_narrative */
void panel_render_narrative(const Zone* zone, const NarrativeData* data) {
    if (!zone || !data) return;

    /* Panel background */
    draw_rounded_rect(zone->x, zone->y, zone->w, zone->h,
                      PANEL_CORNER_RADIUS, THEME_BG_PANEL);
    draw_rounded_rect_outline(zone->x, zone->y, zone->w, zone->h,
                              PANEL_CORNER_RADIUS, THEME_BORDER_NORMAL, 1);

    /* Header */
    draw_rect(zone->x + 1, zone->y + 1, zone->w - 2, 24, THEME_BG_DARK);
    draw_text_ex(zone->x + zone->w / 2, zone->y + 12, "Narrative",
                 THEME_TEXT_SECONDARY, FONT_SIZE_SMALL,
                 TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);

    /* Content area */
    int content_x = zone->x + PADDING_NORMAL;
    int content_y = zone->y + 30;
    int content_w = zone->w - PADDING_NORMAL * 2;
    int content_h = zone->h - 35;

    /* Line height */
    int line_h = FONT_SIZE_NORMAL + 4;
    int max_visible = content_h / line_h;

    /* Calculate visible range with scroll */
    int start_line = data->scroll_offset;
    int visible_count = data->line_count - start_line;
    if (visible_count > max_visible) visible_count = max_visible;

    /* Render lines from bottom up (most recent at bottom) */
    for (int i = 0; i < visible_count && i < MAX_NARRATIVE_LINES; i++) {
        int line_idx = start_line + i;
        if (line_idx >= data->line_count) break;

        const NarrativeLine* line = &data->lines[line_idx];
        int line_y = content_y + i * line_h;

        int font_size = line->is_heading ? FONT_SIZE_LARGE : FONT_SIZE_NORMAL;
        uint32_t color = line->color ? line->color : THEME_TEXT_SECONDARY;

        /* Action lines get a different style */
        if (line->is_action) {
            draw_text_ex(content_x, line_y, ">", THEME_TEXT_DIM, font_size,
                         TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);
            draw_text_ex(content_x + 12, line_y, line->text, color, font_size,
                         TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);
        } else {
            draw_text_ex(content_x, line_y, line->text, color, font_size,
                         TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);
        }
    }

    /* Streaming indicator */
    if (data->is_streaming) {
        int cursor_y = content_y + visible_count * line_h;

        /* Partial line being streamed */
        if (data->streaming_partial) {
            draw_text_ex(content_x, cursor_y, data->streaming_partial,
                         THEME_TEXT_SECONDARY, FONT_SIZE_NORMAL,
                         TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);
            int partial_w = draw_measure_text(data->streaming_partial, FONT_SIZE_NORMAL);
            /* Blinking cursor effect (simple static for now) */
            draw_rect(content_x + partial_w + 2, cursor_y, 8, FONT_SIZE_NORMAL, THEME_TEXT_PRIMARY);
        } else {
            /* Just show typing indicator */
            draw_text_ex(content_x, cursor_y, "...", THEME_TEXT_DIM, FONT_SIZE_NORMAL,
                         TEXT_ALIGN_LEFT, TEXT_BASELINE_TOP);
        }
    }

    /* Scroll indicators */
    if (data->scroll_offset > 0) {
        /* More content above */
        draw_text_ex(zone->x + zone->w - PADDING_SMALL, zone->y + 30,
                     "▲", THEME_TEXT_DIM, FONT_SIZE_SMALL,
                     TEXT_ALIGN_RIGHT, TEXT_BASELINE_TOP);
    }

    if (start_line + visible_count < data->line_count) {
        /* More content below */
        draw_text_ex(zone->x + zone->w - PADDING_SMALL, zone->y + zone->h - PADDING_SMALL,
                     "▼", THEME_TEXT_DIM, FONT_SIZE_SMALL,
                     TEXT_ALIGN_RIGHT, TEXT_BASELINE_BOTTOM);
    }
}
/* }}} */

/* {{{ panel_hit_test_button */
int panel_hit_test_button(int x, int y, const Zone* zone, const StatusBarData* data) {
    if (!zone || !data) return -1;

    /* Button area bounds */
    int btn_area_x = zone->x + STAT_BLOCK_WIDTH;
    int btn_area_w = zone->w - STAT_BLOCK_WIDTH * 2;

    /* Calculate button positions */
    int total_width = data->button_count * BUTTON_WIDTH +
                      (data->button_count - 1) * BUTTON_SPACING;
    int start_x = btn_area_x + (btn_area_w - total_width) / 2;
    int btn_y = zone->y + (zone->h - BUTTON_HEIGHT) / 2;

    for (int i = 0; i < data->button_count; i++) {
        int bx = start_x + i * (BUTTON_WIDTH + BUTTON_SPACING);

        if (x >= bx && x < bx + BUTTON_WIDTH &&
            y >= btn_y && y < btn_y + BUTTON_HEIGHT) {
            return i;
        }
    }

    return -1;
}
/* }}} */

/* {{{ panel_handle_narrative_scroll */
void panel_handle_narrative_scroll(int wheel_delta, const Zone* zone, NarrativeData* data) {
    if (!zone || !data || wheel_delta == 0) return;

    /* Calculate max scroll */
    int line_h = FONT_SIZE_NORMAL + 4;
    int content_h = zone->h - 35;
    int max_visible = content_h / line_h;

    int max_scroll = data->line_count - max_visible;
    if (max_scroll < 0) max_scroll = 0;

    /* Apply scroll */
    data->scroll_offset -= wheel_delta;

    /* Clamp */
    if (data->scroll_offset < 0) data->scroll_offset = 0;
    if (data->scroll_offset > max_scroll) data->scroll_offset = max_scroll;
}
/* }}} */
