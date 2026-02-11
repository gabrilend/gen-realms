/*
 * 01-comfyui-client.c - ComfyUI API Client Implementation
 *
 * Uses libcurl for HTTP requests and cJSON for JSON handling.
 * Implements async workflow submission with polling for completion.
 * Retrieved images are returned as raw PNG bytes.
 */

#include "01-comfyui-client.h"
#include "../../libs/cJSON.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Default configuration values
#define DEFAULT_SERVER_URL "localhost"
#define DEFAULT_PORT 8188
#define DEFAULT_TIMEOUT_MS 60000
#define DEFAULT_POLL_INTERVAL_MS 500
#define DEFAULT_MAX_POLL_ATTEMPTS 120

// {{{ WriteBuffer
// Buffer for accumulating HTTP response data.
typedef struct {
    unsigned char* data;
    size_t size;
    size_t capacity;
} WriteBuffer;
// }}}

// {{{ buffer_init
static void buffer_init(WriteBuffer* buf) {
    buf->data = malloc(1);
    buf->data[0] = '\0';
    buf->size = 0;
    buf->capacity = 1;
}
// }}}

// {{{ buffer_free
static void buffer_free(WriteBuffer* buf) {
    free(buf->data);
    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
}
// }}}

// {{{ write_callback
// libcurl write callback to accumulate response data.
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    WriteBuffer* buf = (WriteBuffer*)userp;

    // Grow buffer if needed
    size_t needed = buf->size + realsize + 1;
    if (needed > buf->capacity) {
        size_t new_capacity = buf->capacity * 2;
        while (new_capacity < needed) {
            new_capacity *= 2;
        }
        unsigned char* new_data = realloc(buf->data, new_capacity);
        if (new_data == NULL) {
            return 0; // Signal error to curl
        }
        buf->data = new_data;
        buf->capacity = new_capacity;
    }

    memcpy(buf->data + buf->size, contents, realsize);
    buf->size += realsize;
    buf->data[buf->size] = '\0';

    return realsize;
}
// }}}

// {{{ strdup_safe
static char* strdup_safe(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    return strdup(str);
}
// }}}

// {{{ comfyui_config_create
ComfyUIConfig* comfyui_config_create(void) {
    ComfyUIConfig* config = malloc(sizeof(ComfyUIConfig));
    if (config == NULL) {
        return NULL;
    }

    config->server_url = strdup_safe(DEFAULT_SERVER_URL);
    config->port = DEFAULT_PORT;
    config->timeout_ms = DEFAULT_TIMEOUT_MS;
    config->poll_interval_ms = DEFAULT_POLL_INTERVAL_MS;
    config->max_poll_attempts = DEFAULT_MAX_POLL_ATTEMPTS;

    return config;
}
// }}}

// {{{ comfyui_config_free
void comfyui_config_free(ComfyUIConfig* config) {
    if (config == NULL) {
        return;
    }

    free(config->server_url);
    free(config);
}
// }}}

// {{{ comfyui_init
bool comfyui_init(void) {
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    return res == CURLE_OK;
}
// }}}

// {{{ comfyui_cleanup
void comfyui_cleanup(void) {
    curl_global_cleanup();
}
// }}}

// {{{ comfyui_get_endpoint
char* comfyui_get_endpoint(const ComfyUIConfig* config) {
    if (config == NULL || config->server_url == NULL) {
        return NULL;
    }

    // Format as http://server:port
    size_t len = strlen("http://") + strlen(config->server_url) + 10 + 1;
    char* url = malloc(len);
    if (url == NULL) {
        return NULL;
    }
    snprintf(url, len, "http://%s:%d", config->server_url, config->port);
    return url;
}
// }}}

// {{{ comfyui_response_create
static ComfyUIResponse* comfyui_response_create(void) {
    ComfyUIResponse* response = malloc(sizeof(ComfyUIResponse));
    if (response == NULL) {
        return NULL;
    }

    response->prompt_id = NULL;
    response->status = COMFYUI_STATUS_PENDING;
    response->image_data = NULL;
    response->image_size = 0;
    response->error_message = NULL;

    return response;
}
// }}}

// {{{ comfyui_response_free
void comfyui_response_free(ComfyUIResponse* response) {
    if (response == NULL) {
        return;
    }

    free(response->prompt_id);
    free(response->image_data);
    free(response->error_message);
    free(response);
}
// }}}

// {{{ http_get
// Performs HTTP GET request and returns response body.
static char* http_get(const char* url, int timeout_ms, long* http_code) {
    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        return NULL;
    }

    WriteBuffer buffer;
    buffer_init(&buffer);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)timeout_ms);

    CURLcode res = curl_easy_perform(curl);

    if (http_code != NULL) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, http_code);
    }

    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        buffer_free(&buffer);
        return NULL;
    }

    return (char*)buffer.data;
}
// }}}

// {{{ http_get_binary
// Performs HTTP GET request and returns binary response.
static unsigned char* http_get_binary(const char* url, int timeout_ms, size_t* size) {
    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        return NULL;
    }

    WriteBuffer buffer;
    buffer_init(&buffer);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)timeout_ms);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        buffer_free(&buffer);
        return NULL;
    }

    *size = buffer.size;
    return buffer.data;
}
// }}}

// {{{ http_post_json
// Performs HTTP POST with JSON body.
static char* http_post_json(const char* url, const char* json_body,
                             int timeout_ms, long* http_code) {
    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        return NULL;
    }

    WriteBuffer buffer;
    buffer_init(&buffer);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)timeout_ms);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (http_code != NULL) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, http_code);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        buffer_free(&buffer);
        return NULL;
    }

    return (char*)buffer.data;
}
// }}}

// {{{ comfyui_submit_workflow
char* comfyui_submit_workflow(const ComfyUIConfig* config,
                               const char* workflow_json) {
    if (config == NULL || workflow_json == NULL) {
        return NULL;
    }

    char* base_url = comfyui_get_endpoint(config);
    if (base_url == NULL) {
        return NULL;
    }

    // Build prompt URL
    size_t url_len = strlen(base_url) + strlen("/prompt") + 1;
    char* url = malloc(url_len);
    snprintf(url, url_len, "%s/prompt", base_url);
    free(base_url);

    // Build request body with workflow as "prompt" field
    cJSON* request = cJSON_CreateObject();
    cJSON* prompt = cJSON_Parse(workflow_json);
    if (prompt == NULL) {
        free(url);
        cJSON_Delete(request);
        return NULL;
    }
    cJSON_AddItemToObject(request, "prompt", prompt);

    char* request_json = cJSON_PrintUnformatted(request);
    cJSON_Delete(request);

    // Submit workflow
    long http_code = 0;
    char* response = http_post_json(url, request_json, config->timeout_ms, &http_code);
    free(url);
    free(request_json);

    if (response == NULL || http_code != 200) {
        free(response);
        return NULL;
    }

    // Parse prompt_id from response
    cJSON* response_json = cJSON_Parse(response);
    free(response);

    if (response_json == NULL) {
        return NULL;
    }

    cJSON* prompt_id = cJSON_GetObjectItem(response_json, "prompt_id");
    char* result = NULL;
    if (prompt_id != NULL && cJSON_IsString(prompt_id)) {
        result = strdup_safe(prompt_id->valuestring);
    }

    cJSON_Delete(response_json);
    return result;
}
// }}}

// {{{ comfyui_get_status
ComfyUIResponse* comfyui_get_status(const ComfyUIConfig* config,
                                     const char* prompt_id) {
    ComfyUIResponse* response = comfyui_response_create();
    if (response == NULL || config == NULL || prompt_id == NULL) {
        if (response != NULL) {
            response->status = COMFYUI_STATUS_ERROR;
            response->error_message = strdup_safe("Invalid arguments");
        }
        return response;
    }

    response->prompt_id = strdup_safe(prompt_id);

    char* base_url = comfyui_get_endpoint(config);
    if (base_url == NULL) {
        response->status = COMFYUI_STATUS_ERROR;
        response->error_message = strdup_safe("Failed to build endpoint URL");
        return response;
    }

    // Build history URL
    size_t url_len = strlen(base_url) + strlen("/history/") + strlen(prompt_id) + 1;
    char* url = malloc(url_len);
    snprintf(url, url_len, "%s/history/%s", base_url, prompt_id);
    free(base_url);

    long http_code = 0;
    char* history = http_get(url, config->timeout_ms, &http_code);
    free(url);

    if (history == NULL) {
        response->status = COMFYUI_STATUS_ERROR;
        response->error_message = strdup_safe("Failed to get history");
        return response;
    }

    // Parse history response
    cJSON* history_json = cJSON_Parse(history);
    free(history);

    if (history_json == NULL) {
        response->status = COMFYUI_STATUS_ERROR;
        response->error_message = strdup_safe("Failed to parse history JSON");
        return response;
    }

    // Check if prompt_id exists in history (means completed)
    cJSON* prompt_data = cJSON_GetObjectItem(history_json, prompt_id);
    if (prompt_data == NULL) {
        // Not in history yet - either pending or running
        response->status = COMFYUI_STATUS_PENDING;
        cJSON_Delete(history_json);
        return response;
    }

    // Check for status/error
    cJSON* status = cJSON_GetObjectItem(prompt_data, "status");
    if (status != NULL) {
        cJSON* status_str = cJSON_GetObjectItem(status, "status_str");
        if (status_str != NULL && cJSON_IsString(status_str)) {
            if (strcmp(status_str->valuestring, "success") == 0) {
                response->status = COMFYUI_STATUS_COMPLETED;
            } else if (strcmp(status_str->valuestring, "error") == 0) {
                response->status = COMFYUI_STATUS_ERROR;

                cJSON* messages = cJSON_GetObjectItem(status, "messages");
                if (messages != NULL && cJSON_IsArray(messages)) {
                    cJSON* first_msg = cJSON_GetArrayItem(messages, 0);
                    if (first_msg != NULL && cJSON_IsArray(first_msg)) {
                        cJSON* msg_text = cJSON_GetArrayItem(first_msg, 1);
                        if (msg_text != NULL && cJSON_IsString(msg_text)) {
                            response->error_message = strdup_safe(msg_text->valuestring);
                        }
                    }
                }
            }
        }
    } else {
        // Old API format - presence in history means completed
        response->status = COMFYUI_STATUS_COMPLETED;
    }

    // If completed, try to find output filename
    if (response->status == COMFYUI_STATUS_COMPLETED) {
        cJSON* outputs = cJSON_GetObjectItem(prompt_data, "outputs");
        if (outputs != NULL) {
            // Iterate through nodes to find image output
            cJSON* node = outputs->child;
            while (node != NULL) {
                cJSON* images = cJSON_GetObjectItem(node, "images");
                if (images != NULL && cJSON_IsArray(images)) {
                    cJSON* first_image = cJSON_GetArrayItem(images, 0);
                    if (first_image != NULL) {
                        cJSON* filename = cJSON_GetObjectItem(first_image, "filename");
                        if (filename != NULL && cJSON_IsString(filename)) {
                            // Found output filename - store in error_message temporarily
                            // (will be used by wait_for_completion to fetch image)
                            response->error_message = strdup_safe(filename->valuestring);
                            break;
                        }
                    }
                }
                node = node->next;
            }
        }
    }

    cJSON_Delete(history_json);
    return response;
}
// }}}

// {{{ comfyui_get_image
unsigned char* comfyui_get_image(const ComfyUIConfig* config,
                                  const char* filename,
                                  size_t* size) {
    if (config == NULL || filename == NULL || size == NULL) {
        return NULL;
    }

    char* base_url = comfyui_get_endpoint(config);
    if (base_url == NULL) {
        return NULL;
    }

    // URL-encode filename components
    CURL* curl = curl_easy_init();
    char* encoded_filename = curl_easy_escape(curl, filename, 0);
    curl_easy_cleanup(curl);

    // Build view URL
    size_t url_len = strlen(base_url) + strlen("/view?filename=") +
                     strlen(encoded_filename) + 1;
    char* url = malloc(url_len);
    snprintf(url, url_len, "%s/view?filename=%s", base_url, encoded_filename);
    free(base_url);
    curl_free(encoded_filename);

    unsigned char* data = http_get_binary(url, config->timeout_ms, size);
    free(url);

    return data;
}
// }}}

// {{{ comfyui_wait_for_completion
ComfyUIResponse* comfyui_wait_for_completion(const ComfyUIConfig* config,
                                              const char* workflow_json) {
    ComfyUIResponse* response = comfyui_response_create();

    if (config == NULL || workflow_json == NULL) {
        response->status = COMFYUI_STATUS_ERROR;
        response->error_message = strdup_safe("Invalid arguments");
        return response;
    }

    // Submit workflow
    char* prompt_id = comfyui_submit_workflow(config, workflow_json);
    if (prompt_id == NULL) {
        response->status = COMFYUI_STATUS_ERROR;
        response->error_message = strdup_safe("Failed to submit workflow");
        return response;
    }

    response->prompt_id = prompt_id;

    // Poll for completion
    for (int attempt = 0; attempt < config->max_poll_attempts; attempt++) {
        ComfyUIResponse* status = comfyui_get_status(config, prompt_id);

        if (status->status == COMFYUI_STATUS_COMPLETED) {
            // Get filename from status (stored in error_message field)
            if (status->error_message != NULL) {
                size_t image_size = 0;
                unsigned char* image_data = comfyui_get_image(config,
                                                                status->error_message,
                                                                &image_size);
                if (image_data != NULL) {
                    response->image_data = image_data;
                    response->image_size = image_size;
                    response->status = COMFYUI_STATUS_COMPLETED;
                } else {
                    response->status = COMFYUI_STATUS_ERROR;
                    response->error_message = strdup_safe("Failed to retrieve image");
                }
            } else {
                response->status = COMFYUI_STATUS_COMPLETED;
            }
            comfyui_response_free(status);
            return response;
        }

        if (status->status == COMFYUI_STATUS_ERROR) {
            response->status = COMFYUI_STATUS_ERROR;
            response->error_message = strdup_safe(status->error_message);
            comfyui_response_free(status);
            return response;
        }

        comfyui_response_free(status);

        // Wait before next poll
        usleep(config->poll_interval_ms * 1000);
    }

    // Timeout
    response->status = COMFYUI_STATUS_ERROR;
    response->error_message = strdup_safe("Timeout waiting for completion");
    return response;
}
// }}}
