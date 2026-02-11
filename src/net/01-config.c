/*
 * 01-config.c - Configuration System Implementation
 *
 * Parses JSON configuration file using cJSON library.
 * Provides sensible defaults when config is missing.
 * Validates required fields before returning config.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "01-config.h"
#include "../../libs/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Default values for configuration
// Selected based on typical local development setup
#define DEFAULT_GAME_PORT 8080
#define DEFAULT_SSH_PORT 8022
#define DEFAULT_LLM_ENDPOINT "localhost"
#define DEFAULT_LLM_PORT 5000
#define DEFAULT_LLM_MODEL "llama3"
#define DEFAULT_LLM_TIMEOUT_MS 30000
#define DEFAULT_LLM_MAX_RETRIES 3
#define DEFAULT_COMFYUI_ENDPOINT "localhost"
#define DEFAULT_COMFYUI_PORT 8188
#define DEFAULT_COMFYUI_TIMEOUT_MS 60000
#define DEFAULT_COMFYUI_POLL_INTERVAL_MS 500
#define DEFAULT_MAX_PLAYERS 4
#define DEFAULT_MAX_SESSIONS 10
#define DEFAULT_STARTING_AUTHORITY 50
#define DEFAULT_STARTING_HAND_SIZE 5
#define DEFAULT_D10_START 5
#define DEFAULT_D4_START 0

// {{{ strdup_safe
// Safe string duplication that handles NULL input.
static char* strdup_safe(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    return strdup(str);
}
// }}}

// {{{ read_file
// Reads entire file into a newly allocated buffer.
// Returns NULL on failure.
static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(length + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(buffer, 1, length, file);
    buffer[read] = '\0';

    fclose(file);
    return buffer;
}
// }}}

// {{{ config_set_defaults
void config_set_defaults(ServerConfig* config) {
    if (config == NULL) {
        return;
    }

    config->game_port = DEFAULT_GAME_PORT;
    config->ssh_port = DEFAULT_SSH_PORT;

    // LLM defaults - formatted as host:port in endpoint
    config->llm_endpoint = strdup_safe(DEFAULT_LLM_ENDPOINT);
    config->llm_api_key = NULL;
    config->llm_model = strdup_safe(DEFAULT_LLM_MODEL);
    config->llm_timeout_ms = DEFAULT_LLM_TIMEOUT_MS;
    config->llm_max_retries = DEFAULT_LLM_MAX_RETRIES;

    // ComfyUI defaults
    config->comfyui_endpoint = strdup_safe(DEFAULT_COMFYUI_ENDPOINT);
    config->comfyui_port = DEFAULT_COMFYUI_PORT;
    config->comfyui_timeout_ms = DEFAULT_COMFYUI_TIMEOUT_MS;
    config->comfyui_poll_interval_ms = DEFAULT_COMFYUI_POLL_INTERVAL_MS;

    // Server limits
    config->max_players = DEFAULT_MAX_PLAYERS;
    config->max_sessions = DEFAULT_MAX_SESSIONS;

    // Game rules
    config->game_rules.starting_authority = DEFAULT_STARTING_AUTHORITY;
    config->game_rules.starting_hand_size = DEFAULT_STARTING_HAND_SIZE;
    config->game_rules.d10_start = DEFAULT_D10_START;
    config->game_rules.d4_start = DEFAULT_D4_START;
}
// }}}

// {{{ parse_game_rules
// Parses game_rules section from JSON.
static void parse_game_rules(cJSON* rules_json, GameRules* rules) {
    if (rules_json == NULL || !cJSON_IsObject(rules_json)) {
        return;
    }

    cJSON* item;

    item = cJSON_GetObjectItem(rules_json, "starting_authority");
    if (item != NULL && cJSON_IsNumber(item)) {
        rules->starting_authority = item->valueint;
    }

    item = cJSON_GetObjectItem(rules_json, "starting_hand_size");
    if (item != NULL && cJSON_IsNumber(item)) {
        rules->starting_hand_size = item->valueint;
    }

    item = cJSON_GetObjectItem(rules_json, "d10_start");
    if (item != NULL && cJSON_IsNumber(item)) {
        rules->d10_start = item->valueint;
    }

    item = cJSON_GetObjectItem(rules_json, "d4_start");
    if (item != NULL && cJSON_IsNumber(item)) {
        rules->d4_start = item->valueint;
    }
}
// }}}

// {{{ config_load
ServerConfig* config_load(const char* path) {
    ServerConfig* config = malloc(sizeof(ServerConfig));
    if (config == NULL) {
        return NULL;
    }

    // Initialize with defaults first
    config_set_defaults(config);

    // Try to read config file
    char* file_contents = read_file(path);
    if (file_contents == NULL) {
        // No config file, use defaults
        return config;
    }

    // Parse JSON
    cJSON* json = cJSON_Parse(file_contents);
    free(file_contents);

    if (json == NULL) {
        // Parse error - this is a failure, not just missing file
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Config parse error before: %s\n", error_ptr);
        }
        config_free(config);
        return NULL;
    }

    // Parse network settings
    cJSON* item;

    item = cJSON_GetObjectItem(json, "game_port");
    if (item != NULL && cJSON_IsNumber(item)) {
        config->game_port = item->valueint;
    }

    item = cJSON_GetObjectItem(json, "ssh_port");
    if (item != NULL && cJSON_IsNumber(item)) {
        config->ssh_port = item->valueint;
    }

    // Parse LLM settings
    item = cJSON_GetObjectItem(json, "llm_endpoint");
    if (item != NULL && cJSON_IsString(item)) {
        free(config->llm_endpoint);
        config->llm_endpoint = strdup_safe(item->valuestring);
    }

    item = cJSON_GetObjectItem(json, "llm_api_key");
    if (item != NULL && cJSON_IsString(item)) {
        free(config->llm_api_key);
        config->llm_api_key = strdup_safe(item->valuestring);
    }

    item = cJSON_GetObjectItem(json, "llm_model");
    if (item != NULL && cJSON_IsString(item)) {
        free(config->llm_model);
        config->llm_model = strdup_safe(item->valuestring);
    }

    item = cJSON_GetObjectItem(json, "llm_timeout_ms");
    if (item != NULL && cJSON_IsNumber(item)) {
        config->llm_timeout_ms = item->valueint;
    }

    item = cJSON_GetObjectItem(json, "llm_max_retries");
    if (item != NULL && cJSON_IsNumber(item)) {
        config->llm_max_retries = item->valueint;
    }

    // Parse ComfyUI settings
    item = cJSON_GetObjectItem(json, "comfyui_endpoint");
    if (item != NULL && cJSON_IsString(item)) {
        free(config->comfyui_endpoint);
        config->comfyui_endpoint = strdup_safe(item->valuestring);
    }

    item = cJSON_GetObjectItem(json, "comfyui_port");
    if (item != NULL && cJSON_IsNumber(item)) {
        config->comfyui_port = item->valueint;
    }

    item = cJSON_GetObjectItem(json, "comfyui_timeout_ms");
    if (item != NULL && cJSON_IsNumber(item)) {
        config->comfyui_timeout_ms = item->valueint;
    }

    item = cJSON_GetObjectItem(json, "comfyui_poll_interval_ms");
    if (item != NULL && cJSON_IsNumber(item)) {
        config->comfyui_poll_interval_ms = item->valueint;
    }

    // Parse server limits
    item = cJSON_GetObjectItem(json, "max_players");
    if (item != NULL && cJSON_IsNumber(item)) {
        config->max_players = item->valueint;
    }

    item = cJSON_GetObjectItem(json, "max_sessions");
    if (item != NULL && cJSON_IsNumber(item)) {
        config->max_sessions = item->valueint;
    }

    // Parse game rules
    cJSON* rules_json = cJSON_GetObjectItem(json, "game_rules");
    parse_game_rules(rules_json, &config->game_rules);

    cJSON_Delete(json);
    return config;
}
// }}}

// {{{ config_free
void config_free(ServerConfig* config) {
    if (config == NULL) {
        return;
    }

    free(config->llm_endpoint);
    free(config->llm_api_key);
    free(config->llm_model);
    free(config->comfyui_endpoint);
    free(config);
}
// }}}

// {{{ config_validate
bool config_validate(const ServerConfig* config, char** error_msg) {
    if (config == NULL) {
        if (error_msg != NULL) {
            *error_msg = strdup("Config is NULL");
        }
        return false;
    }

    // Validate ports
    if (config->game_port < 1 || config->game_port > 65535) {
        if (error_msg != NULL) {
            *error_msg = strdup("game_port must be between 1 and 65535");
        }
        return false;
    }

    if (config->ssh_port < 1 || config->ssh_port > 65535) {
        if (error_msg != NULL) {
            *error_msg = strdup("ssh_port must be between 1 and 65535");
        }
        return false;
    }

    // Validate timeouts
    if (config->llm_timeout_ms < 0) {
        if (error_msg != NULL) {
            *error_msg = strdup("llm_timeout_ms must be non-negative");
        }
        return false;
    }

    if (config->comfyui_timeout_ms < 0) {
        if (error_msg != NULL) {
            *error_msg = strdup("comfyui_timeout_ms must be non-negative");
        }
        return false;
    }

    // Validate game rules
    if (config->game_rules.starting_authority < 1) {
        if (error_msg != NULL) {
            *error_msg = strdup("starting_authority must be at least 1");
        }
        return false;
    }

    if (config->game_rules.starting_hand_size < 1) {
        if (error_msg != NULL) {
            *error_msg = strdup("starting_hand_size must be at least 1");
        }
        return false;
    }

    if (error_msg != NULL) {
        *error_msg = NULL;
    }
    return true;
}
// }}}

// {{{ config_get_llm_endpoint
char* config_get_llm_endpoint(const ServerConfig* config) {
    if (config == NULL || config->llm_endpoint == NULL) {
        return NULL;
    }

    // Check if endpoint already contains protocol
    if (strncmp(config->llm_endpoint, "http://", 7) == 0 ||
        strncmp(config->llm_endpoint, "https://", 8) == 0) {
        return strdup(config->llm_endpoint);
    }

    // Format as http://endpoint
    size_t len = strlen("http://") + strlen(config->llm_endpoint) + 1;
    char* url = malloc(len);
    if (url == NULL) {
        return NULL;
    }
    snprintf(url, len, "http://%s", config->llm_endpoint);
    return url;
}
// }}}

// {{{ config_get_comfyui_endpoint
char* config_get_comfyui_endpoint(const ServerConfig* config) {
    if (config == NULL || config->comfyui_endpoint == NULL) {
        return NULL;
    }

    // Check if endpoint already contains protocol
    if (strncmp(config->comfyui_endpoint, "http://", 7) == 0 ||
        strncmp(config->comfyui_endpoint, "https://", 8) == 0) {
        return strdup(config->comfyui_endpoint);
    }

    // Format as http://endpoint:port
    size_t len = strlen("http://") + strlen(config->comfyui_endpoint) +
                 10 + 1; // 10 digits for port, 1 for colon
    char* url = malloc(len);
    if (url == NULL) {
        return NULL;
    }
    snprintf(url, len, "http://%s:%d", config->comfyui_endpoint, config->comfyui_port);
    return url;
}
// }}}
