/*
 * test-http.c - Tests for HTTP Server
 *
 * Tests MIME type detection and file path utilities.
 * Full server tests require libwebsockets and are run separately.
 *
 * Compile: gcc -o test-http test-http.c ../src/net/01-config.c ../src/net/02-http.c \
 *          ../libs/cJSON.c -lwebsockets -lm
 */

/* Enable POSIX functions */
#define _POSIX_C_SOURCE 200809L

#include "../src/net/02-http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("Running %s...", #name); name(); printf(" PASS\n"); } while(0)

/* {{{ test_mime_html */
TEST(test_mime_html) {
    const char* mime = http_get_mime_type(".html");
    assert(mime != NULL);
    assert(strcmp(mime, "text/html") == 0);

    mime = http_get_mime_type(".htm");
    assert(strcmp(mime, "text/html") == 0);
}
/* }}} */

/* {{{ test_mime_javascript */
TEST(test_mime_javascript) {
    const char* mime = http_get_mime_type(".js");
    assert(mime != NULL);
    assert(strcmp(mime, "text/javascript") == 0);

    mime = http_get_mime_type(".mjs");
    assert(strcmp(mime, "text/javascript") == 0);
}
/* }}} */

/* {{{ test_mime_wasm */
TEST(test_mime_wasm) {
    const char* mime = http_get_mime_type(".wasm");
    assert(mime != NULL);
    assert(strcmp(mime, "application/wasm") == 0);
}
/* }}} */

/* {{{ test_mime_css */
TEST(test_mime_css) {
    const char* mime = http_get_mime_type(".css");
    assert(mime != NULL);
    assert(strcmp(mime, "text/css") == 0);
}
/* }}} */

/* {{{ test_mime_images */
TEST(test_mime_images) {
    assert(strcmp(http_get_mime_type(".png"), "image/png") == 0);
    assert(strcmp(http_get_mime_type(".jpg"), "image/jpeg") == 0);
    assert(strcmp(http_get_mime_type(".jpeg"), "image/jpeg") == 0);
    assert(strcmp(http_get_mime_type(".gif"), "image/gif") == 0);
    assert(strcmp(http_get_mime_type(".svg"), "image/svg+xml") == 0);
    assert(strcmp(http_get_mime_type(".webp"), "image/webp") == 0);
}
/* }}} */

/* {{{ test_mime_unknown */
TEST(test_mime_unknown) {
    const char* mime = http_get_mime_type(".xyz");
    assert(mime != NULL);
    assert(strcmp(mime, "application/octet-stream") == 0);

    mime = http_get_mime_type(NULL);
    assert(strcmp(mime, "application/octet-stream") == 0);
}
/* }}} */

/* {{{ test_extension_basic */
TEST(test_extension_basic) {
    const char* ext = http_get_file_extension("index.html");
    assert(ext != NULL);
    assert(strcmp(ext, ".html") == 0);

    ext = http_get_file_extension("app.js");
    assert(strcmp(ext, ".js") == 0);

    ext = http_get_file_extension("style.min.css");
    assert(strcmp(ext, ".css") == 0);
}
/* }}} */

/* {{{ test_extension_with_path */
TEST(test_extension_with_path) {
    const char* ext = http_get_file_extension("/var/www/index.html");
    assert(ext != NULL);
    assert(strcmp(ext, ".html") == 0);

    ext = http_get_file_extension("assets/web/canvas.js");
    assert(strcmp(ext, ".js") == 0);
}
/* }}} */

/* {{{ test_extension_no_extension */
TEST(test_extension_no_extension) {
    const char* ext = http_get_file_extension("Makefile");
    assert(ext == NULL);

    ext = http_get_file_extension("README");
    assert(ext == NULL);
}
/* }}} */

/* {{{ test_extension_dot_in_path */
TEST(test_extension_dot_in_path) {
    /* Dot in directory name, no extension */
    const char* ext = http_get_file_extension("/path.with.dots/filename");
    assert(ext == NULL);

    /* Dot in directory and file has extension */
    ext = http_get_file_extension("/path.with.dots/file.txt");
    assert(ext != NULL);
    assert(strcmp(ext, ".txt") == 0);
}
/* }}} */

/* {{{ test_extension_edge_cases */
TEST(test_extension_edge_cases) {
    const char* ext = http_get_file_extension(NULL);
    assert(ext == NULL);

    ext = http_get_file_extension("");
    assert(ext == NULL);

    ext = http_get_file_extension(".");
    assert(ext == NULL);

    ext = http_get_file_extension(".hidden");
    assert(ext == NULL);
}
/* }}} */

/* {{{ test_server_create_destroy */
TEST(test_server_create_destroy) {
    /* Create config with defaults */
    ServerConfig* config = malloc(sizeof(ServerConfig));
    config_set_defaults(config);

    /* Create server */
    HttpServer* server = http_server_create(config, "assets/web");
    assert(server != NULL);
    assert(!http_server_is_running(server));

    /* Destroy server */
    http_server_destroy(server);
    config_free(config);
}
/* }}} */

/* {{{ test_server_null_config */
TEST(test_server_null_config) {
    HttpServer* server = http_server_create(NULL, NULL);
    assert(server == NULL);
}
/* }}} */

/* {{{ main */
int main(void) {
    printf("=== HTTP Server Tests ===\n\n");

    printf("--- MIME Type Tests ---\n");
    RUN_TEST(test_mime_html);
    RUN_TEST(test_mime_javascript);
    RUN_TEST(test_mime_wasm);
    RUN_TEST(test_mime_css);
    RUN_TEST(test_mime_images);
    RUN_TEST(test_mime_unknown);

    printf("\n--- File Extension Tests ---\n");
    RUN_TEST(test_extension_basic);
    RUN_TEST(test_extension_with_path);
    RUN_TEST(test_extension_no_extension);
    RUN_TEST(test_extension_dot_in_path);
    RUN_TEST(test_extension_edge_cases);

    printf("\n--- Server Lifecycle Tests ---\n");
    RUN_TEST(test_server_create_destroy);
    RUN_TEST(test_server_null_config);

    printf("\nAll tests passed!\n");
    return 0;
}
/* }}} */
