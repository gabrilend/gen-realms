/*
 * 01-config.h - Configuration System
 *
 * Loads server configuration from JSON file, providing settings for
 * game ports, LLM endpoints, ComfyUI endpoints, and game rule overrides.
 * Falls back to sensible defaults when config file is missing.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// {{{ GameRules
typedef struct {
    int starting_authority;
    int starting_hand_size;
    int d10_start;
    int d4_start;
} GameRules;
// }}}

// {{{ ServerConfig
typedef struct {
    // Network settings
    int game_port;
    int ssh_port;

    // LLM settings
    char* llm_endpoint;
    char* llm_api_key;
    char* llm_model;
    int llm_timeout_ms;
    int llm_max_retries;

    // ComfyUI settings
    char* comfyui_endpoint;
    int comfyui_port;
    int comfyui_timeout_ms;
    int comfyui_poll_interval_ms;

    // Server limits
    int max_players;
    int max_sessions;

    // Game rule overrides
    GameRules game_rules;
} ServerConfig;
// }}}

// {{{ config_load
// Loads configuration from JSON file at path.
// Returns NULL on parse error (missing file uses defaults).
ServerConfig* config_load(const char* path);
// }}}

// {{{ config_free
// Frees all memory associated with config.
void config_free(ServerConfig* config);
// }}}

// {{{ config_set_defaults
// Populates config with default values.
void config_set_defaults(ServerConfig* config);
// }}}

// {{{ config_validate
// Returns true if config is valid, false otherwise.
// Sets error_msg to describe the problem.
bool config_validate(const ServerConfig* config, char** error_msg);
// }}}

// {{{ config_get_llm_endpoint
// Returns formatted LLM endpoint URL.
// Caller must free returned string.
char* config_get_llm_endpoint(const ServerConfig* config);
// }}}

// {{{ config_get_comfyui_endpoint
// Returns formatted ComfyUI endpoint URL.
// Caller must free returned string.
char* config_get_comfyui_endpoint(const ServerConfig* config);
// }}}

#endif /* CONFIG_H */
