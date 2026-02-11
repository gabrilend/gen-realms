/*
 * test-ssh.c - Tests for SSH Server
 *
 * Tests SSH server lifecycle and utility functions.
 * Full connection tests require libssh.
 *
 * Compile: make test-ssh
 * Requires: libssh-devel (xbps-install libssh-devel)
 */

/* Enable POSIX functions */
#define _POSIX_C_SOURCE 200809L

#include "../src/net/03-ssh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("Running %s...", #name); name(); printf(" PASS\n"); } while(0)

/* {{{ test_server_create_destroy */
TEST(test_server_create_destroy) {
    /* Create config with defaults */
    ServerConfig* config = malloc(sizeof(ServerConfig));
    config_set_defaults(config);

    /* Create server */
    SSHServer* server = ssh_server_create(config);
    assert(server != NULL);
    assert(!ssh_server_is_running(server));
    assert(ssh_server_get_connection_count(server) == 0);

    /* Destroy server */
    ssh_server_destroy(server);
    config_free(config);
}
/* }}} */

/* {{{ test_server_null_config */
TEST(test_server_null_config) {
    SSHServer* server = ssh_server_create(NULL);
    assert(server == NULL);
}
/* }}} */

/* {{{ test_host_key_generation */
TEST(test_host_key_generation) {
    /* Use temp directory for test key */
    const char* test_key_path = "/tmp/symbeline-test-ssh-key";

    /* Remove any existing test key */
    unlink(test_key_path);

    /* Generate key */
    bool result = ssh_ensure_host_key(test_key_path);
    assert(result == true);

    /* Check key file exists */
    assert(access(test_key_path, F_OK) == 0);

    /* Second call should use existing key */
    result = ssh_ensure_host_key(test_key_path);
    assert(result == true);

    /* Cleanup */
    unlink(test_key_path);
}
/* }}} */

/* {{{ test_host_key_null_path */
TEST(test_host_key_null_path) {
    bool result = ssh_ensure_host_key(NULL);
    assert(result == false);
}
/* }}} */

/* {{{ test_connection_send_null */
TEST(test_connection_send_null) {
    /* Sending to NULL connection should return -1 */
    int result = ssh_connection_send(NULL, "test", 4);
    assert(result == -1);

    /* Sending NULL data should return -1 */
    SSHConnection conn = {0};
    result = ssh_connection_send(&conn, NULL, 0);
    assert(result == -1);
}
/* }}} */

/* {{{ test_connection_send_string_null */
TEST(test_connection_send_string_null) {
    /* Sending NULL string should return -1 */
    SSHConnection conn = {0};
    int result = ssh_connection_send_string(&conn, NULL);
    assert(result == -1);
}
/* }}} */

/* {{{ test_connection_close_null */
TEST(test_connection_close_null) {
    /* Closing NULL connection should not crash */
    ssh_connection_close(NULL);

    /* Closing connection with NULL channel should not crash */
    SSHConnection conn = {0};
    conn.channel = NULL;
    ssh_connection_close(&conn);
    assert(conn.active == false);
}
/* }}} */

/* {{{ test_server_stop_not_running */
TEST(test_server_stop_not_running) {
    /* Stopping a non-running server should not crash */
    ServerConfig* config = malloc(sizeof(ServerConfig));
    config_set_defaults(config);

    SSHServer* server = ssh_server_create(config);
    assert(server != NULL);

    /* Stop before start - should be safe */
    ssh_server_stop(server);
    assert(!ssh_server_is_running(server));

    ssh_server_destroy(server);
    config_free(config);
}
/* }}} */

/* {{{ main */
int main(void) {
    printf("=== SSH Server Tests ===\n\n");

    printf("--- Server Lifecycle Tests ---\n");
    RUN_TEST(test_server_create_destroy);
    RUN_TEST(test_server_null_config);
    RUN_TEST(test_server_stop_not_running);

    printf("\n--- Host Key Tests ---\n");
    RUN_TEST(test_host_key_generation);
    RUN_TEST(test_host_key_null_path);

    printf("\n--- Connection Utility Tests ---\n");
    RUN_TEST(test_connection_send_null);
    RUN_TEST(test_connection_send_string_null);
    RUN_TEST(test_connection_close_null);

    printf("\nAll tests passed!\n");
    printf("\nNote: Full server start/stop tests require SSH port binding.\n");
    printf("Run './bin/symbeline-server' to test SSH server interactively.\n");
    return 0;
}
/* }}} */
