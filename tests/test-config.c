/*
 * test-config.c - Tests for Configuration System
 *
 * Validates config loading, defaults, validation, and error handling.
 * Run with: gcc -o test-config test-config.c ../src/net/01-config.c ../libs/cJSON.c && ./test-config
 */

#include "../src/net/01-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("Running %s...", #name); name(); printf(" PASS\n"); } while(0)

// {{{ test_defaults
TEST(test_defaults) {
    ServerConfig* config = malloc(sizeof(ServerConfig));
    config_set_defaults(config);

    assert(config->game_port == 8080);
    assert(config->ssh_port == 8022);
    assert(config->llm_timeout_ms == 30000);
    assert(config->comfyui_port == 8188);
    assert(config->game_rules.starting_authority == 50);
    assert(config->game_rules.starting_hand_size == 5);

    config_free(config);
}
// }}}

// {{{ test_missing_file
TEST(test_missing_file) {
    // Loading missing file should return defaults, not NULL
    ServerConfig* config = config_load("/nonexistent/path/config.json");
    assert(config != NULL);
    assert(config->game_port == 8080); // Should have default

    config_free(config);
}
// }}}

// {{{ test_validation_valid
TEST(test_validation_valid) {
    ServerConfig* config = malloc(sizeof(ServerConfig));
    config_set_defaults(config);

    char* error_msg = NULL;
    bool valid = config_validate(config, &error_msg);

    assert(valid == true);
    assert(error_msg == NULL);

    config_free(config);
}
// }}}

// {{{ test_validation_invalid_port
TEST(test_validation_invalid_port) {
    ServerConfig* config = malloc(sizeof(ServerConfig));
    config_set_defaults(config);
    config->game_port = 999999; // Invalid

    char* error_msg = NULL;
    bool valid = config_validate(config, &error_msg);

    assert(valid == false);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "game_port") != NULL);

    free(error_msg);
    config_free(config);
}
// }}}

// {{{ test_endpoint_formatting
TEST(test_endpoint_formatting) {
    ServerConfig* config = malloc(sizeof(ServerConfig));
    config_set_defaults(config);

    // Test LLM endpoint
    char* llm_url = config_get_llm_endpoint(config);
    assert(llm_url != NULL);
    assert(strncmp(llm_url, "http://", 7) == 0);
    free(llm_url);

    // Test ComfyUI endpoint
    char* comfy_url = config_get_comfyui_endpoint(config);
    assert(comfy_url != NULL);
    assert(strstr(comfy_url, ":8188") != NULL);
    free(comfy_url);

    config_free(config);
}
// }}}

// {{{ test_parse_valid_json
TEST(test_parse_valid_json) {
    // Create temporary config file
    const char* test_json = "{"
        "\"game_port\": 9000,"
        "\"llm_model\": \"gpt-4\","
        "\"game_rules\": {"
            "\"starting_authority\": 100"
        "}"
    "}";

    const char* temp_path = "/tmp/symbeline-test-config.json";
    FILE* f = fopen(temp_path, "w");
    fprintf(f, "%s", test_json);
    fclose(f);

    ServerConfig* config = config_load(temp_path);
    assert(config != NULL);
    assert(config->game_port == 9000);
    assert(strcmp(config->llm_model, "gpt-4") == 0);
    assert(config->game_rules.starting_authority == 100);
    // Other values should be defaults
    assert(config->ssh_port == 8022);

    config_free(config);
    remove(temp_path);
}
// }}}

// {{{ test_parse_invalid_json
TEST(test_parse_invalid_json) {
    const char* temp_path = "/tmp/symbeline-test-bad-config.json";
    FILE* f = fopen(temp_path, "w");
    fprintf(f, "{ this is not valid json }");
    fclose(f);

    ServerConfig* config = config_load(temp_path);
    assert(config == NULL); // Should fail on invalid JSON

    remove(temp_path);
}
// }}}

// {{{ main
int main(void) {
    printf("=== Configuration System Tests ===\n");

    RUN_TEST(test_defaults);
    RUN_TEST(test_missing_file);
    RUN_TEST(test_validation_valid);
    RUN_TEST(test_validation_invalid_port);
    RUN_TEST(test_endpoint_formatting);
    RUN_TEST(test_parse_valid_json);
    RUN_TEST(test_parse_invalid_json);

    printf("\nAll tests passed!\n");
    return 0;
}
// }}}
