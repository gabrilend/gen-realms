/*
 * test-comfyui.c - Tests for ComfyUI API Client
 *
 * Validates config creation, endpoint formatting, and error handling.
 * Network tests require a running ComfyUI server.
 * Run with: gcc -o test-comfyui test-comfyui.c ../src/visual/01-comfyui-client.c ../libs/cJSON.c -lcurl && ./test-comfyui
 */

#include "../src/visual/01-comfyui-client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("Running %s...", #name); name(); printf(" PASS\n"); } while(0)

// {{{ test_config_create
TEST(test_config_create) {
    ComfyUIConfig* config = comfyui_config_create();
    assert(config != NULL);
    assert(config->server_url != NULL);
    assert(config->port > 0);
    assert(config->timeout_ms > 0);
    assert(config->poll_interval_ms > 0);

    comfyui_config_free(config);
}
// }}}

// {{{ test_config_defaults
TEST(test_config_defaults) {
    ComfyUIConfig* config = comfyui_config_create();
    assert(strcmp(config->server_url, "localhost") == 0);
    assert(config->port == 8188);
    assert(config->timeout_ms == 60000);
    assert(config->poll_interval_ms == 500);
    assert(config->max_poll_attempts == 120);

    comfyui_config_free(config);
}
// }}}

// {{{ test_get_endpoint
TEST(test_get_endpoint) {
    ComfyUIConfig* config = comfyui_config_create();
    char* endpoint = comfyui_get_endpoint(config);

    assert(endpoint != NULL);
    assert(strcmp(endpoint, "http://localhost:8188") == 0);

    free(endpoint);
    comfyui_config_free(config);
}
// }}}

// {{{ test_get_endpoint_custom
TEST(test_get_endpoint_custom) {
    ComfyUIConfig* config = comfyui_config_create();
    free(config->server_url);
    config->server_url = strdup("192.168.1.100");
    config->port = 9999;

    char* endpoint = comfyui_get_endpoint(config);
    assert(endpoint != NULL);
    assert(strcmp(endpoint, "http://192.168.1.100:9999") == 0);

    free(endpoint);
    comfyui_config_free(config);
}
// }}}

// {{{ test_init_cleanup
TEST(test_init_cleanup) {
    bool success = comfyui_init();
    assert(success == true);

    comfyui_cleanup();

    // Should be able to re-init
    success = comfyui_init();
    assert(success == true);

    comfyui_cleanup();
}
// }}}

// {{{ test_config_free_null
TEST(test_config_free_null) {
    // Should not crash on NULL
    comfyui_config_free(NULL);
}
// }}}

// {{{ test_response_free_null
TEST(test_response_free_null) {
    // Should not crash on NULL
    comfyui_response_free(NULL);
}
// }}}

// {{{ test_submit_null_args
TEST(test_submit_null_args) {
    comfyui_init();

    // NULL config should return NULL
    char* prompt_id = comfyui_submit_workflow(NULL, "{}");
    assert(prompt_id == NULL);

    // NULL workflow should return NULL
    ComfyUIConfig* config = comfyui_config_create();
    prompt_id = comfyui_submit_workflow(config, NULL);
    assert(prompt_id == NULL);

    comfyui_config_free(config);
    comfyui_cleanup();
}
// }}}

// {{{ test_get_status_null_args
TEST(test_get_status_null_args) {
    comfyui_init();

    // NULL config
    ComfyUIResponse* response = comfyui_get_status(NULL, "test-id");
    assert(response != NULL);
    assert(response->status == COMFYUI_STATUS_ERROR);
    assert(response->error_message != NULL);
    comfyui_response_free(response);

    // NULL prompt_id
    ComfyUIConfig* config = comfyui_config_create();
    response = comfyui_get_status(config, NULL);
    assert(response != NULL);
    assert(response->status == COMFYUI_STATUS_ERROR);
    comfyui_response_free(response);

    comfyui_config_free(config);
    comfyui_cleanup();
}
// }}}

// {{{ test_get_image_null_args
TEST(test_get_image_null_args) {
    comfyui_init();

    size_t size = 0;

    // NULL config
    unsigned char* data = comfyui_get_image(NULL, "test.png", &size);
    assert(data == NULL);

    // NULL filename
    ComfyUIConfig* config = comfyui_config_create();
    data = comfyui_get_image(config, NULL, &size);
    assert(data == NULL);

    // NULL size
    data = comfyui_get_image(config, "test.png", NULL);
    assert(data == NULL);

    comfyui_config_free(config);
    comfyui_cleanup();
}
// }}}

// {{{ test_wait_null_args
TEST(test_wait_null_args) {
    comfyui_init();

    // NULL config
    ComfyUIResponse* response = comfyui_wait_for_completion(NULL, "{}");
    assert(response != NULL);
    assert(response->status == COMFYUI_STATUS_ERROR);
    comfyui_response_free(response);

    // NULL workflow
    ComfyUIConfig* config = comfyui_config_create();
    response = comfyui_wait_for_completion(config, NULL);
    assert(response != NULL);
    assert(response->status == COMFYUI_STATUS_ERROR);
    comfyui_response_free(response);

    comfyui_config_free(config);
    comfyui_cleanup();
}
// }}}

// {{{ test_connection_refused
TEST(test_connection_refused) {
    comfyui_init();

    // Use a port that's unlikely to be in use
    ComfyUIConfig* config = comfyui_config_create();
    config->port = 59998;
    config->timeout_ms = 1000; // Short timeout
    config->max_poll_attempts = 1;
    config->poll_interval_ms = 100;

    char* prompt_id = comfyui_submit_workflow(config, "{}");
    assert(prompt_id == NULL); // Should fail to connect

    printf(" (connection refused as expected) ");

    comfyui_config_free(config);
    comfyui_cleanup();
}
// }}}

// {{{ main
int main(void) {
    printf("=== ComfyUI API Client Tests ===\n");

    RUN_TEST(test_config_create);
    RUN_TEST(test_config_defaults);
    RUN_TEST(test_get_endpoint);
    RUN_TEST(test_get_endpoint_custom);
    RUN_TEST(test_init_cleanup);
    RUN_TEST(test_config_free_null);
    RUN_TEST(test_response_free_null);
    RUN_TEST(test_submit_null_args);
    RUN_TEST(test_get_status_null_args);
    RUN_TEST(test_get_image_null_args);
    RUN_TEST(test_wait_null_args);
    RUN_TEST(test_connection_refused);

    printf("\nAll tests passed!\n");
    return 0;
}
// }}}
