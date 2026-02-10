# 5-001: LLM API Client

## Current Behavior
No LLM integration exists.

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
- [ ] HTTP client connects to LLM endpoint
- [ ] JSON requests formatted correctly
- [ ] Responses parsed successfully
- [ ] Errors handled gracefully
- [ ] Retries work with backoff
