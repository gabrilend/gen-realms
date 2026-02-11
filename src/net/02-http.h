/*
 * 02-http.h - HTTP Server
 *
 * Lightweight HTTP server using libwebsockets for serving static files
 * (HTML, JS, Wasm) to browser clients. Provides the foundation for
 * WebSocket upgrade and REST API endpoints.
 *
 * Dependencies: libwebsockets (install via: xbps-install libwebsockets-devel)
 */

#ifndef SYMBELINE_HTTP_H
#define SYMBELINE_HTTP_H

#include <stdbool.h>
#include "01-config.h"

/* Forward declaration to avoid libwebsockets header dependency */
struct lws_context;

/* {{{ HTTP Server Structures */

/* HTTP server instance */
typedef struct {
    struct lws_context* context;    /* libwebsockets context */
    const ServerConfig* config;     /* Reference to server config */
    char* web_root;                 /* Path to assets/web/ directory */
    bool running;                   /* Server running flag */
} HttpServer;

/* MIME type entry for file extension mapping */
typedef struct {
    const char* extension;
    const char* mime_type;
} MimeTypeEntry;

/* }}} */

/* {{{ HTTP Server API */

/*
 * http_server_create
 * Creates and initializes an HTTP server instance.
 * Uses port from config->game_port.
 * Returns NULL on failure.
 */
HttpServer* http_server_create(const ServerConfig* config, const char* web_root);

/*
 * http_server_start
 * Starts the HTTP server (non-blocking).
 * Call http_server_service() in main loop to handle connections.
 * Returns true on success, false on failure.
 */
bool http_server_start(HttpServer* server);

/*
 * http_server_service
 * Services pending connections and requests.
 * Call this periodically (or in event loop).
 * timeout_ms: maximum time to wait for events.
 * Returns number of connections serviced, or -1 on error.
 */
int http_server_service(HttpServer* server, int timeout_ms);

/*
 * http_server_stop
 * Stops the HTTP server gracefully.
 */
void http_server_stop(HttpServer* server);

/*
 * http_server_destroy
 * Frees all resources associated with the HTTP server.
 * Safe to call with NULL.
 */
void http_server_destroy(HttpServer* server);

/*
 * http_server_is_running
 * Returns true if server is currently running.
 */
bool http_server_is_running(const HttpServer* server);

/* }}} */

/* {{{ Utility Functions */

/*
 * http_get_mime_type
 * Returns the MIME type for a given file extension.
 * Returns "application/octet-stream" for unknown types.
 */
const char* http_get_mime_type(const char* extension);

/*
 * http_get_file_extension
 * Extracts the file extension from a path.
 * Returns pointer into the path string, or NULL if no extension.
 */
const char* http_get_file_extension(const char* path);

/* }}} */

/* {{{ Default Settings */

#define HTTP_DEFAULT_WEB_ROOT "assets/web"
#define HTTP_MAX_PATH_LENGTH 1024
#define HTTP_SERVICE_TIMEOUT_MS 50

/* }}} */

#endif /* SYMBELINE_HTTP_H */
