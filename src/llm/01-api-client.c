/*
 * 01-api-client.c - LLM API Client Implementation
 *
 * Uses libcurl for HTTP requests and cJSON for JSON handling.
 * Implements exponential backoff retry logic for reliability.
 * Compatible with OpenAI-style chat completion endpoints.
 */

#include "01-api-client.h"
#include "../../libs/cJSON.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Default configuration values
#define DEFAULT_ENDPOINT "http://localhost:5000"
#define DEFAULT_MODEL "llama3"
#define DEFAULT_TIMEOUT_MS 30000
#define DEFAULT_MAX_RETRIES 3
#define INITIAL_BACKOFF_MS 1000
#define MAX_BACKOFF_MS 16000

// {{{ WriteBuffer
// Buffer for accumulating HTTP response data.
typedef struct {
    char* data;
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
        char* new_data = realloc(buf->data, new_capacity);
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

// {{{ llm_config_create
LLMConfig* llm_config_create(void) {
    LLMConfig* config = malloc(sizeof(LLMConfig));
    if (config == NULL) {
        return NULL;
    }

    config->endpoint = strdup_safe(DEFAULT_ENDPOINT);
    config->api_key = NULL;
    config->model = strdup_safe(DEFAULT_MODEL);
    config->timeout_ms = DEFAULT_TIMEOUT_MS;
    config->max_retries = DEFAULT_MAX_RETRIES;

    return config;
}
// }}}

// {{{ llm_config_free
void llm_config_free(LLMConfig* config) {
    if (config == NULL) {
        return;
    }

    free(config->endpoint);
    free(config->api_key);
    free(config->model);
    free(config);
}
// }}}

// {{{ llm_init
bool llm_init(void) {
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    return res == CURLE_OK;
}
// }}}

// {{{ llm_cleanup
void llm_cleanup(void) {
    curl_global_cleanup();
}
// }}}

// {{{ build_request_json
// Builds the JSON request body for chat completion.
static char* build_request_json(const LLMConfig* config,
                                 const LLMMessage* messages,
                                 size_t message_count) {
    cJSON* root = cJSON_CreateObject();
    if (root == NULL) {
        return NULL;
    }

    cJSON_AddStringToObject(root, "model", config->model);

    cJSON* msgs_array = cJSON_CreateArray();
    for (size_t i = 0; i < message_count; i++) {
        cJSON* msg = cJSON_CreateObject();
        cJSON_AddStringToObject(msg, "role", messages[i].role);
        cJSON_AddStringToObject(msg, "content", messages[i].content);
        cJSON_AddItemToArray(msgs_array, msg);
    }
    cJSON_AddItemToObject(root, "messages", msgs_array);

    char* json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_str;
}
// }}}

// {{{ parse_response_json
// Parses the JSON response from the LLM API.
static LLMResponse* parse_response_json(const char* json_str) {
    LLMResponse* response = malloc(sizeof(LLMResponse));
    if (response == NULL) {
        return NULL;
    }

    response->text = NULL;
    response->tokens_used = 0;
    response->success = false;
    response->error = NULL;

    cJSON* root = cJSON_Parse(json_str);
    if (root == NULL) {
        response->error = strdup_safe("Failed to parse JSON response");
        return response;
    }

    // Check for error response
    cJSON* error = cJSON_GetObjectItem(root, "error");
    if (error != NULL) {
        cJSON* message = cJSON_GetObjectItem(error, "message");
        if (message != NULL && cJSON_IsString(message)) {
            response->error = strdup_safe(message->valuestring);
        } else {
            response->error = strdup_safe("Unknown API error");
        }
        cJSON_Delete(root);
        return response;
    }

    // Parse successful response
    cJSON* choices = cJSON_GetObjectItem(root, "choices");
    if (choices != NULL && cJSON_IsArray(choices)) {
        cJSON* first_choice = cJSON_GetArrayItem(choices, 0);
        if (first_choice != NULL) {
            cJSON* message = cJSON_GetObjectItem(first_choice, "message");
            if (message != NULL) {
                cJSON* content = cJSON_GetObjectItem(message, "content");
                if (content != NULL && cJSON_IsString(content)) {
                    response->text = strdup_safe(content->valuestring);
                    response->success = true;
                }
            }
        }
    }

    // Parse token usage
    cJSON* usage = cJSON_GetObjectItem(root, "usage");
    if (usage != NULL) {
        cJSON* total = cJSON_GetObjectItem(usage, "total_tokens");
        if (total != NULL && cJSON_IsNumber(total)) {
            response->tokens_used = total->valueint;
        }
    }

    if (response->text == NULL && response->error == NULL) {
        response->error = strdup_safe("No content in response");
    }

    cJSON_Delete(root);
    return response;
}
// }}}

// {{{ perform_request
// Performs a single HTTP request with given JSON body.
static LLMResponse* perform_request(const LLMConfig* config, const char* json_body) {
    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        LLMResponse* response = malloc(sizeof(LLMResponse));
        response->text = NULL;
        response->tokens_used = 0;
        response->success = false;
        response->error = strdup_safe("Failed to initialize curl");
        return response;
    }

    WriteBuffer buffer;
    buffer_init(&buffer);

    // Build URL
    size_t url_len = strlen(config->endpoint) + strlen("/v1/chat/completions") + 1;
    char* url = malloc(url_len);
    snprintf(url, url_len, "%s/v1/chat/completions", config->endpoint);

    // Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)config->timeout_ms);

    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    if (config->api_key != NULL && strlen(config->api_key) > 0) {
        size_t auth_len = strlen("Authorization: Bearer ") + strlen(config->api_key) + 1;
        char* auth_header = malloc(auth_len);
        snprintf(auth_header, auth_len, "Authorization: Bearer %s", config->api_key);
        headers = curl_slist_append(headers, auth_header);
        free(auth_header);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    LLMResponse* response;
    if (res != CURLE_OK) {
        response = malloc(sizeof(LLMResponse));
        response->text = NULL;
        response->tokens_used = 0;
        response->success = false;
        response->error = strdup_safe(curl_easy_strerror(res));
    } else {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code >= 200 && http_code < 300) {
            response = parse_response_json(buffer.data);
        } else {
            response = malloc(sizeof(LLMResponse));
            response->text = NULL;
            response->tokens_used = 0;
            response->success = false;

            // Try to parse error from response body
            cJSON* error_json = cJSON_Parse(buffer.data);
            if (error_json != NULL) {
                cJSON* error = cJSON_GetObjectItem(error_json, "error");
                if (error != NULL) {
                    cJSON* message = cJSON_GetObjectItem(error, "message");
                    if (message != NULL && cJSON_IsString(message)) {
                        response->error = strdup_safe(message->valuestring);
                    }
                }
                cJSON_Delete(error_json);
            }

            if (response->error == NULL) {
                char error_buf[64];
                snprintf(error_buf, sizeof(error_buf), "HTTP error %ld", http_code);
                response->error = strdup_safe(error_buf);
            }
        }
    }

    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    buffer_free(&buffer);
    free(url);

    return response;
}
// }}}

// {{{ llm_request_messages
LLMResponse* llm_request_messages(const LLMConfig* config,
                                   const LLMMessage* messages,
                                   size_t message_count) {
    if (config == NULL || messages == NULL || message_count == 0) {
        LLMResponse* response = malloc(sizeof(LLMResponse));
        response->text = NULL;
        response->tokens_used = 0;
        response->success = false;
        response->error = strdup_safe("Invalid arguments");
        return response;
    }

    char* json_body = build_request_json(config, messages, message_count);
    if (json_body == NULL) {
        LLMResponse* response = malloc(sizeof(LLMResponse));
        response->text = NULL;
        response->tokens_used = 0;
        response->success = false;
        response->error = strdup_safe("Failed to build request JSON");
        return response;
    }

    LLMResponse* response = NULL;
    int backoff_ms = INITIAL_BACKOFF_MS;

    // Retry loop with exponential backoff
    for (int attempt = 0; attempt <= config->max_retries; attempt++) {
        if (attempt > 0) {
            // Wait before retry
            usleep(backoff_ms * 1000);
            backoff_ms *= 2;
            if (backoff_ms > MAX_BACKOFF_MS) {
                backoff_ms = MAX_BACKOFF_MS;
            }

            // Free previous failed response
            if (response != NULL) {
                llm_response_free(response);
            }
        }

        response = perform_request(config, json_body);

        if (response->success) {
            break;
        }

        // Don't retry on client errors (4xx)
        // Only retry on server errors (5xx) or network issues
        if (response->error != NULL &&
            (strstr(response->error, "HTTP error 4") != NULL)) {
            break;
        }
    }

    free(json_body);
    return response;
}
// }}}

// {{{ llm_request
LLMResponse* llm_request(const LLMConfig* config,
                          const char* system_prompt,
                          const char* user_prompt) {
    if (config == NULL || user_prompt == NULL) {
        LLMResponse* response = malloc(sizeof(LLMResponse));
        response->text = NULL;
        response->tokens_used = 0;
        response->success = false;
        response->error = strdup_safe("Invalid arguments");
        return response;
    }

    // Build messages array
    size_t message_count = (system_prompt != NULL) ? 2 : 1;
    LLMMessage* messages = malloc(sizeof(LLMMessage) * message_count);

    size_t idx = 0;
    if (system_prompt != NULL) {
        messages[idx].role = "system";
        messages[idx].content = (char*)system_prompt;
        idx++;
    }
    messages[idx].role = "user";
    messages[idx].content = (char*)user_prompt;

    LLMResponse* response = llm_request_messages(config, messages, message_count);

    free(messages);
    return response;
}
// }}}

// {{{ llm_response_free
void llm_response_free(LLMResponse* response) {
    if (response == NULL) {
        return;
    }

    free(response->text);
    free(response->error);
    free(response);
}
// }}}

// {{{ llm_message_create
LLMMessage* llm_message_create(const char* role, const char* content) {
    LLMMessage* message = malloc(sizeof(LLMMessage));
    if (message == NULL) {
        return NULL;
    }

    message->role = strdup_safe(role);
    message->content = strdup_safe(content);

    return message;
}
// }}}

// {{{ llm_message_free
void llm_message_free(LLMMessage* message) {
    if (message == NULL) {
        return;
    }

    free(message->role);
    free(message->content);
    free(message);
}
// }}}
