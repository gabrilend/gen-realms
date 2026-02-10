# 2-001: Configuration System

## Status: COMPLETED

## Current Behavior
Configuration system implemented with JSON parsing and sensible defaults.

## Intended Behavior
A JSON-based configuration system that:
- Parses server.json for runtime settings
- Supports external service endpoints (LLM, ComfyUI)
- Handles game rule overrides
- Validates configuration on startup
- Provides sensible defaults

## Suggested Implementation Steps

1. Create `src/net/01-config.h` with type definitions
2. Create `src/net/01-config.c` with implementation
3. Define configuration structure:
   ```c
   typedef struct {
       int game_port;
       int ssh_port;
       char* llm_endpoint;
       char* llm_model;
       char* comfyui_endpoint;
       int max_players;
       // Game rule overrides
       int starting_authority;
       int starting_hand_size;
       int d10_start;
   } ServerConfig;
   ```
4. Implement `ServerConfig* config_load(const char* path)`
5. Implement `void config_free(ServerConfig* config)`
6. Implement `void config_set_defaults(ServerConfig* config)`
7. Implement validation for required fields
8. Create `config/server.json.example`
9. Write tests in `tests/test-config.c`

## Related Documents
- docs/04-architecture-c-server.md

## Dependencies
- cJSON library

## Configuration Example

```json
{
  "game_port": 8080,
  "ssh_port": 8022,
  "llm_endpoint": "192.168.1.10:5000",
  "llm_model": "mistral-7b",
  "comfyui_endpoint": "192.168.1.10:8188",
  "max_players": 4,
  "game_rules": {
    "starting_authority": 50,
    "starting_hand_size": 5,
    "d10_start": 5
  }
}
```

## Acceptance Criteria
- [x] Config file parses correctly
- [x] Missing file uses defaults
- [x] Invalid config fails with clear error
- [x] Endpoints stored for later use
- [x] Example config file documented

## Implementation Notes

### Files Created
- `src/net/01-config.h` - Type definitions and function prototypes
- `src/net/01-config.c` - Implementation with cJSON parsing
- `config/server.json.example` - Example configuration
- `tests/test-config.c` - Unit tests (7 tests, all passing)

### Key Decisions
- Extended struct beyond spec to include timeout and retry settings for LLM/ComfyUI
- Added GameRules nested struct for game-specific settings
- Used local cJSON library from libs/ rather than system installation
- Endpoint formatter functions add http:// prefix when needed

### Test Coverage
- Default values initialization
- Missing file handling (returns defaults)
- Valid config validation
- Invalid port validation
- Endpoint URL formatting
- Valid JSON parsing
- Invalid JSON error handling

Completed: 2026-02-10
