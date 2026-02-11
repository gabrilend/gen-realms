# 5-001: LLM API Client

## Status: COMPLETED

## Current Behavior
LLM API client implemented with libcurl for HTTP requests and cJSON for JSON handling.

## Intended Behavior
A C-based HTTP client for LLM API calls that:
- Connects to configurable LLM endpoint (OpenAI-compatible)
- Sends JSON requests with prompts
- Parses JSON responses
- Handles errors and retries
- Supports streaming responses (optional)

## Suggested Implementation Steps

1. Create `src/llm/api-client.h` with type definitions:
   ```c
   // {{{ LLM types
   typedef struct {
       char* endpoint;
       char* api_key;
       char* model;
       int timeout_ms;
       int max_retries;
   } LLMConfig;

   typedef struct {
       char* text;
       int tokens_used;
       bool success;
       char* error;
   } LLMResponse;
   // }}}
   ```

2. Create `src/llm/api-client.c` with implementation

3. Implement `llm_init(LLMConfig* config)`:
   ```c
   // {{{ llm_init
   void llm_init(LLMConfig* config) {
       // Initialize libcurl or similar HTTP client
       curl_global_init(CURL_GLOBAL_DEFAULT);
   }
   // }}}
   ```

4. Implement `llm_request()`:
   ```c
   // {{{ llm_request
   LLMResponse* llm_request(const char* prompt, const char* system_prompt) {
       cJSON* request = cJSON_CreateObject();
       cJSON_AddStringToObject(request, "model", config->model);

       cJSON* messages = cJSON_CreateArray();
       // Add system and user messages
       // ...

       // Send HTTP POST to endpoint
       // Parse response
   }
   // }}}
   ```

5. Implement retry logic with exponential backoff

6. Implement response parsing

7. Add error handling for network failures

8. Write tests with mock server

## Related Documents
- docs/04-architecture-c-server.md
- 2-001-configuration-system.md

## Dependencies
- libcurl for HTTP requests
- cJSON for JSON handling
- 2-001: Configuration System (endpoint settings)

## API Format

```json
// Request
{
  "model": "gpt-4",
  "messages": [
    {"role": "system", "content": "You are a fantasy narrator..."},
    {"role": "user", "content": "Describe the dire bear attack"}
  ]
}

// Response
{
  "choices": [{
    "message": {"content": "The dire bear emerges..."}
  }],
  "usage": {"total_tokens": 150}
}
```

## Acceptance Criteria
- [x] HTTP client connects to LLM endpoint
- [x] JSON requests formatted correctly
- [x] Responses parsed successfully
- [x] Errors handled gracefully
- [x] Retries work with backoff

## Implementation Notes

### Files Created
- `src/llm/01-api-client.h` - Type definitions and function prototypes
- `src/llm/01-api-client.c` - Implementation with libcurl and cJSON
- `tests/test-llm.c` - Unit tests (9 tests, all passing)

### Key Decisions
- Used numbered filename prefix (01-) for reading order per project conventions
- Added LLMMessage struct for multi-message conversations
- Implemented exponential backoff (1s initial, 16s max, doubles each retry)
- Only retry on 5xx errors or network issues, not on 4xx client errors
- llm_init/llm_cleanup handle global curl state

### API Functions
- `llm_config_create()` - Creates config with defaults
- `llm_init()` / `llm_cleanup()` - Global initialization
- `llm_request()` - Simple system+user prompt request
- `llm_request_messages()` - Multi-message conversation request
- `llm_response_free()` - Cleanup response memory

### Test Coverage
- Config creation and defaults
- Message creation
- Init/cleanup cycle
- NULL argument handling
- Connection refused error
- Memory safety (NULL free operations)

Completed: 2026-02-10
