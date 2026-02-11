/*
 * 01-api-client.h - LLM API Client
 *
 * HTTP client for LLM API calls (OpenAI-compatible endpoints).
 * Sends JSON requests with prompts and parses responses.
 * Supports retry logic with exponential backoff.
 */

#ifndef LLM_API_CLIENT_H
#define LLM_API_CLIENT_H

#include <stdbool.h>
#include <stddef.h>

// {{{ LLMConfig
typedef struct {
    char* endpoint;      // Base URL (e.g., "http://localhost:5000")
    char* api_key;       // Optional API key
    char* model;         // Model name (e.g., "llama3", "gpt-4")
    int timeout_ms;      // Request timeout in milliseconds
    int max_retries;     // Max retry attempts on failure
} LLMConfig;
// }}}

// {{{ LLMResponse
typedef struct {
    char* text;          // Generated text content
    int tokens_used;     // Total tokens consumed
    bool success;        // True if request succeeded
    char* error;         // Error message if failed
} LLMResponse;
// }}}

// {{{ LLMMessage
typedef struct {
    char* role;          // "system", "user", or "assistant"
    char* content;       // Message content
} LLMMessage;
// }}}

// {{{ llm_config_create
// Creates an LLMConfig with default values.
LLMConfig* llm_config_create(void);
// }}}

// {{{ llm_config_free
// Frees all memory associated with config.
void llm_config_free(LLMConfig* config);
// }}}

// {{{ llm_init
// Initializes the LLM client. Call once at startup.
// Returns true on success, false on failure.
bool llm_init(void);
// }}}

// {{{ llm_cleanup
// Cleans up the LLM client. Call once at shutdown.
void llm_cleanup(void);
// }}}

// {{{ llm_request
// Sends a simple request with system and user prompts.
// Caller must free the response with llm_response_free.
LLMResponse* llm_request(const LLMConfig* config,
                          const char* system_prompt,
                          const char* user_prompt);
// }}}

// {{{ llm_request_messages
// Sends a request with an array of messages.
// Caller must free the response with llm_response_free.
LLMResponse* llm_request_messages(const LLMConfig* config,
                                   const LLMMessage* messages,
                                   size_t message_count);
// }}}

// {{{ llm_response_free
// Frees all memory associated with response.
void llm_response_free(LLMResponse* response);
// }}}

// {{{ llm_message_create
// Creates a message with the given role and content.
// Caller must free with llm_message_free.
LLMMessage* llm_message_create(const char* role, const char* content);
// }}}

// {{{ llm_message_free
// Frees a single message.
void llm_message_free(LLMMessage* message);
// }}}

#endif /* LLM_API_CLIENT_H */
