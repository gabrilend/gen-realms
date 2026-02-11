# 5-007a: Context Manager Structure

## Status: COMPLETED

## Current Behavior
Context manager structure defined with token estimation, statistics tracking, and memory management. Foundation for context window management.

## Intended Behavior
Define the core context manager structure:
- Track entries with their token counts
- Maintain total token count
- Configure maximum tokens
- Initialize and cleanup properly

## Implementation Details

### Files Created
- `src/llm/06-context-manager.h` - Header with types and function declarations
- `src/llm/06-context-manager.c` - Core implementation
- `tests/test-context-manager.c` - Test suite (16 tests)

### Key Components

1. **ContextPriority Enum**: 6 priority levels for eviction ordering:
   - PRIORITY_SYSTEM (0): Highest, never evicted
   - PRIORITY_CURRENT_TURN (1): Current turn events
   - PRIORITY_WORLD_STATE (2): World state context
   - PRIORITY_RECENT_EVENTS (3): Last 3 turns
   - PRIORITY_FORCE_DESC (4): Force descriptions
   - PRIORITY_OLD_EVENTS (5): Lowest, evicted first

2. **ContextEntry Struct**: Individual context entries with:
   - text: The context string
   - token_count: Estimated tokens
   - priority: ContextPriority value
   - added_at: Timestamp
   - is_summary: Flag for summarized entries

3. **ContextManager Struct**: Main manager with:
   - entries array (default 100 max)
   - token counting (current/max)
   - eviction/summary statistics

4. **Token Estimation**: ~4 characters per token heuristic

### Functions Implemented
- `context_init()` - Create manager with max_tokens
- `context_free()` - Free all resources
- `context_estimate_tokens()` - Estimate tokens for text
- `context_get_stats()` - Get usage statistics
- `context_get_available_tokens()` - Remaining capacity
- `context_is_full()` - Check if at threshold
- `context_clear()` - Remove all entries
- `context_clear_priority()` - Remove entries by priority

### Test Results
All 16 tests passing:
- Initialization (3 tests)
- Token estimation (3 tests)
- Statistics (3 tests)
- Capacity checking (3 tests)
- Cleanup (3 tests)
- Priority values (1 test)

## Related Documents
- 5-007-context-window-management.md (parent issue)
- 5-001-llm-api-client.md

## Dependencies
- None (standalone module)

## Next Steps
5-007b will implement entry addition and context building functions.
5-007c will implement summarization for overflow handling.

## Acceptance Criteria
- [x] ContextManager structure defined
- [x] Init allocates resources correctly
- [x] Token estimation works reasonably
- [x] Cleanup frees all memory
- [x] Usage statistics available
