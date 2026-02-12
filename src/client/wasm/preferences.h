/*
 * preferences.h - User Preferences Storage for WASM Client
 *
 * Stores and retrieves user preferences using browser localStorage.
 * Includes display settings, gameplay options, and accessibility.
 */

#ifndef WASM_PREFERENCES_H
#define WASM_PREFERENCES_H

#include <stdbool.h>
#include <stdint.h>

/* {{{ Display preferences */
typedef struct {
    int animation_speed;          /* 0=off, 1=fast, 2=normal, 3=slow */
    bool show_card_tooltips;
    bool show_value_changes;
    bool high_contrast_mode;
    bool reduce_motion;
    int text_size;                /* 0=small, 1=normal, 2=large */
} DisplayPrefs;
/* }}} */

/* {{{ Audio preferences */
typedef struct {
    int master_volume;            /* 0-100 */
    int sfx_volume;               /* 0-100 */
    int music_volume;             /* 0-100 */
    bool mute_all;
} AudioPrefs;
/* }}} */

/* {{{ Gameplay preferences */
typedef struct {
    bool auto_end_turn;           /* Auto end when no actions left */
    bool confirm_attacks;         /* Require confirmation for attacks */
    bool confirm_purchases;       /* Require confirmation for purchases */
    bool show_tutorial_hints;
    int card_size;                /* 0=small, 1=normal, 2=large */
} GameplayPrefs;
/* }}} */

/* {{{ AI preferences */
typedef struct {
    bool enable_narrative;        /* Show LLM-generated narrative */
    int narrative_verbosity;      /* 0=minimal, 1=normal, 2=verbose */
    bool enable_ai_hints;         /* Show AI play suggestions */
    bool enable_ai_opponent;      /* AI controls opponent */
    int ai_difficulty;            /* 0=easy, 1=normal, 2=hard */
} AIPrefs;
/* }}} */

/* {{{ Keybind preferences */
#define MAX_KEYBINDS 16
typedef struct {
    int key_end_turn;
    int key_cancel;
    int key_select_next;
    int key_select_prev;
    int key_confirm;
    int key_toggle_narrative;
} KeybindPrefs;
/* }}} */

/* {{{ UserPreferences
 * All user preferences combined.
 */
typedef struct {
    DisplayPrefs display;
    AudioPrefs audio;
    GameplayPrefs gameplay;
    AIPrefs ai;
    KeybindPrefs keybinds;

    /* Meta */
    int version;                  /* Schema version for migration */
    bool first_launch;            /* First time running */
} UserPreferences;
/* }}} */

/* {{{ prefs_init
 * Initialize preferences system and load saved prefs.
 * @return true on success
 */
bool prefs_init(void);
/* }}} */

/* {{{ prefs_cleanup
 * Clean up preferences system.
 */
void prefs_cleanup(void);
/* }}} */

/* {{{ prefs_get
 * Get current preferences (read-only).
 * @return Pointer to preferences (do not free)
 */
const UserPreferences* prefs_get(void);
/* }}} */

/* {{{ prefs_get_mutable
 * Get mutable preferences for modification.
 * Call prefs_save() after modifications.
 * @return Pointer to preferences
 */
UserPreferences* prefs_get_mutable(void);
/* }}} */

/* {{{ prefs_save
 * Save current preferences to localStorage.
 * @return true on success
 */
bool prefs_save(void);
/* }}} */

/* {{{ prefs_reset
 * Reset all preferences to defaults.
 */
void prefs_reset(void);
/* }}} */

/* {{{ prefs_reset_keybinds
 * Reset keybinds to defaults.
 */
void prefs_reset_keybinds(void);
/* }}} */

/* {{{ prefs_export_json
 * Export preferences as JSON string.
 * @param buf - Output buffer
 * @param buf_size - Buffer size
 * @return Number of bytes written, or -1 on error
 */
int prefs_export_json(char* buf, int buf_size);
/* }}} */

/* {{{ prefs_import_json
 * Import preferences from JSON string.
 * @param json - JSON string
 * @return true on success
 */
bool prefs_import_json(const char* json);
/* }}} */

#endif /* WASM_PREFERENCES_H */
