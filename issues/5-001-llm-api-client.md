# 3-001: LLM API Integration Module

## Current Behavior
No LLM integration exists. Game has no AI narrative capability.

## Intended Behavior
A module that connects to LLM APIs for text generation:
- Supports multiple LLM backends (local, API)
- Handles authentication and rate limiting
- Manages request/response formatting
- Provides error handling and fallbacks
- Configurable model selection
- Async-capable for non-blocking calls

## Suggested Implementation Steps

1. Create `src/llm/` directory
2. Create `src/llm/api-client.lua` with base client:
   ```lua
   local LLMClient = {
     endpoint = "",
     api_key = "",
     model = "",
     max_tokens = 500,
     temperature = 0.7
   }
   ```
3. Implement `LLMClient.new(config)` - create configured client
4. Implement `LLMClient.complete(prompt)` - send prompt, get response
5. Implement `LLMClient.chat(messages)` - multi-turn conversation
6. Add request retry logic
7. Add rate limiting
8. Create config file for API credentials (gitignored)
9. Support environment variable auth
10. Write tests with mock responses

## Related Documents
- docs/01-architecture-overview.md
- notes/vision

## Dependencies
- None (standalone module)

## Configuration Example

```lua
-- config/llm.lua (gitignored)
return {
  backend = "anthropic",  -- or "openai", "local"
  endpoint = "https://api.anthropic.com/v1/messages",
  api_key = os.getenv("ANTHROPIC_API_KEY"),
  model = "claude-3-haiku-20240307",
  max_tokens = 500,
  temperature = 0.7
}
```

## Acceptance Criteria
- [ ] Can connect to LLM API
- [ ] Sends prompts and receives responses
- [ ] Handles API errors gracefully
- [ ] Configuration is flexible
- [ ] Credentials not committed to repo
