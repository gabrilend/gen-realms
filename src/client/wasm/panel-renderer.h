/*
 * panel-renderer.h - Panel Rendering for WASM Client
 *
 * Renders UI panels: status bar and narrative panel.
 * Status bar shows player stats, turn info, and action buttons.
 * Narrative panel shows LLM-generated story text.
 */

#ifndef WASM_PANEL_RENDERER_H
#define WASM_PANEL_RENDERER_H

#include <stdbool.h>
#include <stdint.h>
#include "theme.h"

/* {{{ Maximum text lengths */
#define MAX_PLAYER_NAME 32
#define MAX_NARRATIVE_LINES 20
#define MAX_NARRATIVE_LINE_LEN 80
/* }}} */

/* {{{ PlayerStats
 * Player statistics for status bar display.
 */
typedef struct {
    char name[MAX_PLAYER_NAME];
    int authority;
    int trade;
    int combat;
    int deck_count;
    int discard_count;
    int hand_count;
    bool is_current_player;
} PlayerStats;
/* }}} */

/* {{{ TurnInfo
 * Turn state information.
 */
typedef struct {
    int turn_number;
    bool is_player_turn;
    bool can_end_turn;
    bool waiting_for_action;
    const char* phase_name;          /* "Main", "Combat", "Buy", etc. */
} TurnInfo;
/* }}} */

/* {{{ ActionButton
 * Button in the action bar.
 */
typedef enum {
    BTN_END_TURN,
    BTN_ATTACK,
    BTN_BUY,
    BTN_SCRAP,
    BTN_ALLY,
    BTN_COUNT
} ActionButtonType;

typedef struct {
    ActionButtonType type;
    bool enabled;
    bool hovered;
    bool pressed;
    const char* label;
    const char* hotkey;              /* Keyboard shortcut display */
} ActionButton;
/* }}} */

/* {{{ StatusBarData
 * All data for status bar rendering.
 */
typedef struct {
    PlayerStats player;
    PlayerStats opponent;
    TurnInfo turn;
    ActionButton buttons[BTN_COUNT];
    int button_count;
} StatusBarData;
/* }}} */

/* {{{ NarrativeLine
 * A line in the narrative panel.
 */
typedef struct {
    char text[MAX_NARRATIVE_LINE_LEN];
    uint32_t color;                  /* Text color (faction, emphasis, etc.) */
    bool is_heading;                 /* Larger font for headings */
    bool is_action;                  /* Player action vs narrative */
} NarrativeLine;
/* }}} */

/* {{{ NarrativeData
 * All data for narrative panel rendering.
 */
typedef struct {
    NarrativeLine lines[MAX_NARRATIVE_LINES];
    int line_count;
    int scroll_offset;               /* Lines scrolled from bottom */
    bool is_streaming;               /* Currently receiving text */
    const char* streaming_partial;   /* Partial line being streamed */
} NarrativeData;
/* }}} */

/* {{{ panel_render_status_bar
 * Render the status bar.
 * @param zone - Status bar zone boundaries
 * @param data - Status bar data
 */
void panel_render_status_bar(const Zone* zone, const StatusBarData* data);
/* }}} */

/* {{{ panel_render_narrative
 * Render the narrative panel.
 * @param zone - Narrative zone boundaries
 * @param data - Narrative data
 */
void panel_render_narrative(const Zone* zone, const NarrativeData* data);
/* }}} */

/* {{{ panel_render_player_stats
 * Render a single player's stats block.
 * @param x, y - Position
 * @param w - Available width
 * @param stats - Player statistics
 * @param is_left - true if left-aligned, false for right
 */
void panel_render_player_stats(int x, int y, int w, const PlayerStats* stats, bool is_left);
/* }}} */

/* {{{ panel_render_action_buttons
 * Render action buttons in the center of status bar.
 * @param x, y - Position
 * @param w, h - Available space
 * @param buttons - Button array
 * @param count - Number of buttons
 */
void panel_render_action_buttons(int x, int y, int w, int h,
                                 ActionButton* buttons, int count);
/* }}} */

/* {{{ panel_hit_test_button
 * Test if a position hits an action button.
 * @param x, y - Position to test
 * @param zone - Status bar zone
 * @param data - Status bar data
 * @return Button type if hit, -1 if none
 */
int panel_hit_test_button(int x, int y, const Zone* zone, const StatusBarData* data);
/* }}} */

/* {{{ panel_hit_test_narrative_scroll
 * Test if scrolling should occur in narrative panel.
 * @param wheel_delta - Mouse wheel delta
 * @param zone - Narrative zone
 * @param data - Narrative data (scroll_offset modified)
 */
void panel_handle_narrative_scroll(int wheel_delta, const Zone* zone, NarrativeData* data);
/* }}} */

#endif /* WASM_PANEL_RENDERER_H */
