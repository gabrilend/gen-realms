/*
 * game-client.c - Main Game Client Implementation for WASM
 *
 * Integrates canvas, rendering, input, WebSocket, preferences,
 * animations, and AI hooks into a complete game client.
 */

#include "game-client.h"
#include "canvas.h"
#include "draw2d.h"
#include "input.h"
#include "card-renderer.h"
#include "zone-renderer.h"
#include "panel-renderer.h"
#include "animation.h"
#include "websocket.h"
#include "preferences.h"
#include "ai-hooks.h"

#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* {{{ Global state */
static GameState g_game_state = GAME_STATE_LOADING;
static GameData g_game_data = {0};
static Layout g_layout = {0};
static ActionMode g_action_mode = ACTION_NONE;

static char g_server_url[256] = {0};
static char g_player_name[64] = {0};
static char g_error_message[256] = {0};

static bool g_initialized = false;
static bool g_running = false;
static float g_last_time = 0.0f;

/* Action buttons */
static StatusBarData g_status_data = {0};
/* }}} */

/* {{{ Forward declarations */
static void main_loop(void);
static void handle_input(void);
static void update(float dt);
static void render(void);

static void on_ws_connect(void* user_data);
static void on_ws_disconnect(int code, const char* reason, void* user_data);
static void on_ws_message(const WebSocketMessage* msg, void* user_data);
static void on_ws_error(const char* error, void* user_data);

static void on_narrative(const char* text, bool complete, void* user_data);
static void on_ai_error(const char* error, void* user_data);
/* }}} */

/* {{{ init_default_data
 * Initialize game data with defaults for testing.
 */
static void init_default_data(void) {
    memset(&g_game_data, 0, sizeof(g_game_data));

    /* Player defaults */
    strncpy(g_game_data.player.name, g_player_name[0] ? g_player_name : "Player",
            sizeof(g_game_data.player.name) - 1);
    g_game_data.player.authority = 50;
    g_game_data.player.trade = 0;
    g_game_data.player.combat = 0;
    g_game_data.player.deck_count = 10;
    g_game_data.player.discard_count = 0;
    g_game_data.player.hand_count = 5;
    g_game_data.player.is_current_player = true;

    /* Opponent defaults */
    strncpy(g_game_data.opponent.name, "Opponent", sizeof(g_game_data.opponent.name) - 1);
    g_game_data.opponent.authority = 50;
    g_game_data.opponent.trade = 0;
    g_game_data.opponent.combat = 0;
    g_game_data.opponent.deck_count = 10;
    g_game_data.opponent.discard_count = 0;
    g_game_data.opponent.hand_count = 5;
    g_game_data.opponent.is_current_player = false;

    /* Turn info */
    g_game_data.turn.turn_number = 1;
    g_game_data.turn.is_player_turn = true;
    g_game_data.turn.can_end_turn = true;
    g_game_data.turn.phase_name = "Main Phase";

    /* Hand with sample cards */
    g_game_data.hand.card_count = 3;
    g_game_data.hand.selected_index = -1;
    g_game_data.hand.hover_index = -1;

    for (int i = 0; i < g_game_data.hand.card_count; i++) {
        CardRenderData* card = &g_game_data.hand.cards[i].render_data;
        card->name = "Scout";
        card->faction = "neutral";
        card->card_type = "ship";
        card->cost = 0;
        card->trade_value = 1;
        card->state = CARD_STATE_NORMAL;
        g_game_data.hand.cards[i].id = 100 + i;
        g_game_data.hand.cards[i].visible = true;
    }

    /* Trade row with sample cards */
    g_game_data.trade_row.card_count = 5;
    g_game_data.trade_row.deck_count = 60;
    g_game_data.trade_row.hover_index = -1;
    g_game_data.trade_row.show_explorer = true;

    const char* factions[] = {"merchant", "wilds", "kingdom", "artificer", "neutral"};
    const char* names[] = {"Trade Ship", "Forest Sprite", "Royal Guard", "Mech Walker", "Explorer"};

    for (int i = 0; i < g_game_data.trade_row.card_count; i++) {
        CardRenderData* card = &g_game_data.trade_row.cards[i].render_data;
        card->name = names[i];
        card->faction = factions[i];
        card->card_type = "ship";
        card->cost = 2 + i;
        card->trade_value = 2;
        card->combat_value = i > 1 ? 2 : 0;
        card->state = CARD_STATE_NORMAL;
        g_game_data.trade_row.cards[i].id = 200 + i;
        g_game_data.trade_row.cards[i].visible = true;
    }

    /* Narrative panel */
    g_game_data.narrative.line_count = 3;
    strncpy(g_game_data.narrative.lines[0].text, "Welcome to Symbeline Realms!",
            MAX_NARRATIVE_LINE_LEN - 1);
    g_game_data.narrative.lines[0].is_heading = true;
    g_game_data.narrative.lines[0].color = THEME_TEXT_PRIMARY;

    strncpy(g_game_data.narrative.lines[1].text, "The realm awaits your command.",
            MAX_NARRATIVE_LINE_LEN - 1);
    g_game_data.narrative.lines[1].color = THEME_TEXT_SECONDARY;

    strncpy(g_game_data.narrative.lines[2].text, "Play cards from your hand to begin.",
            MAX_NARRATIVE_LINE_LEN - 1);
    g_game_data.narrative.lines[2].is_action = true;
    g_game_data.narrative.lines[2].color = THEME_TEXT_DIM;
}
/* }}} */

/* {{{ init_status_buttons
 * Initialize action buttons.
 */
static void init_status_buttons(void) {
    g_status_data.button_count = 3;

    g_status_data.buttons[0].type = BTN_END_TURN;
    g_status_data.buttons[0].label = "End Turn";
    g_status_data.buttons[0].hotkey = "E";
    g_status_data.buttons[0].enabled = true;

    g_status_data.buttons[1].type = BTN_ATTACK;
    g_status_data.buttons[1].label = "Attack";
    g_status_data.buttons[1].hotkey = "A";
    g_status_data.buttons[1].enabled = false;

    g_status_data.buttons[2].type = BTN_SCRAP;
    g_status_data.buttons[2].label = "Scrap";
    g_status_data.buttons[2].hotkey = "S";
    g_status_data.buttons[2].enabled = false;
}
/* }}} */

/* {{{ game_init */
bool game_init(const GameClientConfig* config) {
    if (g_initialized) {
        game_cleanup();
    }

    const char* canvas_id = config && config->canvas_id ? config->canvas_id : "#canvas";

    /* Initialize canvas */
    if (!canvas_init(canvas_id)) {
        snprintf(g_error_message, sizeof(g_error_message), "Failed to initialize canvas");
        g_game_state = GAME_STATE_ERROR;
        return false;
    }

    /* Initialize input */
    if (!input_init(canvas_id)) {
        snprintf(g_error_message, sizeof(g_error_message), "Failed to initialize input");
        g_game_state = GAME_STATE_ERROR;
        return false;
    }

    /* Initialize preferences */
    prefs_init();

    /* Initialize animation system */
    anim_init();

    /* Initialize WebSocket */
    WebSocketCallbacks ws_callbacks = {
        .on_connect = on_ws_connect,
        .on_disconnect = on_ws_disconnect,
        .on_message = on_ws_message,
        .on_error = on_ws_error,
        .user_data = NULL
    };
    ws_init(&ws_callbacks);

    /* Initialize AI hooks */
    AICallbacks ai_callbacks = {
        .on_narrative = on_narrative,
        .on_error = on_ai_error,
        .user_data = NULL
    };
    ai_init(&ai_callbacks);

    /* Store config */
    if (config) {
        if (config->server_url) {
            strncpy(g_server_url, config->server_url, sizeof(g_server_url) - 1);
        }
        if (config->player_name) {
            strncpy(g_player_name, config->player_name, sizeof(g_player_name) - 1);
        }

        const UserPreferences* prefs = prefs_get();
        ai_set_enabled(
            config->enable_narrative && prefs->ai.enable_narrative,
            prefs->ai.enable_ai_hints,
            config->enable_ai_opponent && prefs->ai.enable_ai_opponent
        );
    }

    /* Initialize default game data */
    init_default_data();
    init_status_buttons();

    /* Calculate initial layout */
    int w, h;
    canvas_get_size(&w, &h);
    theme_calculate_layout(&g_layout, w, h);

    g_game_state = GAME_STATE_MENU;
    g_initialized = true;

    return true;
}
/* }}} */

/* {{{ game_cleanup */
void game_cleanup(void) {
    if (!g_initialized) return;

    game_stop();

    ai_cleanup();
    ws_cleanup();
    anim_cleanup();
    prefs_cleanup();
    input_cleanup();
    canvas_cleanup();

    g_initialized = false;
}
/* }}} */

/* {{{ game_start */
void game_start(void) {
    if (!g_initialized || g_running) return;

    g_running = true;
    g_last_time = (float)emscripten_get_now() / 1000.0f;

    /* Set render callback */
    canvas_set_render_callback(main_loop);
    canvas_start_render_loop();
}
/* }}} */

/* {{{ game_stop */
void game_stop(void) {
    g_running = false;
}
/* }}} */

/* {{{ game_connect */
bool game_connect(const char* url) {
    if (!g_initialized) return false;

    const char* target_url = url ? url : g_server_url;
    if (!target_url[0]) return false;

    g_game_state = GAME_STATE_CONNECTING;
    return ws_connect(target_url);
}
/* }}} */

/* {{{ game_disconnect */
void game_disconnect(void) {
    ws_disconnect();
    g_game_state = GAME_STATE_MENU;
}
/* }}} */

/* {{{ game_get_state */
GameState game_get_state(void) {
    return g_game_state;
}
/* }}} */

/* {{{ game_get_data */
const GameData* game_get_data(void) {
    return &g_game_data;
}
/* }}} */

/* {{{ game_send_action */
bool game_send_action(const char* action_type, int target_id) {
    return ws_send_action(action_type, target_id, NULL);
}
/* }}} */

/* {{{ game_end_turn */
bool game_end_turn(void) {
    return game_send_action("end_turn", -1);
}
/* }}} */

/* {{{ game_play_card */
bool game_play_card(int hand_index) {
    if (hand_index < 0 || hand_index >= g_game_data.hand.card_count) return false;
    int card_id = g_game_data.hand.cards[hand_index].id;
    return game_send_action("play_card", card_id);
}
/* }}} */

/* {{{ game_buy_card */
bool game_buy_card(int trade_index) {
    if (trade_index < 0 || trade_index >= g_game_data.trade_row.card_count) return false;
    int card_id = g_game_data.trade_row.cards[trade_index].id;
    return game_send_action("buy_card", card_id);
}
/* }}} */

/* {{{ game_attack_base */
bool game_attack_base(int base_index) {
    if (base_index < 0 || base_index >= g_game_data.opp_bases.base_count) return false;
    int base_id = g_game_data.opp_bases.bases[base_index].id;
    return game_send_action("attack_base", base_id);
}
/* }}} */

/* {{{ game_attack_authority */
bool game_attack_authority(int amount) {
    char extra[32];
    snprintf(extra, sizeof(extra), "{\"amount\":%d}", amount);
    return ws_send_action("attack_authority", -1, extra);
}
/* }}} */

/* {{{ game_set_action_mode */
void game_set_action_mode(ActionMode mode) {
    g_action_mode = mode;
}
/* }}} */

/* {{{ game_cancel_action */
void game_cancel_action(void) {
    g_action_mode = ACTION_NONE;
    g_game_data.hand.selected_index = -1;
    g_game_data.opp_bases.targeted_index = -1;
}
/* }}} */

/* {{{ main_loop
 * Main game loop called each frame.
 */
static void main_loop(void) {
    if (!g_running) return;

    /* Calculate delta time */
    float now = (float)emscripten_get_now() / 1000.0f;
    float dt = now - g_last_time;
    g_last_time = now;

    /* Clamp delta time */
    if (dt > 0.1f) dt = 0.1f;

    /* Update layout on resize */
    int w, h;
    canvas_get_size(&w, &h);
    theme_calculate_layout(&g_layout, w, h);

    /* Process input */
    handle_input();

    /* Update game state */
    update(dt);

    /* Render frame */
    render();

    /* Clear input events for next frame */
    input_update();
}
/* }}} */

/* {{{ handle_input
 * Process input events.
 */
static void handle_input(void) {
    const MouseState* mouse = input_get_mouse();

    /* Hit test for hover effects */
    HitTarget hit = input_hit_test(&g_layout);

    /* Clear previous hover states */
    g_game_data.hand.hover_index = -1;
    g_game_data.trade_row.hover_index = -1;
    g_game_data.opp_bases.hover_index = -1;
    g_game_data.play_area.hover_index = -1;

    /* Update hover based on hit */
    switch (hit.type) {
        case HIT_HAND_CARD: {
            int idx = input_get_card_at(hit.x, g_layout.hand.w,
                                        g_layout.card_w, g_game_data.hand.card_count);
            g_game_data.hand.hover_index = idx;
            break;
        }

        case HIT_TRADE_ROW_CARD: {
            /* Trade row has offset for deck pile */
            int offset_x = hit.x - g_layout.card_w - CARD_SPACING * 2 - PADDING_NORMAL;
            int available_w = g_layout.trade_row.w - g_layout.card_w * 2 - CARD_SPACING * 4;
            int idx = input_get_card_at(offset_x, available_w,
                                        g_layout.card_w, g_game_data.trade_row.card_count);
            g_game_data.trade_row.hover_index = idx;
            break;
        }

        case HIT_OPP_BASE: {
            int idx = hit.x / (g_layout.card_w / 3 + PADDING_SMALL);
            if (idx >= 0 && idx < g_game_data.opp_bases.base_count) {
                g_game_data.opp_bases.hover_index = idx;
            }
            break;
        }

        default:
            break;
    }

    /* Handle button hover */
    for (int i = 0; i < g_status_data.button_count; i++) {
        g_status_data.buttons[i].hovered = false;
    }

    if (hit.type == HIT_STATUS) {
        int btn = panel_hit_test_button(mouse->x, mouse->y, &g_layout.status, &g_status_data);
        if (btn >= 0 && btn < g_status_data.button_count) {
            g_status_data.buttons[btn].hovered = true;
        }
    }

    /* Handle clicks */
    if (input_is_mouse_clicked(MOUSE_BUTTON_LEFT)) {
        switch (hit.type) {
            case HIT_HAND_CARD:
                if (g_game_data.hand.hover_index >= 0) {
                    if (g_action_mode == ACTION_NONE) {
                        g_game_data.hand.selected_index = g_game_data.hand.hover_index;
                        game_play_card(g_game_data.hand.hover_index);
                    }
                }
                break;

            case HIT_TRADE_ROW_CARD:
                if (g_game_data.trade_row.hover_index >= 0) {
                    game_buy_card(g_game_data.trade_row.hover_index);
                }
                break;

            case HIT_OPP_BASE:
                if (g_game_data.opp_bases.hover_index >= 0 &&
                    g_game_data.player.combat > 0) {
                    game_attack_base(g_game_data.opp_bases.hover_index);
                }
                break;

            case HIT_STATUS: {
                int btn = panel_hit_test_button(mouse->x, mouse->y,
                                                &g_layout.status, &g_status_data);
                if (btn >= 0 && g_status_data.buttons[btn].enabled) {
                    switch (g_status_data.buttons[btn].type) {
                        case BTN_END_TURN:
                            game_end_turn();
                            break;
                        case BTN_ATTACK:
                            g_action_mode = ACTION_SELECT_TARGET_BASE;
                            break;
                        default:
                            break;
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    /* Handle keyboard */
    if (input_is_key_pressed(KEY_ESCAPE)) {
        game_cancel_action();
    }

    if (input_is_key_pressed(KEY_E)) {
        if (g_status_data.buttons[0].enabled) {
            game_end_turn();
        }
    }

    /* Handle narrative scroll */
    if (hit.type == HIT_NARRATIVE && mouse->wheel_delta != 0) {
        panel_handle_narrative_scroll(mouse->wheel_delta,
                                      &g_layout.narrative, &g_game_data.narrative);
    }
}
/* }}} */

/* {{{ update
 * Update game state.
 */
static void update(float dt) {
    /* Update animations */
    anim_update(dt);

    /* Update WebSocket */
    ws_update();

    /* Update AI */
    ai_update();

    /* Update button states based on game state */
    g_status_data.buttons[0].enabled = g_game_data.turn.can_end_turn;
    g_status_data.buttons[1].enabled = g_game_data.player.combat > 0;

    /* Update status bar data */
    g_status_data.player = g_game_data.player;
    g_status_data.opponent = g_game_data.opponent;
    g_status_data.turn = g_game_data.turn;
}
/* }}} */

/* {{{ render
 * Render the game frame.
 */
static void render(void) {
    /* Clear canvas */
    canvas_clear(THEME_BG_DARK);

    /* Render based on game state */
    switch (g_game_state) {
        case GAME_STATE_LOADING:
            draw_text_ex(g_layout.status.w / 2, g_layout.status.h / 2 + 200,
                         "Loading...", THEME_TEXT_PRIMARY, FONT_SIZE_TITLE,
                         TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);
            break;

        case GAME_STATE_ERROR:
            draw_text_ex(g_layout.status.w / 2, g_layout.status.h / 2 + 200,
                         g_error_message, THEME_STATUS_DANGER, FONT_SIZE_LARGE,
                         TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);
            break;

        case GAME_STATE_CONNECTING:
            draw_text_ex(g_layout.status.w / 2, g_layout.status.h / 2 + 200,
                         "Connecting...", THEME_TEXT_PRIMARY, FONT_SIZE_TITLE,
                         TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);
            break;

        case GAME_STATE_MENU:
        case GAME_STATE_WAITING:
        case GAME_STATE_PLAYING:
        case GAME_STATE_GAME_OVER:
            /* Render full game UI */

            /* Status bar */
            panel_render_status_bar(&g_layout.status, &g_status_data);

            /* Trade row */
            zone_render_trade_row(&g_layout.trade_row, &g_game_data.trade_row,
                                  g_layout.card_w, g_layout.card_h);

            /* Opponent bases */
            zone_render_bases(&g_layout.opp_bases, &g_game_data.opp_bases,
                              g_layout.card_w, g_layout.card_h, true);

            /* Play area */
            zone_render_play_area(&g_layout.play_area, &g_game_data.play_area,
                                  g_layout.card_w, g_layout.card_h);

            /* Player bases */
            zone_render_bases(&g_layout.player_bases, &g_game_data.player_bases,
                              g_layout.card_w, g_layout.card_h, false);

            /* Hand */
            zone_render_hand(&g_layout.hand, &g_game_data.hand,
                             g_layout.card_w, g_layout.card_h);

            /* Narrative panel */
            panel_render_narrative(&g_layout.narrative, &g_game_data.narrative);

            /* Render particles on top */
            anim_render_particles();
            break;
    }
}
/* }}} */

/* {{{ WebSocket callbacks */
static void on_ws_connect(void* user_data) {
    (void)user_data;
    g_game_state = GAME_STATE_WAITING;
    ws_request_state();
}

static void on_ws_disconnect(int code, const char* reason, void* user_data) {
    (void)user_data;
    g_game_state = GAME_STATE_MENU;
    snprintf(g_error_message, sizeof(g_error_message),
             "Disconnected: %d - %s", code, reason ? reason : "Unknown");
}

static void on_ws_message(const WebSocketMessage* msg, void* user_data) {
    (void)user_data;
    if (!msg) return;

    switch (msg->type) {
        case MSG_GAME_STATE:
            g_game_state = GAME_STATE_PLAYING;
            /* Parse game state from msg->raw_json */
            /* In real implementation, use cJSON to parse and update g_game_data */
            break;

        case MSG_PLAYER_ACTION:
            /* Update game state based on action */
            break;

        case MSG_NARRATIVE_UPDATE:
            /* Update narrative panel */
            break;

        case MSG_ERROR:
            snprintf(g_error_message, sizeof(g_error_message),
                     "Server error: %s", msg->error_message ? msg->error_message : "Unknown");
            break;

        default:
            break;
    }
}

static void on_ws_error(const char* error, void* user_data) {
    (void)user_data;
    snprintf(g_error_message, sizeof(g_error_message), "Connection error: %s", error);
    g_game_state = GAME_STATE_ERROR;
}
/* }}} */

/* {{{ AI callbacks */
static void on_narrative(const char* text, bool complete, void* user_data) {
    (void)user_data;

    if (complete && text) {
        /* Add narrative line */
        if (g_game_data.narrative.line_count < MAX_NARRATIVE_LINES) {
            NarrativeLine* line = &g_game_data.narrative.lines[g_game_data.narrative.line_count];
            strncpy(line->text, text, MAX_NARRATIVE_LINE_LEN - 1);
            line->color = THEME_TEXT_SECONDARY;
            line->is_heading = false;
            line->is_action = false;
            g_game_data.narrative.line_count++;
        }
    }

    g_game_data.narrative.is_streaming = !complete;
    g_game_data.narrative.streaming_partial = complete ? NULL : text;
}

static void on_ai_error(const char* error, void* user_data) {
    (void)user_data;
    (void)error;
    /* AI errors are non-fatal, just log */
}
/* }}} */

/* {{{ game_main
 * Entry point called from HTML/JS.
 */
EMSCRIPTEN_KEEPALIVE
void game_main(void) {
    GameClientConfig config = {
        .canvas_id = "#canvas",
        .server_url = "ws://localhost:8080",
        .player_name = "Player",
        .enable_narrative = true,
        .enable_ai_opponent = true
    };

    if (game_init(&config)) {
        game_start();
    }
}
/* }}} */
