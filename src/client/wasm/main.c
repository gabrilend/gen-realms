/* main.c - WebAssembly entry points for Symbeline Realms browser client
 *
 * This file defines the exported functions that JavaScript calls to interact
 * with the game. All game state and rendering is managed through these entry
 * points, allowing the C game logic to run in the browser via WebAssembly.
 *
 * Exported functions:
 *   client_init()           - Initialize client state
 *   client_cleanup()        - Release resources
 *   client_handle_message() - Process server messages (JSON)
 *   client_get_action()     - Get pending player action (JSON)
 *   client_render_frame()   - Request a frame render
 */

#include <stdlib.h>
#include <string.h>
#include <emscripten.h>

#include "js-interop.h"

/* {{{ client state
 * Main client state structure holding all game data and UI state.
 * This is allocated on init and freed on cleanup.
 */
typedef struct {
    int initialized;
    char* pending_action;      /* JSON string of player's chosen action */
    int pending_action_len;
    /* Game state will be added when Track A provides structures:
     * GameState* game;
     * int player_index;
     */
} ClientState;

static ClientState* g_client = NULL;
/* }}} */

/* {{{ client_init
 * Initialize the client state and prepare for gameplay.
 * Called once when the page loads.
 */
EMSCRIPTEN_KEEPALIVE
void client_init(void) {
    if (g_client != NULL) {
        /* Already initialized - cleanup first */
        client_cleanup();
    }

    g_client = (ClientState*)malloc(sizeof(ClientState));
    if (g_client == NULL) {
        js_log_error("Failed to allocate client state");
        return;
    }

    memset(g_client, 0, sizeof(ClientState));
    g_client->initialized = 1;

    js_log_info("Client initialized");
}
/* }}} */

/* {{{ client_cleanup
 * Release all client resources.
 * Called when navigating away from the page or resetting.
 */
EMSCRIPTEN_KEEPALIVE
void client_cleanup(void) {
    if (g_client == NULL) return;

    if (g_client->pending_action != NULL) {
        free(g_client->pending_action);
    }

    free(g_client);
    g_client = NULL;

    js_log_info("Client cleaned up");
}
/* }}} */

/* {{{ client_handle_message
 * Process an incoming JSON message from the server.
 *
 * Message types handled:
 *   - "gamestate": Full game state update
 *   - "action_result": Result of player's action
 *   - "narrative": Story text to display
 *   - "error": Error message
 *
 * @param json_message  Null-terminated JSON string from server
 */
EMSCRIPTEN_KEEPALIVE
void client_handle_message(const char* json_message) {
    if (g_client == NULL || !g_client->initialized) {
        js_log_error("Client not initialized");
        return;
    }

    if (json_message == NULL) {
        js_log_error("Null message received");
        return;
    }

    /* TODO: Parse JSON and dispatch to appropriate handler
     * This will use cJSON when integrated with Track A.
     * For now, just log that we received a message.
     */
    js_log_info("Received message from server");

    /* Trigger a render after state update */
    js_request_render();
}
/* }}} */

/* {{{ client_get_action
 * Get the player's pending action as a JSON string.
 *
 * JavaScript calls this to poll for player input.
 * Returns NULL if no action is pending.
 *
 * @return  JSON action string (caller must free), or NULL
 *
 * Note: The returned pointer is valid until the next call to
 * client_set_action() or client_cleanup().
 */
EMSCRIPTEN_KEEPALIVE
const char* client_get_action(void) {
    if (g_client == NULL || !g_client->initialized) {
        return NULL;
    }

    if (g_client->pending_action == NULL) {
        return NULL;
    }

    /* Return the pending action and clear it */
    const char* action = g_client->pending_action;
    g_client->pending_action = NULL;
    g_client->pending_action_len = 0;

    return action;
}
/* }}} */

/* {{{ client_set_action
 * Set the player's action (called from JS input handlers).
 * Internal function - not exported, but called via js_interop.
 *
 * @param json_action  JSON string representing the player's action
 */
void client_set_action(const char* json_action) {
    if (g_client == NULL || !g_client->initialized) {
        js_log_error("Client not initialized");
        return;
    }

    /* Free any existing pending action */
    if (g_client->pending_action != NULL) {
        free(g_client->pending_action);
    }

    if (json_action == NULL) {
        g_client->pending_action = NULL;
        g_client->pending_action_len = 0;
        return;
    }

    /* Copy the action string */
    int len = strlen(json_action);
    g_client->pending_action = (char*)malloc(len + 1);
    if (g_client->pending_action == NULL) {
        js_log_error("Failed to allocate action buffer");
        return;
    }

    memcpy(g_client->pending_action, json_action, len + 1);
    g_client->pending_action_len = len;
}
/* }}} */

/* {{{ client_render_frame
 * Render the current game state to the canvas.
 *
 * Called by JavaScript's requestAnimationFrame loop.
 * This function calls into JS to perform actual canvas drawing.
 */
EMSCRIPTEN_KEEPALIVE
void client_render_frame(void) {
    if (g_client == NULL || !g_client->initialized) {
        return;
    }

    /* TODO: When game state is integrated, render:
     *   - Player status (authority, d10/d4, resources)
     *   - Hand cards
     *   - Trade row
     *   - Bases in play
     *   - Narrative panel
     *
     * For now, just draw a placeholder.
     */
    js_clear_canvas();
    js_draw_text(10, 30, "Symbeline Realms - Awaiting game state...");
}
/* }}} */
