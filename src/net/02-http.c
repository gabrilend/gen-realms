/*
 * 02-http.c - HTTP Server Implementation
 *
 * Uses libwebsockets to serve static files and provide REST endpoints.
 * Handles MIME type detection, security headers, and prepares for
 * WebSocket upgrade (handled by 02-websocket.c).
 */

/* Enable POSIX functions like strdup, strcasecmp */
#define _POSIX_C_SOURCE 200809L

#include "02-http.h"
#include "05-websocket.h"
#include "../../libs/cJSON.h"
#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* {{{ MIME Type Table
 * Common MIME types for web serving.
 * Order matters - more specific types should come first for extension matching.
 */
static const MimeTypeEntry mime_types[] = {
    /* Web essentials */
    {".html", "text/html"},
    {".htm",  "text/html"},
    {".css",  "text/css"},
    {".js",   "text/javascript"},
    {".mjs",  "text/javascript"},
    {".json", "application/json"},

    /* WebAssembly */
    {".wasm", "application/wasm"},

    /* Images */
    {".png",  "image/png"},
    {".jpg",  "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif",  "image/gif"},
    {".svg",  "image/svg+xml"},
    {".ico",  "image/x-icon"},
    {".webp", "image/webp"},

    /* Fonts */
    {".woff",  "font/woff"},
    {".woff2", "font/woff2"},
    {".ttf",   "font/ttf"},
    {".otf",   "font/otf"},

    /* Other */
    {".txt",  "text/plain"},
    {".xml",  "application/xml"},
    {".map",  "application/json"},  /* Source maps */

    {NULL, NULL}
};
/* }}} */

/* {{{ http_get_file_extension */
const char* http_get_file_extension(const char* path) {
    if (path == NULL) {
        return NULL;
    }

    const char* dot = strrchr(path, '.');
    if (dot == NULL || dot == path) {
        return NULL;
    }

    /* Make sure the dot isn't part of the directory path */
    const char* slash = strrchr(path, '/');
    if (slash != NULL && dot < slash) {
        return NULL;
    }

    return dot;
}
/* }}} */

/* {{{ http_get_mime_type */
const char* http_get_mime_type(const char* extension) {
    if (extension == NULL) {
        return "application/octet-stream";
    }

    for (int i = 0; mime_types[i].extension != NULL; i++) {
        if (strcasecmp(extension, mime_types[i].extension) == 0) {
            return mime_types[i].mime_type;
        }
    }

    return "application/octet-stream";
}
/* }}} */

/* {{{ Per-session user data for HTTP connections */
typedef struct {
    char path[HTTP_MAX_PATH_LENGTH];
    FILE* file;
    size_t file_size;
    size_t bytes_sent;
} HttpSessionData;
/* }}} */

/* {{{ LwsUserData
 * Wrapper struct stored in lws context user data.
 * Allows both HTTP and WebSocket callbacks to access their data.
 */
typedef struct {
    HttpServer* server;
    WSContext* ws_context;
} LwsUserData;
/* }}} */

/* {{{ build_file_path
 * Safely constructs a file path from web root and request URI.
 * Prevents directory traversal attacks.
 * Returns true if path is valid, false otherwise.
 */
static bool build_file_path(char* dest, size_t dest_size,
                           const char* web_root, const char* uri) {
    if (dest == NULL || web_root == NULL || uri == NULL) {
        return false;
    }

    /* Check for directory traversal attempts */
    if (strstr(uri, "..") != NULL) {
        return false;
    }

    /* Serve index.html for root path */
    const char* file_uri = uri;
    if (strcmp(uri, "/") == 0) {
        file_uri = "/index.html";
    }

    /* Build full path */
    int written = snprintf(dest, dest_size, "%s%s", web_root, file_uri);
    if (written < 0 || (size_t)written >= dest_size) {
        return false;
    }

    return true;
}
/* }}} */

/* {{{ send_api_config
 * Sends JSON response for /api/config endpoint.
 * Exposes public service endpoints to clients.
 */
static int send_api_config(struct lws* wsi, const ServerConfig* config) {
    cJSON* json = cJSON_CreateObject();
    if (json == NULL) {
        return -1;
    }

    /* Only expose public endpoint information */
    if (config->llm_endpoint != NULL) {
        cJSON_AddStringToObject(json, "llm_endpoint", config->llm_endpoint);
    }
    if (config->comfyui_endpoint != NULL) {
        char* comfy_url = config_get_comfyui_endpoint(config);
        if (comfy_url != NULL) {
            cJSON_AddStringToObject(json, "comfyui_endpoint", comfy_url);
            free(comfy_url);
        }
    }
    cJSON_AddNumberToObject(json, "max_players", config->max_players);

    char* json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (json_str == NULL) {
        return -1;
    }

    size_t len = strlen(json_str);

    /* Build HTTP response */
    unsigned char buf[LWS_PRE + 4096];
    unsigned char* p = &buf[LWS_PRE];
    unsigned char* end = &buf[sizeof(buf) - LWS_PRE - 1];

    if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end)) {
        free(json_str);
        return -1;
    }
    if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
            (unsigned char*)"application/json", 16, &p, end)) {
        free(json_str);
        return -1;
    }
    if (lws_add_http_header_content_length(wsi, len, &p, end)) {
        free(json_str);
        return -1;
    }
    if (lws_finalize_http_header(wsi, &p, end)) {
        free(json_str);
        return -1;
    }

    /* Send headers */
    int n = lws_write(wsi, &buf[LWS_PRE], p - &buf[LWS_PRE], LWS_WRITE_HTTP_HEADERS);
    if (n < 0) {
        free(json_str);
        return -1;
    }

    /* Send body */
    memcpy(&buf[LWS_PRE], json_str, len);
    free(json_str);

    n = lws_write(wsi, &buf[LWS_PRE], len, LWS_WRITE_HTTP_FINAL);
    if (n < 0) {
        return -1;
    }

    return lws_http_transaction_completed(wsi) ? -1 : 0;
}
/* }}} */

/* {{{ send_404
 * Sends a 404 Not Found response.
 */
static int send_404(struct lws* wsi) {
    const char* body = "<html><body><h1>404 Not Found</h1></body></html>";
    size_t len = strlen(body);

    unsigned char buf[LWS_PRE + 512];
    unsigned char* p = &buf[LWS_PRE];
    unsigned char* end = &buf[sizeof(buf) - LWS_PRE - 1];

    if (lws_add_http_header_status(wsi, HTTP_STATUS_NOT_FOUND, &p, end)) {
        return -1;
    }
    if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
            (unsigned char*)"text/html", 9, &p, end)) {
        return -1;
    }
    if (lws_add_http_header_content_length(wsi, len, &p, end)) {
        return -1;
    }
    if (lws_finalize_http_header(wsi, &p, end)) {
        return -1;
    }

    int n = lws_write(wsi, &buf[LWS_PRE], p - &buf[LWS_PRE], LWS_WRITE_HTTP_HEADERS);
    if (n < 0) {
        return -1;
    }

    memcpy(&buf[LWS_PRE], body, len);
    n = lws_write(wsi, &buf[LWS_PRE], len, LWS_WRITE_HTTP_FINAL);
    if (n < 0) {
        return -1;
    }

    return lws_http_transaction_completed(wsi) ? -1 : 0;
}
/* }}} */

/* {{{ callback_http
 * Main HTTP protocol callback.
 * Handles file serving and API endpoints.
 */
static int callback_http(struct lws* wsi, enum lws_callback_reasons reason,
                        void* user, void* in, size_t len) {
    HttpSessionData* session = (HttpSessionData*)user;
    LwsUserData* lws_user = (LwsUserData*)lws_context_user(lws_get_context(wsi));
    HttpServer* server = lws_user ? lws_user->server : NULL;

    switch (reason) {
        case LWS_CALLBACK_HTTP: {
            const char* uri = (const char*)in;

            /* Handle API endpoint */
            if (strcmp(uri, "/api/config") == 0) {
                return send_api_config(wsi, server->config);
            }

            /* Build file path */
            if (!build_file_path(session->path, sizeof(session->path),
                                server->web_root, uri)) {
                return send_404(wsi);
            }

            /* Open file */
            session->file = fopen(session->path, "rb");
            if (session->file == NULL) {
                return send_404(wsi);
            }

            /* Get file size */
            fseek(session->file, 0, SEEK_END);
            session->file_size = ftell(session->file);
            fseek(session->file, 0, SEEK_SET);
            session->bytes_sent = 0;

            /* Get MIME type */
            const char* ext = http_get_file_extension(session->path);
            const char* mime = http_get_mime_type(ext);

            /* Build response headers */
            unsigned char buf[LWS_PRE + 512];
            unsigned char* p = &buf[LWS_PRE];
            unsigned char* end = &buf[sizeof(buf) - LWS_PRE - 1];

            if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end)) {
                return -1;
            }
            if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
                    (unsigned char*)mime, strlen(mime), &p, end)) {
                return -1;
            }
            if (lws_add_http_header_content_length(wsi, session->file_size, &p, end)) {
                return -1;
            }

            /* Security headers */
            const char* xfo = "SAMEORIGIN";
            if (lws_add_http_header_by_name(wsi, (unsigned char*)"X-Frame-Options:",
                    (unsigned char*)xfo, strlen(xfo), &p, end)) {
                return -1;
            }
            const char* xcto = "nosniff";
            if (lws_add_http_header_by_name(wsi, (unsigned char*)"X-Content-Type-Options:",
                    (unsigned char*)xcto, strlen(xcto), &p, end)) {
                return -1;
            }

            if (lws_finalize_http_header(wsi, &p, end)) {
                return -1;
            }

            /* Send headers */
            int n = lws_write(wsi, &buf[LWS_PRE], p - &buf[LWS_PRE], LWS_WRITE_HTTP_HEADERS);
            if (n < 0) {
                return -1;
            }

            /* Request callback for body sending */
            lws_callback_on_writable(wsi);
            return 0;
        }

        case LWS_CALLBACK_HTTP_WRITEABLE: {
            if (session->file == NULL) {
                return -1;
            }

            /* Read chunk from file */
            unsigned char buf[LWS_PRE + 4096];
            size_t remaining = session->file_size - session->bytes_sent;
            size_t chunk = remaining > 4096 ? 4096 : remaining;

            size_t read = fread(&buf[LWS_PRE], 1, chunk, session->file);
            if (read == 0) {
                fclose(session->file);
                session->file = NULL;
                return lws_http_transaction_completed(wsi) ? -1 : 0;
            }

            session->bytes_sent += read;

            /* Determine if this is the last chunk */
            enum lws_write_protocol write_mode = LWS_WRITE_HTTP;
            if (session->bytes_sent >= session->file_size) {
                write_mode = LWS_WRITE_HTTP_FINAL;
            }

            int n = lws_write(wsi, &buf[LWS_PRE], read, write_mode);
            if (n < 0) {
                return -1;
            }

            /* More data to send? */
            if (session->bytes_sent < session->file_size) {
                lws_callback_on_writable(wsi);
            } else {
                fclose(session->file);
                session->file = NULL;
                return lws_http_transaction_completed(wsi) ? -1 : 0;
            }

            return 0;
        }

        case LWS_CALLBACK_HTTP_FILE_COMPLETION:
        case LWS_CALLBACK_CLOSED_HTTP: {
            if (session->file != NULL) {
                fclose(session->file);
                session->file = NULL;
            }
            return 0;
        }

        default:
            break;
    }

    return 0;
}
/* }}} */

/* {{{ Protocol definition */
static const struct lws_protocols protocols[] = {
    {
        .name = "http",
        .callback = callback_http,
        .per_session_data_size = sizeof(HttpSessionData),
        .rx_buffer_size = 4096,
    },
    {NULL, NULL, 0, 0}
};
/* }}} */

/* {{{ http_server_create */
HttpServer* http_server_create(const ServerConfig* config, const char* web_root) {
    return http_server_create_with_websocket(config, web_root, NULL);
}
/* }}} */

/* {{{ http_server_create_with_websocket */
HttpServer* http_server_create_with_websocket(const ServerConfig* config,
                                              const char* web_root,
                                              WSContext* ws_context) {
    if (config == NULL) {
        return NULL;
    }

    HttpServer* server = malloc(sizeof(HttpServer));
    if (server == NULL) {
        return NULL;
    }

    server->config = config;
    server->running = false;
    server->context = NULL;
    server->ws_context = ws_context;

    /* Set web root */
    if (web_root != NULL) {
        server->web_root = strdup(web_root);
    } else {
        server->web_root = strdup(HTTP_DEFAULT_WEB_ROOT);
    }

    if (server->web_root == NULL) {
        free(server);
        return NULL;
    }

    return server;
}
/* }}} */

/* Static storage for lws user data (persists across context lifetime) */
static LwsUserData lws_user_data;

/* {{{ http_server_start */
bool http_server_start(HttpServer* server) {
    if (server == NULL || server->running) {
        return false;
    }

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = server->config->game_port;
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    /* Set up user data wrapper */
    lws_user_data.server = server;
    lws_user_data.ws_context = server->ws_context;
    info.user = &lws_user_data;

    /* Build protocols array - HTTP and optionally WebSocket */
    if (server->ws_context != NULL) {
        /* Create combined protocols array: HTTP + WebSocket + NULL terminator */
        static struct lws_protocols combined_protocols[3];
        combined_protocols[0] = protocols[0];  /* HTTP protocol */
        const struct lws_protocols* ws_proto = ws_get_protocol();
        combined_protocols[1] = *ws_proto;     /* WebSocket protocol */
        combined_protocols[2].name = NULL;     /* Terminator */
        combined_protocols[2].callback = NULL;

        info.protocols = combined_protocols;
    } else {
        info.protocols = protocols;
    }

    server->context = lws_create_context(&info);
    if (server->context == NULL) {
        fprintf(stderr, "HTTP server: Failed to create context\n");
        return false;
    }

    server->running = true;
    printf("HTTP server started on port %d\n", server->config->game_port);
    printf("Serving files from: %s\n", server->web_root);
    if (server->ws_context != NULL) {
        printf("WebSocket support enabled\n");
    }

    return true;
}
/* }}} */

/* {{{ http_server_service */
int http_server_service(HttpServer* server, int timeout_ms) {
    if (server == NULL || server->context == NULL || !server->running) {
        return -1;
    }

    return lws_service(server->context, timeout_ms);
}
/* }}} */

/* {{{ http_server_stop */
void http_server_stop(HttpServer* server) {
    if (server == NULL || !server->running) {
        return;
    }

    server->running = false;
    printf("HTTP server stopped\n");
}
/* }}} */

/* {{{ http_server_destroy */
void http_server_destroy(HttpServer* server) {
    if (server == NULL) {
        return;
    }

    if (server->context != NULL) {
        lws_context_destroy(server->context);
    }

    free(server->web_root);
    free(server);
}
/* }}} */

/* {{{ http_server_is_running */
bool http_server_is_running(const HttpServer* server) {
    if (server == NULL) {
        return false;
    }
    return server->running;
}
/* }}} */

/* {{{ http_server_get_ws_context */
WSContext* http_server_get_ws_context(const HttpServer* server) {
    if (server == NULL) {
        return NULL;
    }
    return server->ws_context;
}
/* }}} */
