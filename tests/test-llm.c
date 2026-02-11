/*
 * test-llm.c - Tests for LLM API Client
 *
 * Validates config creation, message handling, and response parsing.
 * Network tests are skipped if no LLM endpoint is available.
 * Run with: gcc -o test-llm test-llm.c ../src/llm/01-api-client.c ../libs/cJSON.c -lcurl && ./test-llm
 */

#include "../src/llm/01-api-client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("Running %s...", #name); name(); printf(" PASS\n"); } while(0)

// {{{ test_config_create
TEST(test_config_create) {
    LLMConfig* config = llm_config_create();
    assert(config != NULL);
    assert(config->endpoint != NULL);
    assert(config->model != NULL);
    assert(config->timeout_ms > 0);
    assert(config->max_retries >= 0);

    llm_config_free(config);
}
// }}}

// {{{ test_config_defaults
TEST(test_config_defaults) {
    LLMConfig* config = llm_config_create();
    assert(strcmp(config->endpoint, "http://localhost:5000") == 0);
    assert(strcmp(config->model, "llama3") == 0);
    assert(config->timeout_ms == 30000);
    assert(config->max_retries == 3);

    llm_config_free(config);
}
// }}}

// {{{ test_message_create
TEST(test_message_create) {
    LLMMessage* msg = llm_message_create("user", "Hello, world!");
    assert(msg != NULL);
    assert(strcmp(msg->role, "user") == 0);
    assert(strcmp(msg->content, "Hello, world!") == 0);

    llm_message_free(msg);
}
// }}}

// {{{ test_init_cleanup
TEST(test_init_cleanup) {
    bool success = llm_init();
    assert(success == true);

    llm_cleanup();

    // Should be able to re-init
    success = llm_init();
    assert(success == true);

    llm_cleanup();
}
// }}}

// {{{ test_request_null_args
TEST(test_request_null_args) {
    llm_init();

    // NULL config should fail gracefully
    LLMResponse* response = llm_request(NULL, NULL, "test");
    assert(response != NULL);
    assert(response->success == false);
    assert(response->error != NULL);

    llm_response_free(response);

    // NULL user prompt should fail gracefully
    LLMConfig* config = llm_config_create();
    response = llm_request(config, "system", NULL);
    assert(response != NULL);
    assert(response->success == false);
    assert(response->error != NULL);

    llm_response_free(response);
    llm_config_free(config);

    llm_cleanup();
}
// }}}

// {{{ test_response_free_null
TEST(test_response_free_null) {
    // Should not crash on NULL
    llm_response_free(NULL);
}
// }}}

// {{{ test_message_free_null
TEST(test_message_free_null) {
    // Should not crash on NULL
    llm_message_free(NULL);
}
// }}}

// {{{ test_config_free_null
TEST(test_config_free_null) {
    // Should not crash on NULL
    llm_config_free(NULL);
}
// }}}

// {{{ test_request_connection_refused
TEST(test_request_connection_refused) {
    llm_init();

    // Use a port that's unlikely to be in use
    LLMConfig* config = llm_config_create();
    free(config->endpoint);
    config->endpoint = strdup("http://localhost:59999");
    config->max_retries = 0; // Don't retry for speed
    config->timeout_ms = 1000; // Short timeout

    LLMResponse* response = llm_request(config, NULL, "test prompt");
    assert(response != NULL);
    assert(response->success == false);
    assert(response->error != NULL);
    // Should be a connection error
    printf(" (error: %s) ", response->error);

    llm_response_free(response);
    llm_config_free(config);

    llm_cleanup();
}
// }}}

// {{{ main
int main(void) {
    printf("=== LLM API Client Tests ===\n");

    RUN_TEST(test_config_create);
    RUN_TEST(test_config_defaults);
    RUN_TEST(test_message_create);
    RUN_TEST(test_init_cleanup);
    RUN_TEST(test_request_null_args);
    RUN_TEST(test_response_free_null);
    RUN_TEST(test_message_free_null);
    RUN_TEST(test_config_free_null);
    RUN_TEST(test_request_connection_refused);

    printf("\nAll tests passed!\n");
    return 0;
}
// }}}
