/*
 * preferences.c - User Preferences Implementation for WASM Client
 *
 * Uses browser localStorage via Emscripten for persistent storage.
 * Handles serialization, defaults, and version migration.
 */

#include "preferences.h"
#include "input.h"
#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* {{{ Constants */
#define PREFS_VERSION 1
#define PREFS_KEY "symbeline_prefs"
/* }}} */

/* {{{ Global state */
static UserPreferences g_prefs;
static bool g_initialized = false;
/* }}} */

/* {{{ set_defaults
 * Set all preferences to default values.
 */
static void set_defaults(void) {
    memset(&g_prefs, 0, sizeof(g_prefs));

    /* Display defaults */
    g_prefs.display.animation_speed = 2;      /* Normal */
    g_prefs.display.show_card_tooltips = true;
    g_prefs.display.show_value_changes = true;
    g_prefs.display.high_contrast_mode = false;
    g_prefs.display.reduce_motion = false;
    g_prefs.display.text_size = 1;            /* Normal */

    /* Audio defaults */
    g_prefs.audio.master_volume = 80;
    g_prefs.audio.sfx_volume = 100;
    g_prefs.audio.music_volume = 60;
    g_prefs.audio.mute_all = false;

    /* Gameplay defaults */
    g_prefs.gameplay.auto_end_turn = false;
    g_prefs.gameplay.confirm_attacks = false;
    g_prefs.gameplay.confirm_purchases = false;
    g_prefs.gameplay.show_tutorial_hints = true;
    g_prefs.gameplay.card_size = 1;           /* Normal */

    /* AI defaults */
    g_prefs.ai.enable_narrative = true;
    g_prefs.ai.narrative_verbosity = 1;       /* Normal */
    g_prefs.ai.enable_ai_hints = false;
    g_prefs.ai.enable_ai_opponent = true;
    g_prefs.ai.ai_difficulty = 1;             /* Normal */

    /* Keybind defaults */
    g_prefs.keybinds.key_end_turn = KEY_E;
    g_prefs.keybinds.key_cancel = KEY_ESCAPE;
    g_prefs.keybinds.key_select_next = KEY_TAB;
    g_prefs.keybinds.key_select_prev = KEY_TAB;  /* With shift */
    g_prefs.keybinds.key_confirm = KEY_ENTER;
    g_prefs.keybinds.key_toggle_narrative = KEY_N;

    /* Meta */
    g_prefs.version = PREFS_VERSION;
    g_prefs.first_launch = true;
}
/* }}} */

/* {{{ load_from_storage
 * Load preferences from localStorage.
 */
static bool load_from_storage(void) {
    /* Read from localStorage via JS */
    int result = EM_ASM_INT({
        try {
            var key = UTF8ToString($0);
            var json = localStorage.getItem(key);
            if (!json) return 0;

            var prefs = JSON.parse(json);

            /* Map JSON to C struct via Module memory */
            /* Display */
            setValue($1 + 0, prefs.display?.animation_speed ?? 2, 'i32');
            setValue($1 + 4, prefs.display?.show_card_tooltips ?? 1, 'i8');
            setValue($1 + 5, prefs.display?.show_value_changes ?? 1, 'i8');
            setValue($1 + 6, prefs.display?.high_contrast_mode ?? 0, 'i8');
            setValue($1 + 7, prefs.display?.reduce_motion ?? 0, 'i8');
            setValue($1 + 8, prefs.display?.text_size ?? 1, 'i32');

            /* Audio */
            setValue($1 + 12, prefs.audio?.master_volume ?? 80, 'i32');
            setValue($1 + 16, prefs.audio?.sfx_volume ?? 100, 'i32');
            setValue($1 + 20, prefs.audio?.music_volume ?? 60, 'i32');
            setValue($1 + 24, prefs.audio?.mute_all ?? 0, 'i8');

            /* Gameplay */
            setValue($1 + 28, prefs.gameplay?.auto_end_turn ?? 0, 'i8');
            setValue($1 + 29, prefs.gameplay?.confirm_attacks ?? 0, 'i8');
            setValue($1 + 30, prefs.gameplay?.confirm_purchases ?? 0, 'i8');
            setValue($1 + 31, prefs.gameplay?.show_tutorial_hints ?? 1, 'i8');
            setValue($1 + 32, prefs.gameplay?.card_size ?? 1, 'i32');

            /* AI */
            setValue($1 + 36, prefs.ai?.enable_narrative ?? 1, 'i8');
            setValue($1 + 40, prefs.ai?.narrative_verbosity ?? 1, 'i32');
            setValue($1 + 44, prefs.ai?.enable_ai_hints ?? 0, 'i8');
            setValue($1 + 45, prefs.ai?.enable_ai_opponent ?? 1, 'i8');
            setValue($1 + 48, prefs.ai?.ai_difficulty ?? 1, 'i32');

            /* Meta */
            setValue($1 + 76, prefs.version ?? 1, 'i32');
            setValue($1 + 80, 0, 'i8'); /* Not first launch anymore */

            return 1;
        } catch (e) {
            console.error('Failed to load preferences:', e);
            return 0;
        }
    }, PREFS_KEY, &g_prefs);

    return result == 1;
}
/* }}} */

/* {{{ prefs_init */
bool prefs_init(void) {
    if (g_initialized) {
        return true;
    }

    set_defaults();

    if (!load_from_storage()) {
        /* No saved prefs - use defaults */
        g_prefs.first_launch = true;
    }

    g_initialized = true;
    return true;
}
/* }}} */

/* {{{ prefs_cleanup */
void prefs_cleanup(void) {
    prefs_save();
    g_initialized = false;
}
/* }}} */

/* {{{ prefs_get */
const UserPreferences* prefs_get(void) {
    return &g_prefs;
}
/* }}} */

/* {{{ prefs_get_mutable */
UserPreferences* prefs_get_mutable(void) {
    return &g_prefs;
}
/* }}} */

/* {{{ prefs_save */
bool prefs_save(void) {
    if (!g_initialized) return false;

    int result = EM_ASM_INT({
        try {
            var key = UTF8ToString($0);

            var prefs = {
                version: getValue($1 + 76, 'i32'),
                display: {
                    animation_speed: getValue($1 + 0, 'i32'),
                    show_card_tooltips: getValue($1 + 4, 'i8') !== 0,
                    show_value_changes: getValue($1 + 5, 'i8') !== 0,
                    high_contrast_mode: getValue($1 + 6, 'i8') !== 0,
                    reduce_motion: getValue($1 + 7, 'i8') !== 0,
                    text_size: getValue($1 + 8, 'i32')
                },
                audio: {
                    master_volume: getValue($1 + 12, 'i32'),
                    sfx_volume: getValue($1 + 16, 'i32'),
                    music_volume: getValue($1 + 20, 'i32'),
                    mute_all: getValue($1 + 24, 'i8') !== 0
                },
                gameplay: {
                    auto_end_turn: getValue($1 + 28, 'i8') !== 0,
                    confirm_attacks: getValue($1 + 29, 'i8') !== 0,
                    confirm_purchases: getValue($1 + 30, 'i8') !== 0,
                    show_tutorial_hints: getValue($1 + 31, 'i8') !== 0,
                    card_size: getValue($1 + 32, 'i32')
                },
                ai: {
                    enable_narrative: getValue($1 + 36, 'i8') !== 0,
                    narrative_verbosity: getValue($1 + 40, 'i32'),
                    enable_ai_hints: getValue($1 + 44, 'i8') !== 0,
                    enable_ai_opponent: getValue($1 + 45, 'i8') !== 0,
                    ai_difficulty: getValue($1 + 48, 'i32')
                }
            };

            localStorage.setItem(key, JSON.stringify(prefs));
            return 1;
        } catch (e) {
            console.error('Failed to save preferences:', e);
            return 0;
        }
    }, PREFS_KEY, &g_prefs);

    return result == 1;
}
/* }}} */

/* {{{ prefs_reset */
void prefs_reset(void) {
    set_defaults();
    g_prefs.first_launch = false;
    prefs_save();
}
/* }}} */

/* {{{ prefs_reset_keybinds */
void prefs_reset_keybinds(void) {
    g_prefs.keybinds.key_end_turn = KEY_E;
    g_prefs.keybinds.key_cancel = KEY_ESCAPE;
    g_prefs.keybinds.key_select_next = KEY_TAB;
    g_prefs.keybinds.key_select_prev = KEY_TAB;
    g_prefs.keybinds.key_confirm = KEY_ENTER;
    g_prefs.keybinds.key_toggle_narrative = KEY_N;
    prefs_save();
}
/* }}} */

/* {{{ prefs_export_json */
int prefs_export_json(char* buf, int buf_size) {
    if (!buf || buf_size < 512) return -1;

    int len = snprintf(buf, buf_size,
        "{"
        "\"version\":%d,"
        "\"display\":{"
            "\"animation_speed\":%d,"
            "\"show_card_tooltips\":%s,"
            "\"show_value_changes\":%s,"
            "\"high_contrast_mode\":%s,"
            "\"reduce_motion\":%s,"
            "\"text_size\":%d"
        "},"
        "\"audio\":{"
            "\"master_volume\":%d,"
            "\"sfx_volume\":%d,"
            "\"music_volume\":%d,"
            "\"mute_all\":%s"
        "},"
        "\"gameplay\":{"
            "\"auto_end_turn\":%s,"
            "\"confirm_attacks\":%s,"
            "\"confirm_purchases\":%s,"
            "\"show_tutorial_hints\":%s,"
            "\"card_size\":%d"
        "},"
        "\"ai\":{"
            "\"enable_narrative\":%s,"
            "\"narrative_verbosity\":%d,"
            "\"enable_ai_hints\":%s,"
            "\"enable_ai_opponent\":%s,"
            "\"ai_difficulty\":%d"
        "}"
        "}",
        g_prefs.version,
        g_prefs.display.animation_speed,
        g_prefs.display.show_card_tooltips ? "true" : "false",
        g_prefs.display.show_value_changes ? "true" : "false",
        g_prefs.display.high_contrast_mode ? "true" : "false",
        g_prefs.display.reduce_motion ? "true" : "false",
        g_prefs.display.text_size,
        g_prefs.audio.master_volume,
        g_prefs.audio.sfx_volume,
        g_prefs.audio.music_volume,
        g_prefs.audio.mute_all ? "true" : "false",
        g_prefs.gameplay.auto_end_turn ? "true" : "false",
        g_prefs.gameplay.confirm_attacks ? "true" : "false",
        g_prefs.gameplay.confirm_purchases ? "true" : "false",
        g_prefs.gameplay.show_tutorial_hints ? "true" : "false",
        g_prefs.gameplay.card_size,
        g_prefs.ai.enable_narrative ? "true" : "false",
        g_prefs.ai.narrative_verbosity,
        g_prefs.ai.enable_ai_hints ? "true" : "false",
        g_prefs.ai.enable_ai_opponent ? "true" : "false",
        g_prefs.ai.ai_difficulty
    );

    return len;
}
/* }}} */

/* {{{ prefs_import_json */
bool prefs_import_json(const char* json) {
    if (!json) return false;

    /* Use JS to parse and validate JSON, then load */
    int result = EM_ASM_INT({
        try {
            var json = UTF8ToString($0);
            var prefs = JSON.parse(json);

            /* Validate basic structure */
            if (typeof prefs.version !== 'number') return 0;
            if (!prefs.display || !prefs.audio || !prefs.gameplay || !prefs.ai) return 0;

            /* Store temporarily */
            var key = UTF8ToString($1);
            localStorage.setItem(key, json);

            return 1;
        } catch (e) {
            console.error('Failed to import preferences:', e);
            return 0;
        }
    }, json, PREFS_KEY);

    if (result == 1) {
        /* Reload from storage */
        return load_from_storage();
    }

    return false;
}
/* }}} */

