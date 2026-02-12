/* phase-2-stubs.c - Stub functions for Phase 2 demo
 *
 * Provides minimal implementations of network send functions
 * that the connections module depends on. Since the demo simulates
 * network communication rather than using actual sockets, these
 * stubs allow the demo to compile without libwebsockets/libssh.
 *
 * These stubs simply return success without doing anything,
 * as the demo uses its own simulate_* functions for output.
 */

#include <stdbool.h>
#include <stddef.h>

/* Forward declarations to match what connections.c expects */
typedef struct WSConnection WSConnection;
typedef struct SSHConnection SSHConnection;

/* {{{ ws_send
 * Stub for WebSocket send - does nothing, returns success.
 */
bool ws_send(WSConnection* ws, const char* message) {
    (void)ws;
    (void)message;
    return true;
}
/* }}} */

/* {{{ ssh_connection_send_string
 * Stub for SSH send - does nothing, returns success.
 */
bool ssh_connection_send_string(SSHConnection* ssh, const char* message) {
    (void)ssh;
    (void)message;
    return true;
}
/* }}} */
